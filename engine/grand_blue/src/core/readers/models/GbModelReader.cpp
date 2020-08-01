#include "GbModelReader.h"

#include <algorithm> 
#include "../../containers/GbContainerExtensions.h"
#include "../../resource/GbResourceCache.h"
#include "../../rendering/models/GbModel.h"
#include "../../rendering/geometry/GbSkeleton.h"
#include "../../rendering/renderer/GbMainRenderer.h"
#include "../../utils/GbParallelization.h"
#include "../../animation/GbAnimation.h"
#include "../../utils/GbInterpolation.h"

#include <QElapsedTimer>

#include "GbOBJReader.h"

#define USE_THREADING false

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelReader::ModelReader(ResourceCache* cache, ResourceHandle& handle) :
    FileReader(handle.getPath()),
    m_resourceCache(cache),
    m_handle(&handle)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelReader::ModelReader(const ModelReader & reader) :
    FileReader(reader.m_filePath),
    m_resourceCache(reader.m_resourceCache),
    m_handle(reader.m_handle)
{
    // Only used for QObject construction
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ModelReader::~ModelReader()
{
    // Delete cached bones
    for (const std::pair<QString, Bone*>& bonePair: m_boneMapping) {
        delete bonePair.second;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadFile()
{
    m_scene = m_importer.ReadFile(m_filePath.toStdString(),
        //aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals | // generates smooth normals for all vertices, ignored if normals are present
        aiProcess_Triangulate | // splits faces with more than three indices into triangles
        aiProcess_SortByPType | // separates out meshes with different primitive types into separate meshes
        aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
        aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
        aiProcess_ValidateDataStructure | // perform a full validation of the loader's output
        aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
        aiProcess_RemoveRedundantMaterials | // remove redundant materials
        //aiProcess_FindDegenerates | // remove degenerated polygons from the import, was causing graphical bugs with OBJ files
        aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
        aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
        aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
        aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
        aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
        aiProcess_OptimizeMeshes | // join small meshes, if possible;
        aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for more limited hardware skinning shader
        0
    );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> ModelReader::loadModel()
{
    // Load file into scene
    loadFile();

    // Instantiate model
    Model::ModelType modelType = Model::kStaticMesh;
    if (m_scene->mNumAnimations > 0) modelType = Model::kAnimatedMesh;
    std::shared_ptr<Model> model = std::make_shared<Model>(m_resourceCache->engine(),
        FileReader::pathToName(m_handle->getPath()),
        modelType);
    //if (modelType == Model::kAnimatedMesh)
    m_handle->setResource(model, false);
    m_handle->setIsLoading(true);
    model->addSkeleton(); // Current approach relies on skeleton for mesh hierarchy

    // Check that file loaded correctly
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
    {
        logError(QStringLiteral("ERROR::ASSIMP::") +  m_importer.GetErrorString());
        return nullptr;
    }

    // Process nodes
    processNodes(m_scene->mRootNode);

    // Load materials
    loadMaterials();

    // Load meshes
    loadMeshes();

    // Load animations
    loadAnimations();

    // Assign animation metadata to skeleton
    postProcessSkeleton();

    // Assign node animation data to each animation
    //for (const auto& animation : m_animations) {
    //    for (const auto& joint : ModelReader::model()->skeleton()->nodes()) {
    //        animation->m_nameToIndex[joint.second->getName()] = joint.second->skeletonTransformIndex();
    //    }
    //}

    return model;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadMaterials()
{
    std::vector<aiMaterial*> materials;
    if (m_scene->mNumMaterials > 0) {
        materials = std::vector<aiMaterial*>(m_scene->mMaterials, m_scene->mMaterials + m_scene->mNumMaterials);
    }

    QString dir = FileReader::dirFromPath(m_filePath);
    for (size_t i = 0; i < m_scene->mNumMaterials; i++) {
        // Obtain material data
        aiMaterial* aiMaterial = m_scene->mMaterials[i];
        MaterialData materialData = processMaterial(aiMaterial);

        // Construct material filepath
        QString mtlName = QString::fromStdString(materialData.m_name);
        QString materialPath = QDir::cleanPath(dir + QDir::separator() + mtlName);
        materialData.m_path = materialPath.toStdString();

        // Create material and handle
        std::shared_ptr<Material> mtl = model()->addMaterial(mtlName);
        mtl->handle()->setIsLoading(true); // Flag loading for post-construction

        // Populate material data
        mtl->setData(std::move(materialData));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadMeshes()
{
    // Convert to custom Mesh type
    for(unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh *mesh = m_scene->mMeshes[i]; 
        const Mesh& meshData = processMesh(mesh);
        m_meshNames[i] = meshData.getName();
    }

    // Set skeleton name and global inverse transform
    model()->skeleton()->setGlobalInverseTransform(m_globalInverseTransform);

    // Walk scene tree to construct mesh hierarchy
    model()->skeleton()->addRootNode(m_scene->mRootNode->mName.C_Str());
    parseNodeHierarchy(m_scene->mRootNode, model()->skeleton()->m_root);

    // Generate bind pose for the skeleton
    model()->skeleton()->constructInverseBindPose();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadAnimations()
{
    // Create animations
    unsigned int loopSize = m_scene->mNumAnimations;
    //for (unsigned int i = 0; i < loopSize; i++) {
    //    const aiAnimation* aiAnimation = m_scene->mAnimations[i];
    //    QString animationName = aiAnimation->mName.C_Str();
    //    Map::Emplace(m_animations, animationName, std::make_shared<Animation>(animationName));
    //}

    // Populate animations
    ParallelLoopGenerator loop(nullptr, USE_THREADING);
    loop.parallelFor(loopSize, [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            const aiAnimation* aiAnimation = m_scene->mAnimations[i];
            QString animationName = aiAnimation->mName.C_Str();
            //Map::Emplace(m_animations, animationName, std::make_shared<Animation>(animationName));

            // Create handle for animation
            auto animHandle = model()->addAnimation(animationName)->handle();
            animHandle->setIsLoading(true);

            // Create animation
            std::shared_ptr<Animation> animation = animHandle->resourceAs<Animation>();
            m_animations.push_back(animation);
            animation->m_ticksPerSecond = aiAnimation->mTicksPerSecond;
            animation->m_durationInTicks = aiAnimation->mDuration;
            std::vector<float>& animationTimes = animation->m_times;

            // Resize to number of bones, not channels, since not all animations are actually
            // used to impact the shader results, i.e., not all animations make it into a bone ID
            // or bone weight vertex attribute
            std::vector<NodeAnimation>& nodeAnimations = animation->m_nodeAnimations;

            // NOTE: Assumes that animated nodes are the same for each animation
            // Each channel is a node (and possible bone) with all of its transformations
            if (!m_processedSkeleton) {
                for (unsigned int n = 0; n < aiAnimation->mNumChannels; n++) {
                    // Mark that nodes are animated
                    const aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[n];
                    QString nodeName = aiNodeAnim->mNodeName.C_Str();
                    SkeletonJoint* node = model()->skeleton()->getNode(nodeName);
                    node->setAnimated(true);
                }

                // Set node transform indices
                postProcessSkeleton();
            }

            // Iterate through channels (nodes) of the animation for actual load
            // Each channel is a node (and possible bone) with all of its transformations
            nodeAnimations.resize(aiAnimation->mNumChannels);
            for (unsigned int n = 0; n < aiAnimation->mNumChannels; n++) {
                const aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[n];
                QString nodeName = aiNodeAnim->mNodeName.C_Str();
                SkeletonJoint* node = model()->skeleton()->getNode(nodeName);
                int boneIndex = node->bone().m_index;
                nodeAnimations[node->skeletonTransformIndex()] = NodeAnimation();

                size_t maxKeys = std::max(aiNodeAnim->mNumPositionKeys,
                    std::max(aiNodeAnim->mNumRotationKeys,
                        aiNodeAnim->mNumScalingKeys));
                animationTimes.reserve(maxKeys);

                // Construct vector of unique animation times
                // Note: This is done since we cannot assume that animation key frames will always
                // have a translation, rotation, and scaling (although this is usually the case)
                for (unsigned int j = 0; j < maxKeys; j++) {
                    if (aiNodeAnim->mNumPositionKeys > j) {
                        Vec::EmplaceBack(animationTimes, aiNodeAnim->mPositionKeys[j].mTime);
                    }
                    if (aiNodeAnim->mNumRotationKeys > j) {
                        Vec::EmplaceBack(animationTimes, aiNodeAnim->mRotationKeys[j].mTime);
                    }
                    if (aiNodeAnim->mNumScalingKeys > j) {
                        Vec::EmplaceBack(animationTimes, aiNodeAnim->mScalingKeys[j].mTime);
                    }
                }

                // Tally which nodes have bones in the animation
                if (node->hasBone()) {
                    animation->m_boneIndices[nodeName] = boneIndex;
                }
                else {
                    animation->m_boneIndices[nodeName] = -1;
                    animation->m_numNonBones++;
                }


                // Sort and remove non-unique entries from animation times
                // Must sort before removal
                std::sort(animationTimes.begin(), animationTimes.end());
                animationTimes.erase(
                    std::unique(animationTimes.begin(),
                        animationTimes.end()), animationTimes.end());
            }

            // TODO: Figure out a performant way to avoid including translation
            // and scaling if they are not used
            // Iterate through all time-steps to create skeletal poses
            for (unsigned int n = 0; n < aiAnimation->mNumChannels; n++) {
                const aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[n];
                QString nodeName = aiNodeAnim->mNodeName.C_Str();
                SkeletonJoint* node = model()->skeleton()->getNode(nodeName);
                NodeAnimation& nodeAnimation = nodeAnimations[node->skeletonTransformIndex()];

                // Convert arrays to vectors
                std::vector<aiVectorKey> translations(aiNodeAnim->mPositionKeys,
                    aiNodeAnim->mPositionKeys + aiNodeAnim->mNumPositionKeys);
                std::vector<aiQuatKey> rotations(aiNodeAnim->mRotationKeys,
                    aiNodeAnim->mRotationKeys + aiNodeAnim->mNumRotationKeys);
                std::vector<aiVectorKey> scales(aiNodeAnim->mScalingKeys,
                    aiNodeAnim->mScalingKeys + aiNodeAnim->mNumScalingKeys);

                // Match key frames with animation times
                nodeAnimation.translations().reserve(animationTimes.size());
                nodeAnimation.rotations().reserve(animationTimes.size());
                nodeAnimation.scales().reserve(animationTimes.size());
                for (unsigned int j = 0; j < animationTimes.size(); j++) {
                    float time = animationTimes[j];
                    // Add translation if there is one
                    auto transIter = std::find_if(translations.begin(), translations.end(),
                        [&](const aiVectorKey& t) {
                        return qFuzzyCompare((float)t.mTime, time);
                    });
                    if (transIter != translations.end()) {
                        const aiVector3D& aiTranslation = transIter->mValue;
                        Vec::EmplaceBack(nodeAnimation.translations(),
                            aiTranslation.x, aiTranslation.y, aiTranslation.z);
                    }
                    else {
                        // INTERPOLATE
                        // Get the upper and lower bound iterators
                        aiVectorKey timeKey;
                        timeKey.mTime = time;
                        auto larger = std::upper_bound(translations.begin(), translations.end(),
                            timeKey, [&](const aiVectorKey& t1, const aiVectorKey& t2) {
                            return t1.mTime < t2.mTime;
                        });
                        if (larger == translations.end()) {
                            // Decrement if larger reached the end of the vector
                            --larger;
                        }
                        auto smaller = larger;
                        if (larger != translations.begin()) {
                            // Decrement smaller when larger is not the start of the vector
                            --smaller;
                        }
                        Vector3g start(smaller->mValue.x, smaller->mValue.y, smaller->mValue.z);
                        Vector3g end(larger->mValue.x, larger->mValue.y, larger->mValue.z);
                        float startTime = (float)smaller->mTime;
                        float endTime = (float)larger->mTime;
                        float weight = (time - startTime) / (endTime - startTime);
                        weight = startTime == endTime ? 0.0f : weight;
                        Vector3g res = Interpolation::lerp(start, end, weight);

                        Vec::EmplaceBack(nodeAnimation.translations(), res);

                    }

                    // Add rotation if there is one
                    auto rotIter = std::find_if(rotations.begin(), rotations.end(),
                        [&](const aiQuatKey& r) {
                        return qFuzzyCompare((float)r.mTime, time);
                    });
                    if (rotIter != rotations.end()) {
                        const aiQuaternion& rotation = rotIter->mValue;
                        Vec::EmplaceBack(nodeAnimation.rotations(), 
                            rotation.x, rotation.y, rotation.z, rotation.w);
                    }
                    else {
                        // INTERPOLATE
                        // Get the upper and lower bound iterators
                        aiQuatKey timeKey;
                        timeKey.mTime = time;
                        auto larger = std::upper_bound(rotations.begin(), rotations.end(),
                            timeKey, [&](const aiQuatKey& t1, const aiQuatKey& t2) {
                            return t1.mTime < t2.mTime;
                        });
                        if (larger == rotations.end()) {
                            // Decrement if larger reached the end of the vector
                            --larger;
                        }
                        auto smaller = larger;
                        if (larger != rotations.begin()) {
                            // Decrement smaller when larger is not the start of the vector
                            --smaller;
                        }
                        Quaternion start(smaller->mValue.x, smaller->mValue.y, smaller->mValue.z, smaller->mValue.w);
                        Quaternion end(larger->mValue.x, larger->mValue.y, larger->mValue.z, larger->mValue.w);
                        float startTime = (float)smaller->mTime;
                        float endTime = (float)larger->mTime;
                        float weight = (time - startTime) / (endTime - startTime);
                        weight = startTime == endTime ? 0.0f : weight;
                        Quaternion res = Quaternion::slerp(start, end, weight);

                        Vec::EmplaceBack(nodeAnimation.rotations(), res);
                    }

                    // Add scale if there is one
                    auto scaleIter = std::find_if(scales.begin(), scales.end(),
                        [&](const aiVectorKey& s) {
                        return qFuzzyCompare((float)s.mTime, time);
                    });
                    if (scaleIter != scales.end()) {
                        const aiVector3D& aiScale = scaleIter->mValue;
                        Vec::EmplaceBack(nodeAnimation.scales(), 
                            aiScale.x, aiScale.y, aiScale.z);
                    }
                    else {
                        // INTERPOLATE
                        // Get the upper and lower bound iterators
                        aiVectorKey timeKey;
                        timeKey.mTime = time;
                        auto larger = std::upper_bound(scales.begin(), scales.end(),
                            timeKey, [&](const aiVectorKey& t1, const aiVectorKey& t2) {
                            return t1.mTime < t2.mTime;
                        });
                        if (larger == scales.end()) {
                            // Decrement if larger reached the end of the vector
                            --larger;
                        }
                        auto smaller = larger;
                        if (larger != scales.begin()) {
                            // Decrement smaller when larger is not the start of the vector
                            --smaller;
                        }
                        Vector3g start(smaller->mValue.x, smaller->mValue.y, smaller->mValue.z);
                        Vector3g end(larger->mValue.x, larger->mValue.y, larger->mValue.z);
                        float startTime = (float)smaller->mTime;
                        float endTime = (float)larger->mTime;
                        float weight = (time - startTime) / (endTime - startTime);
                        weight = startTime == endTime ? 0.0f : weight;

                        Vec::EmplaceBack(nodeAnimation.scales(),
                            Interpolation::lerp(start, end, weight));
                    }
                }
            }
        }
    });

    // Add to resource cache
    //for (const std::pair<QString, std::shared_ptr<Animation>>& animationPair : m_animations) {
    //    m_resourceCache->addAnimation(animationPair.second);
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Mesh& ModelReader::processMesh(aiMesh * mesh)
{
    // Create sub-mesh
    QString meshName = QString::fromStdString(mesh->mName.C_Str());
    if (meshName.isEmpty()) {
        meshName = Uuid().createUniqueName("mesh_");
    }
    
    // Create handle for mesh resource
    std::shared_ptr<Mesh> newMesh = model()->addMesh(meshName);
    newMesh->handle()->setIsLoading(true); // Flag loading for post-construction
    VertexArrayData& meshData = newMesh->m_vertexData;

    // Initialize limits for bounding box of the mesh
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Populate vertices, texture coordinates, and normals from assimp meshes
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        // Get data for vertex
        const aiVector3D& aiVert = mesh->mVertices[i];
        const aiVector3D& aiNormal = mesh->mNormals[i];
        const aiVector3D& aiTexCoords = mesh->mTextureCoords[0][i]; // TexCoords are stored as array of array, for multiple coords per vertex

        // Update min/max vertex data
        minX = std::min(aiVert.x, minX);
        maxX = std::max(aiVert.x, maxX);
        minY = std::min(aiVert.y, minY);
        maxY = std::max(aiVert.y, maxY);
        minZ = std::min(aiVert.z, minZ);
        maxZ = std::max(aiVert.z, maxZ);

        // Append vertex data to new mesh
        Vec::EmplaceBack(meshData.m_attributes.m_vertices, aiVert.x, aiVert.y, aiVert.z);
        Vec::EmplaceBack(meshData.m_attributes.m_normals, aiNormal.x, aiNormal.y, aiNormal.z);
        Vec::EmplaceBack(meshData.m_attributes.m_texCoords, aiTexCoords.x, aiTexCoords.y);

        // Append tangents as well if they were generated
        if (mesh->mTangents) {
            const aiVector3D& aiTangent = mesh->mTangents[i];
            Vec::EmplaceBack(meshData.m_attributes.m_tangents, aiTangent.x, aiTangent.y, aiTangent.z);
        }

    }

    // Populate indices
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++) {
            meshData.m_indices.push_back(face.mIndices[j]);
        }
    }

    // Create chunk to add to model, matching to material info
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = m_scene->mMaterials[mesh->mMaterialIndex];
        QString materialName = QString(mat->GetName().C_Str()).toLower();
        auto matHandle = model()->getMaterial(materialName);
        auto meshHandle = m_resourceCache->getHandle(newMesh->handle()->getUuid());

        // Set bounding box for chunk
        AABB box;
        box.setMinX(minX);
        box.setMaxX(maxX);
        box.setMinY(minY);
        box.setMaxY(maxY);
        box.setMinZ(minZ);
        box.setMaxZ(maxZ);

        // Add to internal list of chunks
        m_chunks.push_back({ meshHandle->getUuid(),
            matHandle->getUuid(),
            box });
    }
    else {
        throw("Error, chunk not created, material not found");
    }

    // Load bone vertex data
    if (mesh->mNumBones > 0) {
        // Initialize vertex attributes for bones
        std::vector<uint> counts(mesh->mNumVertices); // count of number of weights per bone
        meshData.m_attributes.m_miscInt.resize(mesh->mNumVertices);
        meshData.m_attributes.m_miscReal.resize(mesh->mNumVertices);

        unsigned int boneIndex = 0; 
        for (uint i = 0; i < mesh->mNumBones; i++) {
            // Get bone and name
            const aiBone* bone = mesh->mBones[i];
            QString boneName(mesh->mBones[i]->mName.data);

            // Check that bone is a node
            std::vector<QString>::const_iterator node_it = std::find(m_nodeNames.begin(),
                m_nodeNames.end(),
                boneName);
            if (node_it == m_nodeNames.end()) {
                logError("Error: Bone " + boneName + " from file" + m_filePath + "not found in scene nodes!\n");
            }

            // Cache bone in map
            m_boneMapping[boneName] = new Bone(boneName, boneIndex);
            m_boneMapping[boneName]->m_offsetMatrix = toMatrix(bone->mOffsetMatrix);

            // Iterate through bone weights
            for (unsigned int w = 0; w < bone->mNumWeights; w++) {
                const aiVertexWeight& weight = bone->mWeights[w];
                uint count = counts[weight.mVertexId]; 
                uint vertexID = weight.mVertexId;
                meshData.m_attributes.m_miscInt[vertexID][count] = boneIndex;
                meshData.m_attributes.m_miscReal[vertexID][count] = weight.mWeight;
                counts[weight.mVertexId]++;
            }

            boneIndex++;
        }

    }

    return *newMesh;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MaterialData ModelReader::processMaterial(aiMaterial * material)
{
    MaterialData materialData;
    QString matName = QString(material->GetName().C_Str());
    materialData.m_name = matName.toStdString();

    // Read in properties
    aiColor3D ambientColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor)) {
        logError("Failed to load ambient color property of material");
    }
    else {
        Vector3g color = Vector3g(ambientColor.r, ambientColor.g, ambientColor.b);;
        materialData.m_properties.m_ambient = color;
    }

    aiColor3D diffuseColor = aiColor3D(0.0f, 0.0f, 0.0f);
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
        logError("Failed to load diffuse color property of material");
    }
    else {
        Vector3g color = Vector3g(diffuseColor.r, diffuseColor.g, diffuseColor.b);
        materialData.m_properties.m_diffuse = color;
    }

    aiColor3D specularColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
        logError("Failed to load specular color property of material");
    }
    else {
        Vector3g color = Vector3g(specularColor.r, specularColor.g, specularColor.b);
        materialData.m_properties.m_specularity = color;
    }

    aiColor3D emissiveColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor)) {
        logError("Failed to load emissive color property of material");
    }
    else {
        Vector3g color = Vector3g(emissiveColor.r, emissiveColor.g, emissiveColor.b);
        materialData.m_properties.m_emission = color;
    }

    aiColor3D transparentColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor)) {
        logError("Failed to load transparent color property of material");
    }
    else {
        Vector3g color = Vector3g(transparentColor.r, transparentColor.g, transparentColor.b);
        materialData.m_properties.m_transmittance = color;
    }

    float shininess = 1.0;
    if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS, shininess)) {
        logError("Failed to load shininess property of material");
    }
    else {
        materialData.m_properties.m_shininess = std::max(1.0f, shininess);
    }

    float ior = 1.0;
    if (AI_SUCCESS != material->Get(AI_MATKEY_REFRACTI, ior)) {
        logError("Failed to load index of refraction property of material");
    }
    else {
        materialData.m_properties.m_ior = ior;
    }

    float opacity = 1.0;
    if (AI_SUCCESS != material->Get(AI_MATKEY_OPACITY, ior)) {
        logError("Failed to load opacity property of material");
    }
    else {
        materialData.m_properties.m_dissolve = opacity;
    }


    // Read in textures
    processTexture(material, materialData, aiTextureType_AMBIENT);
    processTexture(material, materialData, aiTextureType_DIFFUSE);
    processTexture(material, materialData, aiTextureType_NORMALS);
    processTexture(material, materialData, aiTextureType_SPECULAR);
    processTexture(material, materialData, aiTextureType_HEIGHT);
    processTexture(material, materialData, aiTextureType_SHININESS);
    processTexture(material, materialData, aiTextureType_OPACITY);
    processTexture(material, materialData, aiTextureType_DISPLACEMENT);
    processTexture(material, materialData, aiTextureType_REFLECTION);
    processTexture(material, materialData, aiTextureType_LIGHTMAP);

    // PBR textures
    processTexture(material, materialData, aiTextureType_BASE_COLOR);
    processTexture(material, materialData, aiTextureType_NORMAL_CAMERA);;
    processTexture(material, materialData, aiTextureType_EMISSION_COLOR);
    processTexture(material, materialData, aiTextureType_METALNESS);
    processTexture(material, materialData, aiTextureType_DIFFUSE_ROUGHNESS);
    processTexture(material, materialData, aiTextureType_AMBIENT_OCCLUSION);

    return materialData;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g ModelReader::calculateGlobalNodeTransform(const aiNode * root)
{
    // Iterate up node tree
    Matrix4x4g ret;
    const aiNode* node = root;
    while (node != nullptr) {
        Matrix4x4g nodeTransform = toMatrix(node->mTransformation);
        ret = nodeTransform * ret;
        node = node->mParent;
    }

    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::processTexture(aiMaterial * mat, MaterialData& outMaterial, aiTextureType type)
{
    TextureData texData;
    if (mat->GetTextureCount(type) == 0) {
        // Return if no textures of the specified type
        return;
    }

    // Populate texture filepath 
    aiString texPath;
    // TODO: Implement remaining texture settings
    mat->GetTexture(type, 0, &texPath);

    QString texPathStr = QString(texPath.C_Str());
    texData.m_textureFileName = FileReader::pathToName(texPathStr, true, true).toStdString();
    //texData.m_textureFileDir = FileReader::dirFromPath(texPathStr).toStdString();

    if (texData.m_textureFileName.empty()) {
        logWarning("Warning, no texture name found");
    }

    if (mat->GetTextureCount(type) > 1) {
        logWarning("Warning, multiple textures of the same type are not handled");
    }

    // Add texture data to material data
    switch (type) {
    case aiTextureType_AMBIENT:
        outMaterial.m_ambientTexture = texData;
        break;
    case aiTextureType_DIFFUSE:
        outMaterial.m_diffuseTexture = texData;
        break;
    case aiTextureType_NORMALS:
        outMaterial.m_normalTexture = texData;
        break;
    case aiTextureType_SPECULAR:
        outMaterial.m_specularTexture = texData;
        break;
    case aiTextureType_HEIGHT:
        outMaterial.m_bumpTexture = texData;
        break;
    case aiTextureType_SHININESS:
        outMaterial.m_specularHighlightTexture = texData;
        break;
    case aiTextureType_OPACITY:
        outMaterial.m_opacityTexture = texData;
        break;
    case aiTextureType_DISPLACEMENT:
        outMaterial.m_displacementTexture = texData;
        break;
    case aiTextureType_REFLECTION:
        outMaterial.m_reflectionTexture = texData;
        break;
    case aiTextureType_LIGHTMAP:
        outMaterial.m_lightmapTexture = texData;
        break;

    // PBR textures
    case aiTextureType_BASE_COLOR:
        outMaterial.m_albedoTexture = texData;
        break;
    case aiTextureType_NORMAL_CAMERA:
        outMaterial.m_pbrBumpMapping = texData;
        break;
    case aiTextureType_EMISSION_COLOR:
        outMaterial.m_emissiveTexture = texData;
        break;
    case aiTextureType_METALNESS:
        outMaterial.m_metallicTexture = texData;
        break;
    case aiTextureType_DIFFUSE_ROUGHNESS:
        outMaterial.m_roughnessTexture = texData;
        break;
    case aiTextureType_AMBIENT_OCCLUSION:
        outMaterial.m_ambientOcclusionTexture = texData;
        break;
    default:
        logError("Texture type not recognized");
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::processNodes(const aiNode* node)
{
    if (node == nullptr) return;
    QString nodeName = QString(node->mName.C_Str());
    m_nodeNames.push_back(nodeName);
    if (node->mParent != nullptr) {
        //QString parentName = QString(node->mParent->mName.C_Str());
        //m_nodeParents[nodeName] = parentName;
    }
    else {
        // Is root node, set global inverse transformation
        m_globalInverseTransform = toMatrix(m_scene->mRootNode->mTransformation).inversed();
    }
    //m_nodeTransforms[nodeName] = calculateNodeTransform(node);
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        this->processNodes(node->mChildren[i]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::parseNodeHierarchy(aiNode * assimpNode, SkeletonJoint * currentNode)
{
    QString nodeName(assimpNode->mName.C_Str());
    uint nameCount = std::count(m_nodeNames.begin(), m_nodeNames.end(), nodeName);
    if (nodeName.isEmpty() || nameCount > 1) {
        // Create unique name if name is duplicated, OR
        // Create unique node name if empty (will not be empty if referenced by a bone or animation)
        nodeName = Uuid().createUniqueName("node_");
    }
    if (assimpNode->mParent) {
        // Set name of mesh node if not root (root's name is set already)
        currentNode->m_name = nodeName;
    }

    // Get transform of the node and set
    aiMatrix4x4 assimpNodeTransform = assimpNode->mTransformation;
    currentNode->m_transform = toMatrix(assimpNodeTransform);

    // Add bone (if any)
    if (Map::HasKey(m_boneMapping, nodeName)) {
        currentNode->m_bone = *m_boneMapping[nodeName];
        currentNode->m_skeleton->m_boneNodes.push_back(currentNode);
    }

    // Iterate through child meshes and add to current node
    for (size_t i = 0; i < assimpNode->mNumMeshes; i++) {
        size_t meshInt = assimpNode->mMeshes[i];
        QString meshName = m_meshNames[meshInt];
        //VertexArrayData* data = mesh.m_vertexData.find(meshName)->second;
        //currentNode->addMeshData(data);
    }

    // Recursively add child nodes and their meshes
    for (size_t i = 0; i < assimpNode->mNumChildren; i++) {
        aiNode* aiChildNode = assimpNode->mChildren[i];
        currentNode->addChild(aiChildNode->mName.C_Str());
        parseNodeHierarchy(aiChildNode, currentNode->m_children.back());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::postProcessSkeleton()
{
    if (m_scene->mNumAnimations == 0) { return; }

    m_animatedJointCount = 0;
    postProcessSkeleton(model()->skeleton()->root());
    m_processedSkeleton = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::postProcessSkeleton(SkeletonJoint * joint)
{
    if (joint->isAnimated()) {
        joint->setSkeletonTransformIndex(m_animatedJointCount++);
        
    }
    for (SkeletonJoint* child : joint->children()) {
        postProcessSkeleton(child);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g ModelReader::toMatrix(const aiMatrix4x4 & mat)
{
    Matrix4x4g matrix(std::vector<Vector4g>{ 
        {mat.a1, mat.b1, mat.c1, mat.d1}, 
        {mat.a2, mat.b2, mat.c2, mat.d2}, 
        {mat.a3, mat.b3, mat.c3, mat.d3},
        {mat.a4, mat.b4, mat.c4, mat.d4} });
    return matrix;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> ModelReader::model()
{
    return m_handle->resourceAs<Model>();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
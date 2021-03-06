#include "GModelReader.h"

#include <algorithm> 
#include "../../containers/GContainerExtensions.h"
#include "../../resource/GResourceCache.h"
#include "../../resource/GFileManager.h"
#include "../../rendering/models/GModel.h"
#include "../../rendering/geometry/GSkeleton.h"
#include "../../rendering/renderer/GMainRenderer.h"
#include "../../threading/GParallelLoop.h"
#include "../../animation/GAnimation.h"
#include "../../utils/GInterpolation.h"

#include <QElapsedTimer>

#include "GOBJReader.h"
#include "../../GCoreEngine.h"
#include "../../scene/GScenario.h"

#define USE_THREADING false

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ModelReader::ModelReader(ResourceCache* cache, ResourceHandle& handle) :
    FileReader(handle.getPath().c_str()),
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
rev::ModelReader::~ModelReader()
{
    // Delete cached bones
    for (const std::pair<GString, Bone*>& bonePair: m_boneMapping) {
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
Model* ModelReader::loadModel()
{
    // Instantiate model
    std::unique_ptr<Model> model_unique = std::make_unique<Model>(Model::ModelFlags());
    m_handle->setName(GString(FileReader::PathToName(m_handle->getPath().c_str())));
    m_handle->setResource(std::move(model_unique), false);
    m_handle->setIsLoading(true);
    Model* model = m_handle->resourceAs<Model>();
    model->addSkeleton(m_resourceCache->engine()); // Current approach relies on skeleton for mesh hierarchy

    // TODO: Remove, once done optionally
    // TODO: Implement this, need to actually generate MDL "offline" for this to work
    // Get scenario path, set directory based on that
    //const GString& scenarioPath = m_resourceCache->engine()->scenario()->getPath();
    //GString modelDir = QDir::cleanPath(
    //    FileReader::DirFromPath(scenarioPath) + QDir::separator() +
    //    "resources" + QDir::separator() + "models");

    GString extension = FileReader::FileExtension(m_handle->getPath().c_str());
    if (extension != "mdl") {
        // If resource handle points to a non-binary format, see if MDL file exists
        // TODO: Move model file to subdirectory of scenario path

        // Set handle and child paths
        GString modelPath(m_handle->getPath());
        modelPath.replace("." + extension, ".mdl");

        // Set handle and model names to reflect path change
        m_handle->setChildPaths(modelPath);
        GString modelName = m_handle->getName();
        modelName.replace("." + extension, ".mdl");
        m_handle->setName(modelName);

        if (FileReader::FileExists(modelPath.c_str())) {
            // Skip loading through ASSIMP if a MDL file exists in same location
            model->loadBinary(modelPath);
            return model;
        } 
        
        // Get name of file with ".mdl" extensition substituted
        QString outPath;
        GString fileName = PathToName(m_handle->getPath().c_str(), true, true);
        fileName.replace("." + extension, ".mdl");
        //GString modelPath = QDir::cleanPath(modelDir + QDir::separator() + 
        //    fileName.c_str());
        if (m_handle->engine()->fileManager()->searchFor(fileName.c_str(), outPath)){
            // Skip loading through ASSIMP if a MDL file exists in search paths
            model->loadBinary(outPath);
            return model;
        }
    }
    else{
        model->loadBinary(m_handle->getPath());
        return model;
    }

    // Use Assimp to load model, since not serialized to my own binary format
    loadAssimp();

    return model;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadAssimp()
{
    Model* model = m_handle->resourceAs<Model>();

    // Load file as ASSIMP scene
    loadFile();

    // Set model as animated
    if (m_scene->mNumAnimations > 0) {
        model->setAnimated(true);
    }

    // Check that file loaded correctly
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
    {
        logError(QStringLiteral("ERROR::ASSIMP::") + m_importer.GetErrorString());
        m_handle->setResource(nullptr, false);
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

    // TODO: REMOVE, done optionally
    // Serialize model as binary and reference that instead
    if (!m_handle->getPath().asLower().contains(".mdl")) {
        throw("Error, extension not set for MDL file");
    }
    model->saveBinary(m_handle->getPath());

    // Iterate through chunks of model and set
    postProcessModel();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadMaterials()
{
    m_materialData.clear();
    m_materialData.reserve(m_scene->mNumMaterials);

    QString dir = FileReader::DirFromPath(m_filePath);
    for (size_t i = 0; i < m_scene->mNumMaterials; i++) {
        // Obtain material data
        aiMaterial* aiMaterial = m_scene->mMaterials[i];
        Vec::EmplaceBack(m_materialData, processMaterial(aiMaterial));
        MaterialData& materialData = m_materialData.back();

        // Construct material filepath
        GString mtlName(materialData.m_name.c_str());
        GString materialPath = QDir::cleanPath(dir + QDir::separator() + mtlName.c_str());
        materialData.m_path = materialPath;

        // Create material from data
        Material* mat = createMaterial(materialData);
        m_matHandles.push_back(mat->handle()->sharedPtr());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadMeshes()
{
    // Convert to custom Mesh type
    //m_meshNames.clear();
    for(unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh *mesh = m_scene->mMeshes[i]; 
        processMesh(mesh);
    }

    // Set skeleton name and global inverse transform
    Skeleton& skeleton = *model()->skeleton();
    skeleton.setGlobalInverseTransform(m_globalInverseTransform);

    // Walk scene tree to construct mesh hierarchy
    skeleton.addRootNode(m_scene->mRootNode->mName.C_Str());
    parseNodeHierarchy(m_scene->mRootNode, skeleton.root()->getIndex(skeleton));

    // Generate bind pose for the skeleton
    skeleton.constructInverseBindPose();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::loadAnimations()
{
    // Create animations
    unsigned int loopSize = m_scene->mNumAnimations;

    // Populate animations
    // Parallel for loop is disabled, not a good idea to call from non-main threads
    ParallelLoopGenerator loop(nullptr, USE_THREADING);
    loop.parallelFor(loopSize, [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            const aiAnimation* aiAnimation = m_scene->mAnimations[i];
            GString animationName = aiAnimation->mName.C_Str();

            // Create handle for animation
            auto animHandle = model()->addAnimation(m_resourceCache->engine(), animationName)->handle();
            animHandle->setIsLoading(true);

            // Create animation
            Animation* animation = animHandle->resourceAs<Animation>();
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
                    GString nodeName = aiNodeAnim->mNodeName.C_Str();
                    SkeletonJoint& node = model()->skeleton()->getNode(nodeName);
                    node.setAnimated(true);
                }

                // Set node transform indices
                postProcessSkeleton();
            }

            // Iterate through channels (nodes) of the animation for actual load
            // Each channel is a node (and possible bone) with all of its transformations
            nodeAnimations.resize(aiAnimation->mNumChannels);
            for (unsigned int n = 0; n < aiAnimation->mNumChannels; n++) {
                const aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[n];
                GString nodeName = aiNodeAnim->mNodeName.C_Str();
                SkeletonJoint& node = model()->skeleton()->getNode(nodeName);
                //int boneIndex = node.bone().m_index;
                nodeAnimations[node.skeletonTransformIndex()] = NodeAnimation();

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
                if (node.hasBone()) {
                    //animation->m_boneIndices[nodeName] = boneIndex;
                }
                else {
                    //animation->m_boneIndices[nodeName] = -1;
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
                GString nodeName = aiNodeAnim->mNodeName.C_Str();
                SkeletonJoint& node = model()->skeleton()->getNode(nodeName);
                NodeAnimation& nodeAnimation = nodeAnimations[node.skeletonTransformIndex()];

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
                        Vector3 start(smaller->mValue.x, smaller->mValue.y, smaller->mValue.z);
                        Vector3 end(larger->mValue.x, larger->mValue.y, larger->mValue.z);
                        float startTime = (float)smaller->mTime;
                        float endTime = (float)larger->mTime;
                        float weight = (time - startTime) / (endTime - startTime);
                        weight = startTime == endTime ? 0.0f : weight;
                        Vector3 res = Interpolation::lerp(start, end, weight);

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
                        Quaternion res = Quaternion::Slerp(start, end, weight);

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
                        Vector3 start(smaller->mValue.x, smaller->mValue.y, smaller->mValue.z);
                        Vector3 end(larger->mValue.x, larger->mValue.y, larger->mValue.z);
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

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Mesh& ModelReader::processMesh(aiMesh * mesh)
{
    // Create sub-mesh
    GString meshName = mesh->mName.C_Str();
    if (meshName.isEmpty()) {
        meshName = Uuid().createUniqueName("mesh_");
    }
    
    // Create handle for mesh resource
    Mesh* newMesh = model()->addMesh(m_resourceCache->engine(), meshName);
    newMesh->handle()->setIsLoading(true); // Flag loading for post-construction
    m_meshHandles.push_back(newMesh->handle()->sharedPtr());

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
    size_t meshIdx = m_meshHandles.size() - 1; // Mesh was just added to handles vec
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = m_scene->mMaterials[mesh->mMaterialIndex];
        GString materialName = GString(mat->GetName().C_Str()).asLower();
        //auto matHandle = model()->getMaterial(materialName);
        //auto meshHandle = m_resourceCache->getHandle(newMesh->handle()->getUuid());

        // Add to internal list of chunks
        m_chunks.emplace_back();

        // Set material and mesh indices for chunk
        m_chunks.back().m_meshHandleIndex = meshIdx;

        auto matIter = std::find_if(m_matHandles.begin(), m_matHandles.end(),
            [&](const std::shared_ptr<ResourceHandle>& matHandle) {
            return matHandle->getName() == materialName;
        });
        if (matIter == m_matHandles.end()) {
            throw("Error, mat handle not found");
        }
        m_chunks.back().m_matHandleIndex = matIter - m_matHandles.begin();

        // Set bounding box for chunk
        AABBData& box = m_chunks.back().m_boundingBox;
        box.setMinX(minX);
        box.setMaxX(maxX);
        box.setMinY(minY);
        box.setMaxY(maxY);
        box.setMinZ(minZ);
        box.setMaxZ(maxZ);
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
            GString boneName(mesh->mBones[i]->mName.data);

            // Check that bone is a node
            std::vector<GString>::const_iterator node_it = std::find(m_nodeNames.begin(),
                m_nodeNames.end(),
                boneName);
            if (node_it == m_nodeNames.end()) {
                logError("Error: Bone " + QString(boneName.c_str()) + " from file" + m_filePath + "not found in scene nodes!\n");
            }

            // Cache bone in map
            m_boneMapping[boneName] = new Bone(boneIndex);
            m_boneMapping[boneName]->m_invBindPose = toMatrix(bone->mOffsetMatrix);

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
    materialData.m_name = material->GetName().C_Str();

    // Read in properties
    aiColor3D ambientColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor)) {
        logError("Failed to load ambient color property of material");
    }
    else {
        Vector3 color = Vector3(ambientColor.r, ambientColor.g, ambientColor.b);;
        materialData.m_properties.m_ambient = color;
    }

    aiColor3D diffuseColor = aiColor3D(0.0f, 0.0f, 0.0f);
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
        logError("Failed to load diffuse color property of material");
    }
    else {
        Vector3 color = Vector3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
        materialData.m_properties.m_diffuse = color;
    }

    aiColor3D specularColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor)) {
        logError("Failed to load specular color property of material");
    }
    else {
        Vector3 color = Vector3(specularColor.r, specularColor.g, specularColor.b);
        materialData.m_properties.m_specularity = color;
    }

    aiColor3D emissiveColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor)) {
        logError("Failed to load emissive color property of material");
    }
    else {
        Vector3 color = Vector3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
        materialData.m_properties.m_emission = color;
    }

    aiColor3D transparentColor;
    if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor)) {
        logError("Failed to load transparent color property of material");
    }
    else {
        Vector3 color = Vector3(transparentColor.r, transparentColor.g, transparentColor.b);
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
Material* ModelReader::createMaterial(const MaterialData & data)
{
    // Create material and handle
    Material* mtl = model()->addMaterial(m_resourceCache->engine(), data.m_name.c_str());
    mtl->handle()->setIsLoading(true); // Flag loading for post-construction

    // Populate material data
    mtl->setData(data);

    return mtl;
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

    GString texPathStr = GString(texPath.C_Str());
    GString fileName = FileReader::PathToName(texPathStr.c_str(), true, true);
    memcpy(texData.m_textureFileName.data(), fileName.c_str(), fileName.length());
    texData.m_textureFileName[fileName.length()] = 0;
    //texData.m_textureFileDir = FileReader::DirFromPath(texPathStr).toStdString();

    if (GString(texData.m_textureFileName.data()).isEmpty()) {
        logWarning("Warning, no texture name found");
    }

    if (mat->GetTextureCount(type) > 1) {
        logWarning("Warning, multiple textures of the same type are not handled");
    }

    // Add texture data to material data
    switch (type) {
    case aiTextureType_AMBIENT:
        outMaterial.m_textureData.m_ambientTexture = texData;
        break;
    case aiTextureType_DIFFUSE:
        outMaterial.m_textureData.m_diffuseTexture = texData;
        break;
    case aiTextureType_NORMALS:
        outMaterial.m_textureData.m_normalTexture = texData;
        break;
    case aiTextureType_SPECULAR:
        outMaterial.m_textureData.m_specularTexture = texData;
        break;
    case aiTextureType_HEIGHT:
        outMaterial.m_textureData.m_bumpTexture = texData;
        break;
    case aiTextureType_SHININESS:
        outMaterial.m_textureData.m_specularHighlightTexture = texData;
        break;
    case aiTextureType_OPACITY:
        outMaterial.m_textureData.m_opacityTexture = texData;
        break;
    case aiTextureType_DISPLACEMENT:
        outMaterial.m_textureData.m_displacementTexture = texData;
        break;
    case aiTextureType_REFLECTION:
        outMaterial.m_textureData.m_reflectionTexture = texData;
        break;
    case aiTextureType_LIGHTMAP:
        outMaterial.m_textureData.m_lightmapTexture = texData;
        break;

    // PBR textures
    case aiTextureType_BASE_COLOR:
        outMaterial.m_textureData.m_albedoTexture = texData;
        break;
    case aiTextureType_NORMAL_CAMERA:
        outMaterial.m_textureData.m_pbrBumpMapping = texData;
        break;
    case aiTextureType_EMISSION_COLOR:
        outMaterial.m_textureData.m_emissiveTexture = texData;
        break;
    case aiTextureType_METALNESS:
        outMaterial.m_textureData.m_metallicTexture = texData;
        break;
    case aiTextureType_DIFFUSE_ROUGHNESS:
        outMaterial.m_textureData.m_roughnessTexture = texData;
        break;
    case aiTextureType_AMBIENT_OCCLUSION:
        outMaterial.m_textureData.m_ambientOcclusionTexture = texData;
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
    GString nodeName = GString(node->mName.C_Str());
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
void ModelReader::parseNodeHierarchy(aiNode * assimpNode, uint32_t currentNodeIndex)
{
    Skeleton& skeleton = *model()->skeleton();
    GString nodeName(assimpNode->mName.C_Str());
    uint nameCount = std::count(m_nodeNames.begin(), m_nodeNames.end(), nodeName);
    SkeletonJoint& currentNode = skeleton.getNode(currentNodeIndex);
    if (nodeName.isEmpty() || nameCount > 1) {
        // Create unique name if name is duplicated, OR
        // Create unique node name if empty (will not be empty if referenced by a bone or animation)
        nodeName = Uuid().createUniqueName("node_");
    }
    if (assimpNode->mParent) {
        // Set name of mesh node if not root (root's name is set already)
        currentNode.m_name = nodeName;
    }

    // Get transform of the node and set
    aiMatrix4x4 assimpNodeTransform = assimpNode->mTransformation;
    currentNode.m_transform.m_localMatrix = toMatrix(assimpNodeTransform);

    // Add bone (if any)
    if (Map::HasKey(m_boneMapping, nodeName)) {
        currentNode.m_bone = *m_boneMapping[nodeName];
        skeleton.m_boneNodes.push_back(currentNodeIndex);
    }

    // Recursively add child nodes and their meshes
    // Note: currentNode is invalidated once new children are added to skeleton
    for (size_t i = 0; i < assimpNode->mNumChildren; i++) {
        aiNode* aiChildNode = assimpNode->mChildren[i];
        size_t childIndex = skeleton.addChild(currentNodeIndex, aiChildNode->mName.C_Str());
        parseNodeHierarchy(aiChildNode, childIndex);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::postProcessSkeleton()
{
    if (m_scene->mNumAnimations == 0) { return; }

    m_animatedJointCount = 0;
    postProcessSkeleton(*model()->skeleton()->root());
    m_processedSkeleton = true;

    // Already done, somewhere...
    // Mark skeleton as done loading
    //model()->skeleton()->handle()->setConstructed(true);
    //model()->skeleton()->handle()->setIsLoading(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::postProcessSkeleton(SkeletonJoint& joint)
{
    if (joint.isAnimated()) {
        joint.setSkeletonTransformIndex(m_animatedJointCount++);
        
    }
    for (const size_t& childIndex : joint.children()) {
        postProcessSkeleton(model()->skeleton()->getNode(childIndex));
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModelReader::postProcessModel()
{
    // Create model chunks and add them to the model
    for (const ModelChunkData& chunkData : m_chunks) {
        const std::shared_ptr<ResourceHandle>& meshHandle = m_meshHandles[chunkData.m_meshHandleIndex];
        const std::shared_ptr<ResourceHandle>& matHandle = m_matHandles[chunkData.m_matHandleIndex];

        Vec::EmplaceBack(model()->m_chunks,
            meshHandle,
            matHandle
        );
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g ModelReader::toMatrix(const aiMatrix4x4 & mat)
{
    Matrix4x4g matrix(std::vector<Vector4>{ 
        {mat.a1, mat.b1, mat.c1, mat.d1}, 
        {mat.a2, mat.b2, mat.c2, mat.d2}, 
        {mat.a3, mat.b3, mat.c3, mat.d3},
        {mat.a4, mat.b4, mat.c4, mat.d4} });
    return matrix;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Model* ModelReader::model()
{
    return m_handle->resourceAs<Model>();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
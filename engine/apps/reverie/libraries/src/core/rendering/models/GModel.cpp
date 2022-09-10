#include "core/rendering/models/GModel.h"

#include "fortress/encoding/binary/GSerializationProtocol.h"
#include "fortress/json/GJson.h"
#include "fortress/streams/GFileStream.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/layer/framework/GFlags.h"
#include "core/readers/models/GModelReader.h"
#include "core/resource/GResourceHandle.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

namespace rev {



// Model

std::shared_ptr<ResourceHandle> Model::CreateHandle(CoreEngine * engine)
{
    auto handle = ResourceHandle::Create(engine, (GResourceType)EResourceType::eModel);
    return handle;
}

std::shared_ptr<ResourceHandle> Model::CreateHandle(CoreEngine * engine,
    const GString & modelName,
    ModelFlags flags)
{
    // Create handle for material
    auto handle = ResourceHandle::Create(engine, (GResourceType)EResourceType::eModel);
    handle->setRuntimeGenerated(true); // model is generated in engine
    handle->setName(modelName);

    // Create model
    auto model = std::make_unique<Model>(flags);
    handle->setResource(std::move(model), false);

    return handle;
}

Model::Model():
    m_modelFlags(0)
{
}

Model::Model(ModelFlags flags):
    m_modelFlags(flags)
{
}

Model::Model(ResourceHandle& handle, const nlohmann::json& json) :
    Resource(),
    m_modelFlags(0)
{
    m_handle = &handle;
    json.get_to(*this);
}

Model::Model(ResourceHandle& handle) :
    Resource(),
    m_modelFlags(0)
{
    m_handle = &handle;
}

Model::~Model()
{
}

Skeleton* Model::skeleton() const
{
    if (!m_skeletonHandle) {
        return nullptr;
    }
    else {
        return m_skeletonHandle->resourceAs<Skeleton>();
    }
}

bool Model::isAnimated() const
{
    return Flags<ModelFlag>(m_modelFlags).testFlag(kIsAnimated);
}

bool Model::isStatic() const
{
    return !isAnimated();
}

void Model::setAnimated(bool animated)
{
    ModelFlags flags = Flags<ModelFlag>(m_modelFlags);
    flags.setFlag(Model::ModelFlag::kIsAnimated, animated);
    m_modelFlags = (uint32_t)flags;
}

//void Model::createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
//    Camera & camera, 
//    ShaderProgram & shader)
//{
//}

const std::shared_ptr<ResourceHandle>& Model::getMaterial(const GString& name)
{
    const std::shared_ptr<ResourceHandle>& matHandle = m_handle->getChild(name, EResourceType::eMaterial);
    if (!matHandle) {
        Logger::Throw("Error, material handle not found as child of model");
    }
    return matHandle;
}

void Model::getChildren(std::vector<std::shared_ptr<ResourceHandle>>& outMaterials,
    std::vector<std::shared_ptr<ResourceHandle>>& outMeshes,
    std::vector<std::shared_ptr<ResourceHandle>>& outAnimations) const
{
    for (const auto& child : m_handle->children()) {
        if (child->getResourceType() == EResourceType::eMaterial) {
            outMaterials.push_back(child);
        }
        else if (child->getResourceType() == EResourceType::eMesh) {
            outMeshes.push_back(child);
        }
        else if (child->getResourceType() == EResourceType::eAnimation) {
            outAnimations.push_back(child);
        }
    }
}

Mesh* Model::addMesh(CoreEngine* core, const GString & meshName)
{
    // Create handle for mesh
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::Create(core,
        m_handle->getPath(),
        EResourceType::eMesh,
        flags);
    handle->setName(meshName);

    // Create mesh
    auto mesh = std::make_unique<Mesh>();
    handle->setResource(std::move(mesh), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Mesh>();
}

Material* Model::addMaterial(CoreEngine* core, const GString & mtlName)
{
    // Create handle for material
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::Create(core,
        m_handle->getPath(),
        EResourceType::eMaterial,
        flags);
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_unique<Material>();
    mtl->initializeUniformValues(core->openGlRenderer()->renderContext().uniformContainer());
    handle->setResource(std::move(mtl), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Material>();
}

Animation* Model::addAnimation(CoreEngine* core, const GString & animationName)
{
    // Create handle for material
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::Create(core,
        m_handle->getPath(),
        EResourceType::eAnimation,
        flags);
    handle->setName(animationName);

    // Create animation
    auto anim = std::make_unique<Animation>();
    handle->setResource(std::move(anim), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Animation>();
}

void Model::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}

void Model::addSkeleton(CoreEngine* core)
{
    if (m_skeletonHandle) {
        Logger::Throw("Error, skeleton already found");
    }
    GString skellyName = m_handle->getName() + "_skeleton";
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    m_skeletonHandle = ResourceHandle::Create(core, EResourceType::eSkeleton, flags);
    m_skeletonHandle->setName(skellyName);
    m_skeletonHandle->setResource(std::make_unique<Skeleton>(), false);
    m_skeletonHandle->setIsLoading(true);
    m_handle->addChild(m_skeletonHandle);
}

void Model::postConstruction(const ResourcePostConstructionData& /*postConstructData*/)
{
    // Load from resource JSON if specified
    if (!m_handle->cachedResourceJson().empty()) {
        /// @fixme Added this flag check, but not sure if this is an actual fix
        ResourceBehaviorFlags flags = m_handle->cachedResourceJson()["flags"].get<Int32_t>();
        if (flags.testFlag(ResourceBehaviorFlag::kRuntimeGenerated)) {
            m_handle->cachedResourceJson().get_to(*this);
        }
    }
}

//bool Model::loadBinary(const GString& filepath)
//{
//    // TODO: Clean this up, see what's actually needed from reader
//    // Create model reader for storage
//    ModelReader reader(&ResourceCache::Instance(), *m_handle);
//
//    // Open file
//    FileStream fileStream(filepath);
//    fileStream.open(FileAccessMode::kRead | FileAccessMode::kBinary);
//
//    // Construct header protocol for read
//    uint32_t numMaterials;
//    uint32_t numMeshes;
//    SerializationProtocol<uint32_t, uint32_t, uint32_t, std::vector<ModelChunkData>> headerProtocol(
//        numMaterials,
//        numMeshes,
//        m_modelFlags.value(),
//        reader.m_chunks);
//    headerProtocol << fileStream;
//
//    // Create skeleton header protocol
//    Skeleton& skeleton = *Model::skeleton();
//    uint32_t numJoints = 0;
//    auto skeletonHeaderProtocol = SerializationProtocol<Matrix4x4g, std::vector<uint32_t>, std::vector<Matrix4x4g>, uint32_t>(
//        skeleton.m_globalInverseTransform,
//        skeleton.m_boneNodes,
//        skeleton.m_inverseBindPose,
//        numJoints);
//    skeletonHeaderProtocol << fileStream;
//
//    // Create skeleton sub-protocol
//    skeleton.m_nodes.resize(numJoints);
//    for (uint32_t i = 0; i < numJoints; i++) {
//        SkeletonJoint& joint = skeleton.m_nodes[i];
//        //GStringProtocolField nameField = joint.m_name.createProtocolField();
//        joint.m_name << fileStream;
//
//        auto jointProtocol = SerializationProtocol< //char*, uint32_t, 
//            Bone, int, uint32_t, TransformMatrices<Matrix4x4>, int, std::vector<uint32_t>>(
//                //nameField,
//                //joint.m_name.length(),
//                joint.m_bone,
//                joint.m_skeletonTransformIndex,
//                joint.m_jointFlags,
//                joint.m_transform,
//                joint.m_parent,
//                joint.m_children
//                );
//        jointProtocol << fileStream;
//    }
//
//    // Generate bounding box for the skeleton
//    skeleton.generateBoundingBox();
//
//    // Materials
//    for (uint32_t i = 0; i < numMaterials; i++) {
//        // Load in material data
//        MaterialData materialData;
//        materialData.m_name << fileStream;
//        materialData.m_path << fileStream;
//
//        auto matProtocol = SerializationProtocol<int32_t, MaterialProperties, std::vector<TextureData>>(
//            materialData.m_id,
//            materialData.m_properties,
//            materialData.m_textureData.m_data
//            );
//
//        matProtocol << fileStream;
//
//        // NOTE: Don't need to load sprite-sheet data, models don't have it
//        // as of 3/18/2021
//
//        // Create material from data
//        Material* mat = reader.createMaterial(materialData);
//        reader.m_matHandles.push_back(mat->handle()->sharedPtr());
//    }
//
//    // Mesh layout
//    VertexAttributesLayout vertexLayout;
//    auto layoutProtocol = SerializationProtocol<std::array<VertexAttributeInfo, 4>>(vertexLayout.m_layout);
//    layoutProtocol << fileStream;
//
//    // TODO: Accomodate different layouts
//    // Verify that MeshVertexAttributes layout has not changed
//    if (VertexAttributesLayout::CurrentLayout().m_layout != vertexLayout.m_layout) {
//        Logger::Throw("Error, vertex attributes layout does not match that found in file");
//    }
//
//    // Meshes
//    if (reader.m_meshHandles.size()) {
//        Logger::Throw("Should not have any mesh handles loaded");
//    }
//    for (uint32_t i = 0; i < numMeshes; i++) {
//        // Create handle for mesh resource
//        Mesh& mesh = *addMesh(m_handle->engine(), "");
//        mesh.handle()->setIsLoading(true); // Flag loading for post-construction
//        reader.m_meshHandles.push_back(mesh.handle()->sharedPtr());
//
//        // Parse in mesh name
//        GString name;
//        name << fileStream;
//        mesh.handle()->setName(name);
//
//        // Create protocol for parsing in mesh data
//        //GStringProtocolField stringField(mesh.getName().createProtocolField());
//        auto meshProtocol = SerializationProtocol<
//            //char*,
//            //uint32_t,
//            gl::BufferStorageMode,
//            std::vector<Vector2>,
//            std::vector<Vector3>,
//            std::vector<Vector3>,
//            std::vector<Vector3>,
//            std::vector<Vector4>,
//            std::vector<Vector4>,
//            std::vector<Vector4i>,
//            std::vector<GLuint>>(
//                //stringField,
//                //mesh.getName().length(),
//                mesh.vertexData().m_usagePattern,
//                mesh.vertexData().m_attributes.m_texCoords,
//                mesh.vertexData().m_attributes.m_vertices,
//                mesh.vertexData().m_attributes.m_normals,
//                mesh.vertexData().m_attributes.m_tangents,
//                mesh.vertexData().m_attributes.m_colors,
//                mesh.vertexData().m_attributes.m_miscReal,
//                mesh.vertexData().m_attributes.m_miscInt,
//                mesh.vertexData().m_attributes.m_indices
//                );
//        meshProtocol << fileStream;
//    }
//
//    // Load animations
//    if (isAnimated()) {
//
//        // Get number of animations
//        uint32_t numAnimations = reader.m_animations.size();
//        SerializationProtocol<uint32_t> animationsProtocol(numAnimations);
//        animationsProtocol << fileStream;
//
//        // Iterate to load animations
//        for (uint32_t i = 0; i < numAnimations; i++) {
//
//            // Create handle for animation
//            auto animHandle = addAnimation(m_handle->engine(), "")->handle();
//            animHandle->setIsLoading(true);
//            Animation& animation = *animHandle->resourceAs<Animation>();
//
//            // Read animation name
//            GString name;
//            name << fileStream;
//            animHandle->setName(name);
//
//            // Read miscellaneous animation attributes
//            uint32_t numNodeAnimations;
//            auto animationProtocol = SerializationProtocol<
//                double,
//                double,
//                std::vector<float>,
//                uint32_t,
//                uint32_t>(
//                    animation.m_ticksPerSecond,
//                    animation.m_durationInTicks,
//                    animation.m_times,
//                    animation.m_numNonBones,
//                    numNodeAnimations
//                    );
//            animationProtocol << fileStream;
//
//            // Read node animations
//            for (uint32_t j = 0; j < numNodeAnimations; j++) {
//                animation.m_nodeAnimations.emplace_back();
//                NodeAnimation& nodeAnimation = animation.m_nodeAnimations.back();
//
//                auto nodeAnimProtocol = SerializationProtocol<
//                    std::vector<Vector3>,
//                    std::vector<Quaternion>,
//                    std::vector<Vector3>>(
//                        nodeAnimation.translations(),
//                        nodeAnimation.rotations(),
//                        nodeAnimation.scales());
//                nodeAnimProtocol << fileStream;
//            }
//        }
//    }
//
//    // Read into protocol
//    //protocol << fileStream;
//
//    // Close file
//    fileStream.close();
//
//    // Get all child resources, organized by type, to match chunk data indexing
//    std::vector<std::shared_ptr<ResourceHandle>> materials, meshes, animations;
//    getChildren(materials, meshes, animations);
//
//    // Iterate through chunks of model and set
//    //reader.postProcessModel();
//    for (const ModelChunkData& chunkData : reader.m_chunks) {
//        const std::shared_ptr<ResourceHandle>& meshHandle = meshes[chunkData.m_meshHandleIndex];
//        const std::shared_ptr<ResourceHandle>& matHandle = materials[chunkData.m_matHandleIndex];
//
//        Vec::EmplaceBack(m_chunks,
//            meshHandle,
//            matHandle
//        );
//    }
//
//    return true;
//}
//
//bool Model::saveBinary(const GString& filepath) const
//{
//    FileStream fileStream(filepath);
//    fileStream.open(FileAccessMode::kWrite | FileAccessMode::kBinary);
//
//    // Create Protocols ---------------------------------------------------
//
//    // Get all child resources, organized by type
//    std::vector<std::shared_ptr<ResourceHandle>> materials, meshes, animations;
//    getChildren(materials, meshes, animations);
//
//    // Lump everything into one SerializationProtocol
//
//    // Need metadata to describe number of child protocols
//    uint32_t numMaterials = (uint32_t)materials.size();
//    uint32_t numMeshes = (uint32_t)meshes.size();
//    uint32_t modelFlags = (uint32_t)m_modelFlags;
//
//    // Create chunk data to serialize model chunks
//    std::vector<ModelChunkData> chunkData;
//    for (const ModelChunk& chunk : m_chunks) {
//        // Get index of mesh in list of mesh resources
//        auto meshIter = std::find_if(meshes.begin(), meshes.end(), 
//            [&](const auto& meshHandle) {return meshHandle->getUuid() == chunk.meshHandle()->getUuid(); });
//        if (meshIter == meshes.end()) {
//            Logger::Throw("Error, mesh not found");
//        }
//        uint32_t meshHandleIndex = meshIter - meshes.begin();
//
//        // Get index of material in list of material resources
//        auto matIter = std::find_if(materials.begin(), materials.end(),
//            [&](const auto& matHandle) {return matHandle->getUuid() == chunk.materialHandle()->getUuid(); });
//        if (matIter == materials.end()) {
//            Logger::Throw("Error, material not found");
//        }
//        uint32_t matHandleIndex = matIter - materials.begin();
//
//        chunkData.emplace_back(ModelChunkData{
//            meshHandleIndex, matHandleIndex, chunk.mesh()->objectBounds().boxData() });
//    }
//
//    SerializationProtocol<uint32_t, uint32_t, uint32_t, std::vector<ModelChunkData>> protocol(
//        numMaterials,
//        numMeshes,
//        modelFlags,
//        chunkData);
//
//    // Skeleton
//    Skeleton& skeleton = *Model::skeleton();
//    uint32_t numJoints = skeleton.m_nodes.size();
//    auto skeletonProtocol = std::make_unique<SerializationProtocol<Matrix4x4g, std::vector<uint32_t>, std::vector<Matrix4x4g>, uint32_t>>(
//        skeleton.m_globalInverseTransform,
//        skeleton.m_boneNodes,
//        skeleton.m_inverseBindPose,
//        numJoints);
//    for (SkeletonJoint& joint : skeleton.m_nodes) {
//        GStringProtocolField nameField = joint.m_name.createProtocolField();
//        auto jointProtocol = std::make_unique<SerializationProtocol<char*, Bone, int, uint32_t, TransformMatrices<Matrix4x4>, int, std::vector<uint32_t>>>(
//            nameField,
//            //joint.m_name.length(),
//            joint.m_bone,
//            joint.m_skeletonTransformIndex,
//            joint.m_jointFlags,
//            joint.m_transform,
//            joint.m_parent,
//            joint.m_children
//            );
//
//        // Add joint protocol to skeleton protocol
//        skeletonProtocol->addChild(std::move(jointProtocol));
//    }
//    protocol.addChild(std::move(skeletonProtocol));
//
//    // Materials
//    std::vector<MaterialData> tempMatData;
//    for (const auto& materialHandle : materials) {
//        tempMatData.emplace_back();
//        MaterialData& materialData = tempMatData.back();
//        auto matProtocol = materialHandle->resourceAs<Material>()->createProtocol(materialData);
//        protocol.addChild(std::move(matProtocol));
//    }
//
//    // Mesh layout
//    VertexAttributesLayout vertexLayout;
//    auto layoutProtocol = std::make_unique<SerializationProtocol<std::array<VertexAttributeInfo, 4>>>(vertexLayout.m_layout);
//    protocol.addChild(std::move(layoutProtocol));
//
//    // Meshes
//    for (const std::shared_ptr<ResourceHandle>& meshHandle : meshes) {
//        Mesh& mesh = *meshHandle->resourceAs<Mesh>();
//        GStringProtocolField stringField(meshHandle->getName().createProtocolField());
//        auto meshProtocol = std::make_unique<SerializationProtocol<
//            char*,
//            //uint32_t, 
//            gl::BufferStorageMode,
//            std::vector<Vector2>,
//            std::vector<Vector3>,
//            std::vector<Vector3>,
//            std::vector<Vector3>,
//            std::vector<Vector4>,
//            std::vector<Vector4>,
//            std::vector<Vector4i>,
//            std::vector<GLuint>>>(
//                stringField,
//                //mesh.getName().length(),
//                mesh.vertexData().m_usagePattern,
//                mesh.vertexData().m_attributes.m_texCoords,
//                mesh.vertexData().m_attributes.m_vertices,
//                mesh.vertexData().m_attributes.m_normals,
//                mesh.vertexData().m_attributes.m_tangents,
//                mesh.vertexData().m_attributes.m_colors,
//                mesh.vertexData().m_attributes.m_miscReal,
//                mesh.vertexData().m_attributes.m_miscInt,
//                mesh.vertexData().m_attributes.m_indices
//                );
//        protocol.addChild(std::move(meshProtocol));
//    }
//
//    // If animated, create protocols
//    if (isAnimated()) {
//        uint32_t numAnimations = (uint32_t)animations.size();
//        auto animationsProtocol = std::make_unique<SerializationProtocol<Uint32_t>>(numAnimations);
//        for (const auto& animationHandle : animations) {
//            Animation& animation = *animationHandle->resourceAs<Animation>();
//
//            GStringProtocolField nameField(animation.handle()->getName().createProtocolField());
//            uint32_t numNodeAnimations = (uint32_t)animation.m_nodeAnimations.size();
//            auto animationProtocol = std::make_unique<SerializationProtocol<
//                char*,
//                double,
//                double,
//                std::vector<float>,
//                uint32_t,
//                uint32_t>>(
//                    nameField,
//                    animation.m_ticksPerSecond,
//                    animation.m_durationInTicks,
//                    animation.m_times,
//                    animation.m_numNonBones,
//                    numNodeAnimations
//                    );
//
//            // Add sub-children for node animations
//            for (NodeAnimation& nodeAnimation : animation.m_nodeAnimations) {
//                auto nodeAnimProtocol = std::make_unique<SerializationProtocol<
//                    std::vector<Vector3>,
//                    std::vector<Quaternion>,
//                    std::vector<Vector3>>>(
//                        nodeAnimation.translations(),
//                        nodeAnimation.rotations(),
//                        nodeAnimation.scales());
//                animationProtocol->addChild(std::move(nodeAnimProtocol));
//            }
//
//            animationsProtocol->addChild(std::move(animationProtocol));
//        }
//
//        protocol.addChild(std::move(animationsProtocol));
//    }
//
//    // Write binary to file --------------------------------------------
//    protocol.write(fileStream);
//
//    fileStream.close();
//
//    return true;
//}

void to_json(json& orJson, const Model& korObject)
{
    //orJson["name"] = korObject.m_name.c_str();
    //orJson["type"] = korObject.m_modelType;
    orJson["flags"] = (int)korObject.m_modelFlags;

    // Don't need to cache other renderable settings, e.g. uniforms
    //orJson["renderSettings"] = korObject.m_renderSettings.asJson();

    // Add meshes, materials, animations, skeleton
    json meshes = json::array();
    json materials = json::array();
    json animations = json::array();
    for (const auto& handle : korObject.m_handle->children()) {
        if (handle->getResourceType() == EResourceType::eMesh) {
            meshes.push_back(handle->getName().c_str());
        }
        else if (handle->getResourceType() == EResourceType::eMaterial) {
            materials.push_back(handle->getName().c_str());
        }
        else if (handle->getResourceType() == EResourceType::eAnimation) {
            animations.push_back(handle->getName().c_str());
        }
        else if (handle->getResourceType() == EResourceType::eSkeleton) {
            orJson["skeleton"] = handle->getName().c_str();
        }
        else {
            Logger::Throw("Invalid or unimplemented resource type in model JSON");
        }
    }
    orJson["meshes"] = meshes;
    orJson["materials"] = materials;
    orJson["animations"] = animations;
}

void from_json(const json& korJson, Model& orObject)
{
    if (korJson.contains("type")) {
        // Legacy, deprecated
        orObject.m_modelFlags = korJson.at("type").get<Int32_t>();
    }
    else {
        orObject.m_modelFlags = korJson.at("flags").get<Int32_t>();
    }

    //m_name = object["name"].toString();
    //m_handle->setName(m_name); // Good for names to match for intuition

    // Clear current attributes
    if (orObject.m_handle->children().size()) {
        orObject.m_handle->children().clear();
    }

    orObject.m_chunks.clear();
    if (korJson.contains("meshes")) {
        // Legacy load method, deprecated
        orObject.loadChunksFromJson(korJson);
    }

    // Add animations
    const json& animations = korJson["animations"];
    for (const json& animName : animations) {
        GString name = animName.get_ref<const std::string&>().c_str();
        auto handle = ResourceCache::Instance().getHandleWithName(name, EResourceType::eAnimation);
#ifdef DEBUG_MODE
        if (!handle) Logger::Throw("Error, handle not found for given name");
#endif
        orObject.m_handle->addChild(handle);
    }

    // Add skeleton
    if (korJson.contains("skeleton")) {
        GString skeletonName = korJson["skeleton"].get_ref<const std::string&>().c_str();
        auto handle = ResourceCache::Instance().getHandleWithName(skeletonName,
            EResourceType::eSkeleton);
        orObject.m_skeletonHandle = handle;
#ifdef DEBUG_MODE
        if (!handle) Logger::Throw("Error, handle not found for given name");
#endif
        orObject.m_handle->addChild(handle);
    }
}

bool Model::checkMesh(const std::shared_ptr<ResourceHandle>& handle)
{
    Mesh* meshPtr = handle->resourceAs<Mesh>();

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        Logger::LogInfo("Mesh is still loading, return");
#endif
        return false;
    }

    if (!handle->isConstructed()) {
#ifdef DEBUG_MODE
        Logger::Throw("Mesh not yet constructed, returning");
#endif
        return false;
    }

    if (!meshPtr) {
#ifdef DEBUG_MODE
        Logger::Throw("No mesh found, returning");
#endif
        return false;
    }

    if (!(handle->getResourceType() == EResourceType::eMesh)) {
        Logger::Throw("Error, incorrect resource type assigned to mesh");
    }

    return true;
}

bool Model::checkMaterial(const std::shared_ptr<ResourceHandle>& handle)
{
    Material* matPtr = handle->resourceAs<Material>();

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        Logger::LogInfo("Material is still loading, return");
#endif
        return false;
    }

    if (!handle->isConstructed()) {
#ifdef DEBUG_MODE
        Logger::Throw("Material not yet constructed, returning");
#endif
        return false;
    }

    if (!matPtr) {
#ifdef DEBUG_MODE
        Logger::Throw("No material found, returning");
#endif
        return false;
    }

    if (!(handle->getResourceType() == EResourceType::eMaterial)) {
        Logger::Throw("Error, incorrect resource type assigned to material");
    }

    return true;
}

void Model::loadChunksFromJson(const json & object)
{
    // Get meshes
    const json& meshes = object["meshes"];
    std::vector<std::shared_ptr<ResourceHandle>> meshHandles;
    ResourceCache& cache = ResourceCache::Instance();
    for (const json& meshID : meshes) {
        GString name = meshID.get_ref<const std::string&>().c_str();
        auto handle = cache.getHandleWithName(name, EResourceType::eMesh);
#ifdef DEBUG_MODE
        if (!handle) {
            Logger::Throw("Error, handle not found for given name");
        }
#endif
        m_handle->addChild(handle);
        meshHandles.push_back(handle);
    }

    // Get materials
    const json& materials = object["materials"];
    std::vector<std::shared_ptr<ResourceHandle>> matHandles;
    for (const json& matID : materials) {
        GString name = matID.get_ref<const std::string&>().c_str();
        auto handle = cache.getHandleWithName(name, EResourceType::eMaterial);
#ifdef DEBUG_MODE
        if (!handle) {
            Logger::Throw("Error, handle not found for given name");
        }
#endif
        m_handle->addChild(handle);
        matHandles.push_back(handle);
    }

    // Load meshes and materials as chunks
    uint32_t i = 0;
    for (i = 0; i < meshHandles.size(); i++) {
        Vec::EmplaceBack(m_chunks, meshHandles[i], matHandles[i]);
    
        // Generate bounding boxes from the model chunk mesh
        Mesh* mesh = meshHandles[i]->resourceAs<Mesh>();
        if (!mesh) {
            Logger::Throw("Error, expecting mesh to be loaded");
        }
        //m_chunks.back().setObjectBounds(mesh->objectBounds());
    }
}





// End namespaces
}
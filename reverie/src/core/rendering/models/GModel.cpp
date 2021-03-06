#include "GModel.h"

#include "../../readers/GFileStream.h"
#include "../../utils/GMemoryManager.h"
#include "../../readers/models/GModelReader.h"
#include "../../resource/GResource.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../readers/GJsonReader.h"
#include "../../rendering/materials/GMaterial.h"
#include "../../rendering/renderer/GRenderContext.h"
#include "../../rendering/lighting/GLightSettings.h"
#include "../../containers/GFlags.h"
#include "../../encoding/GProtocol.h"
#include "../shaders/GShaderProgram.h"
#include "../geometry/GSkeleton.h"
#include "../geometry/GBuffers.h"
#include "../geometry/GMesh.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Model
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Model::CreateHandle(CoreEngine * engine)
{
    auto handle = ResourceHandle::create(engine, ResourceType::kModel);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Model::CreateHandle(CoreEngine * engine,
    const GString & modelName,
    ModelFlags flags)
{
    // Create handle for material
    auto handle = ResourceHandle::create(engine, ResourceType::kModel);
    handle->setRuntimeGenerated(true); // model is generated in engine
    handle->setName(modelName);

    // Create model
    auto model = std::make_unique<Model>(flags);
    handle->setResource(std::move(model), false);

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model():
    m_modelFlags(0)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(ModelFlags flags):
    m_modelFlags(flags)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(ResourceHandle& handle, const QJsonValue& json) :
    Resource(),
    m_modelFlags(0)
{
    m_handle = &handle;
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(ResourceHandle& handle) :
    Resource(),
    m_modelFlags(0)
{
    m_handle = &handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Skeleton* Model::skeleton() const
{
    if (!m_skeletonHandle) {
        return nullptr;
    }
    else {
        return m_skeletonHandle->resourceAs<Skeleton>();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool Model::isAnimated() const
{
    return Flags<ModelFlag>::toQFlags(m_modelFlags).testFlag(kIsAnimated);
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool Model::isStatic() const
{
    return !isAnimated();
}
///////////////////////////////////////////////////////////////////////////////////////////////
void Model::setAnimated(bool animated)
{
    ModelFlags flags = Flags<ModelFlag>::toQFlags(m_modelFlags);
    flags.setFlag(Model::ModelFlag::kIsAnimated, animated);
    m_modelFlags = (uint32_t)flags;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void Model::createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
//    Camera & camera, 
//    ShaderProgram & shader)
//{
//}
/////////////////////////////////////////////////////////////////////////////////////////////
const std::shared_ptr<ResourceHandle>& Model::getMaterial(const GString& name)
{
    const std::shared_ptr<ResourceHandle>& matHandle = m_handle->getChild(name, ResourceType::kMaterial);
    if (!matHandle) {
        throw("Error, material handle not found as child of model");
    }
    return matHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::getChildren(std::vector<std::shared_ptr<ResourceHandle>>& outMaterials,
    std::vector<std::shared_ptr<ResourceHandle>>& outMeshes,
    std::vector<std::shared_ptr<ResourceHandle>>& outAnimations) const
{
    for (const auto& child : m_handle->children()) {
        if (child->getResourceType() == ResourceType::kMaterial) {
            outMaterials.push_back(child);
        }
        else if (child->getResourceType() == ResourceType::kMesh) {
            outMeshes.push_back(child);
        }
        else if (child->getResourceType() == ResourceType::kAnimation) {
            outAnimations.push_back(child);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Mesh* Model::addMesh(CoreEngine* core, const GString & meshName)
{
    // Create handle for mesh
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::create(core,
        m_handle->getPath(),
        ResourceType::kMesh,
        flags);
    handle->setName(meshName);

    // Create mesh
    auto mesh = std::make_unique<Mesh>();
    handle->setResource(std::move(mesh), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Mesh>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material* Model::addMaterial(CoreEngine* core, const GString & mtlName)
{
    // Create handle for material
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::create(core,
        m_handle->getPath(),
        ResourceType::kMaterial,
        flags);
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_unique<Material>();
    handle->setResource(std::move(mtl), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Material>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Animation* Model::addAnimation(CoreEngine* core, const GString & animationName)
{
    // Create handle for material
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    auto handle = ResourceHandle::create(core,
        m_handle->getPath(),
        ResourceType::kAnimation,
        flags);
    handle->setName(animationName);

    // Create animation
    auto anim = std::make_unique<Animation>();
    handle->setResource(std::move(anim), false);

    // Add handle as child
    m_handle->addChild(handle);

    return handle->resourceAs<Animation>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::addSkeleton(CoreEngine* core)
{
    if (m_skeletonHandle) {
        throw("Error, skeleton already found");
    }
    GString skellyName = m_handle->getName() + "_skeleton";
    ResourceBehaviorFlags flags;
    flags.setFlag(ResourceBehaviorFlag::kChild, true);
    m_skeletonHandle = ResourceHandle::create(core, ResourceType::kSkeleton, flags);
    m_skeletonHandle->setName(skellyName);
    m_skeletonHandle->setResource(std::make_unique<Skeleton>(), false);
    m_skeletonHandle->setIsLoading(true);
    m_handle->addChild(m_skeletonHandle);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::postConstruction()
{
    // Load from resource JSON if specified
    if (!m_handle->resourceJson().isEmpty()) {
        loadFromJson(m_handle->resourceJson());
    }

    // Set bounding box for each chunk's mesh
    for (const ModelChunk& chunk : m_chunks) {
        if (!chunk.mesh()->objectBounds().boxData().isInitialized()) {
            chunk.mesh()->generateBounds();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::loadBinary(const GString& filepath)
{
    // TODO: Clean this up, see what's actually needed from reader
    // Create model reader for storage
    ModelReader reader(m_handle->engine()->resourceCache(), *m_handle);

    // Open file
    FileStream fileStream(filepath);
    fileStream.open(FileAccessMode::kRead | FileAccessMode::kBinary);

    // Construct header protocol for read
    uint32_t numMaterials;
    uint32_t numMeshes;
    Protocol<uint32_t, uint32_t, uint32_t, std::vector<ModelChunkData>> headerProtocol(
        numMaterials,
        numMeshes,
        m_modelFlags.value(),
        reader.m_chunks);
    headerProtocol << fileStream;

    // Construct model protocol
    //BaseProtocol protocol;

    // Create skeleton header protocol
    Skeleton& skeleton = *Model::skeleton();
    uint32_t numJoints = 0;
    auto skeletonHeaderProtocol = Protocol<Matrix4x4g, std::vector<uint32_t>, std::vector<Matrix4x4g>, uint32_t>(
        skeleton.m_globalInverseTransform,
        skeleton.m_boneNodes,
        skeleton.m_inverseBindPose,
        numJoints);
    skeletonHeaderProtocol << fileStream;

    // Create skeleton sub-protocol
    //auto* skeletonProtocol = new BaseProtocol();
    skeleton.m_nodes.resize(numJoints);
    for (uint32_t i = 0; i < numJoints; i++) {
        SkeletonJoint& joint = skeleton.m_nodes[i];
        //GStringProtocolField nameField = joint.m_name.createProtocolField();
        joint.m_name << fileStream;

        auto jointProtocol = Protocol< //char*, uint32_t, 
            Bone, int, uint32_t, TransformMatrices, int, std::vector<uint32_t>>(
                //nameField,
                //joint.m_name.length(),
                joint.m_bone,
                joint.m_skeletonTransformIndex,
                joint.m_jointFlags,
                joint.m_transform,
                joint.m_parent,
                joint.m_children
                );
        jointProtocol << fileStream;
    }

    // Generate bounding box for the skeleton
    skeleton.generateBoundingBox();

    // Materials
    reader.m_materialData.resize(numMaterials);
    for (uint32_t i = 0; i < numMaterials; i++) {
        // Load in material data
        MaterialData& materialData = reader.m_materialData[i];
        materialData.m_name << fileStream;
        materialData.m_path << fileStream;

        auto matProtocol = Protocol<int32_t, MaterialProperties, MaterialTextureData>(
            materialData.m_id,
            materialData.m_properties,
            materialData.m_textureData
            );

        matProtocol << fileStream;

        // Create material from data
        Material* mat = reader.createMaterial(materialData);
        reader.m_matHandles.push_back(mat->handle()->sharedPtr());
    }

    // Mesh layout
    VertexAttributesLayout vertexLayout;
    auto layoutProtocol = Protocol<std::vector<VertexAttributeInfo>>(vertexLayout.m_layout);
    layoutProtocol << fileStream;

    // TODO: Accomodate different layouts
    // Verify that VertexAttributes layout has not changed
    if (VertexAttributesLayout().m_layout != vertexLayout.m_layout) {
        throw("Error, vertex attributes layout does not match that found in file");
    }

    // Meshes
    if (reader.m_meshHandles.size()) {
        throw("Should not have any mesh handles loaded");
    }
    for (uint32_t i = 0; i < numMeshes; i++) {
        // Create handle for mesh resource
        Mesh& mesh = *addMesh(m_handle->engine(), "");
        mesh.handle()->setIsLoading(true); // Flag loading for post-construction
        reader.m_meshHandles.push_back(mesh.handle()->sharedPtr());

        // Parse in mesh name
        GString name;
        name << fileStream;
        mesh.handle()->setName(name);

        // Create protocol for parsing in mesh data
        //GStringProtocolField stringField(mesh.getName().createProtocolField());
        auto meshProtocol = Protocol<
            //char*,
            //uint32_t,
            GL::UsagePattern,
            std::vector<Vector2>,
            std::vector<Vector3>,
            std::vector<Vector3>,
            std::vector<Vector3>,
            std::vector<Vector4>,
            std::vector<Vector4>,
            std::vector<Vector4i>,
            std::vector<GLuint>>(
                //stringField,
                //mesh.getName().length(),
                mesh.vertexData().m_usagePattern,
                mesh.vertexData().m_attributes.m_texCoords,
                mesh.vertexData().m_attributes.m_vertices,
                mesh.vertexData().m_attributes.m_normals,
                mesh.vertexData().m_attributes.m_tangents,
                mesh.vertexData().m_attributes.m_colors,
                mesh.vertexData().m_attributes.m_miscReal,
                mesh.vertexData().m_attributes.m_miscInt,
                mesh.vertexData().m_indices
                );
        meshProtocol << fileStream;
    }

    // Load animations
    if (isAnimated()) {

        // Get number of animations
        uint32_t numAnimations = reader.m_animations.size();
        Protocol<uint32_t> animationsProtocol(numAnimations);
        animationsProtocol << fileStream;

        // Iterate to load animations
        for (uint32_t i = 0; i < numAnimations; i++) {

            // Create handle for animation
            auto animHandle = addAnimation(m_handle->engine(), "")->handle();
            animHandle->setIsLoading(true);
            Animation& animation = *animHandle->resourceAs<Animation>();

            // Read animation name
            GString name;
            name << fileStream;
            animHandle->setName(name);

            // Read miscellaneous animation attributes
            uint32_t numNodeAnimations;
            auto animationProtocol = Protocol<
                double,
                double,
                std::vector<float>,
                uint32_t,
                uint32_t>(
                    animation.m_ticksPerSecond,
                    animation.m_durationInTicks,
                    animation.m_times,
                    animation.m_numNonBones,
                    numNodeAnimations
                    );
            animationProtocol << fileStream;

            // Read node animations
            for (uint32_t j = 0; j < numNodeAnimations; j++) {
                animation.m_nodeAnimations.emplace_back();
                NodeAnimation& nodeAnimation = animation.m_nodeAnimations.back();

                auto nodeAnimProtocol = Protocol<
                    std::vector<Vector3>,
                    std::vector<Quaternion>,
                    std::vector<Vector3>>(
                        nodeAnimation.translations(),
                        nodeAnimation.rotations(),
                        nodeAnimation.scales());
                nodeAnimProtocol << fileStream;
            }
        }
    }

    // Read into protocol
    //protocol << fileStream;

    // Close file
    fileStream.close();

    // Get all child resources, organized by type, to match chunk data indexing
    std::vector<std::shared_ptr<ResourceHandle>> materials, meshes, animations;
    getChildren(materials, meshes, animations);

    // Iterate through chunks of model and set
    //reader.postProcessModel();
    for (const ModelChunkData& chunkData : reader.m_chunks) {
        const std::shared_ptr<ResourceHandle>& meshHandle = meshes[chunkData.m_meshHandleIndex];
        const std::shared_ptr<ResourceHandle>& matHandle = materials[chunkData.m_matHandleIndex];

        Vec::EmplaceBack(m_chunks,
            meshHandle,
            matHandle
        );
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::saveBinary(const GString& filepath) const
{
    FileStream fileStream(filepath);
    fileStream.open(FileAccessMode::kWrite | FileAccessMode::kBinary);

    // Create Protocols ---------------------------------------------------

    // Get all child resources, organized by type
    std::vector<std::shared_ptr<ResourceHandle>> materials, meshes, animations;
    getChildren(materials, meshes, animations);

    // Lump everything into one Protocol

    // Need metadata to describe number of child protocols
    uint32_t numMaterials = (uint32_t)materials.size();
    uint32_t numMeshes = (uint32_t)meshes.size();
    uint32_t modelFlags = (uint32_t)m_modelFlags;

    // Create chunk data to serialize model chunks
    std::vector<ModelChunkData> chunkData;
    for (const ModelChunk& chunk : m_chunks) {
        // Get index of mesh in list of mesh resources
        auto meshIter = std::find_if(meshes.begin(), meshes.end(), 
            [&](const auto& meshHandle) {return meshHandle->getUuid() == chunk.meshHandle()->getUuid(); });
        if (meshIter == meshes.end()) {
            throw("Error, mesh not found");
        }
        uint32_t meshHandleIndex = meshIter - meshes.begin();

        // Get index of material in list of material resources
        auto matIter = std::find_if(materials.begin(), materials.end(),
            [&](const auto& matHandle) {return matHandle->getUuid() == chunk.materialHandle()->getUuid(); });
        if (matIter == materials.end()) {
            throw("Error, material not found");
        }
        uint32_t matHandleIndex = matIter - materials.begin();

        chunkData.emplace_back(ModelChunkData{
            meshHandleIndex, matHandleIndex, chunk.mesh()->objectBounds().boxData() });
    }

    Protocol<uint32_t, uint32_t, uint32_t, std::vector<ModelChunkData>> protocol(
        numMaterials,
        numMeshes,
        modelFlags,
        chunkData);

    // Skeleton
    Skeleton& skeleton = *Model::skeleton();
    uint32_t numJoints = skeleton.m_nodes.size();
    auto* skeletonProtocol = new Protocol<Matrix4x4g, std::vector<uint32_t>, std::vector<Matrix4x4g>, uint32_t>(
        skeleton.m_globalInverseTransform,
        skeleton.m_boneNodes,
        skeleton.m_inverseBindPose,
        numJoints);
    for (SkeletonJoint& joint : skeleton.m_nodes) {
        GStringProtocolField nameField = joint.m_name.createProtocolField();
        auto* jointProtocol = new Protocol<char*, Bone, int, uint32_t, TransformMatrices, int, std::vector<uint32_t>>(
            nameField,
            //joint.m_name.length(),
            joint.m_bone,
            joint.m_skeletonTransformIndex,
            joint.m_jointFlags,
            joint.m_transform,
            joint.m_parent,
            joint.m_children
            );

        // Add joint protocol to skeleton protocol
        skeletonProtocol->addChild(jointProtocol);
    }
    protocol.addChild(skeletonProtocol);

    // Materials
    for (const auto& materialHandle : materials) {
        MaterialData& materialData = materialHandle->resourceAs<Material>()->data();
        MaterialDataProtocol* matProtocol = materialData.createProtocol();
        protocol.addChild(matProtocol);
    }

    // Mesh layout
    VertexAttributesLayout vertexLayout;
    auto* layoutProtocol = new Protocol<std::vector<VertexAttributeInfo>>(vertexLayout.m_layout);
    protocol.addChild(layoutProtocol);

    // Meshes
    for (const std::shared_ptr<ResourceHandle>& meshHandle : meshes) {
        Mesh& mesh = *meshHandle->resourceAs<Mesh>();
        GStringProtocolField stringField(meshHandle->getName().createProtocolField());
        auto* meshProtocol = new Protocol<
            char*,
            //uint32_t, 
            GL::UsagePattern,
            std::vector<Vector2>,
            std::vector<Vector3>,
            std::vector<Vector3>,
            std::vector<Vector3>,
            std::vector<Vector4>,
            std::vector<Vector4>,
            std::vector<Vector4i>,
            std::vector<GLuint>>(
                stringField,
                //mesh.getName().length(),
                mesh.vertexData().m_usagePattern,
                mesh.vertexData().m_attributes.m_texCoords,
                mesh.vertexData().m_attributes.m_vertices,
                mesh.vertexData().m_attributes.m_normals,
                mesh.vertexData().m_attributes.m_tangents,
                mesh.vertexData().m_attributes.m_colors,
                mesh.vertexData().m_attributes.m_miscReal,
                mesh.vertexData().m_attributes.m_miscInt,
                mesh.vertexData().m_indices
                );
        protocol.addChild(meshProtocol);
    }

    // If animated, create protocols
    if (isAnimated()) {
        uint32_t numAnimations = (uint32_t)animations.size();
        auto* animationsProtocol = new Protocol<uint32_t>(numAnimations);
        for (const auto& animationHandle : animations) {
            Animation& animation = *animationHandle->resourceAs<Animation>();

            GStringProtocolField nameField(animation.handle()->getName().createProtocolField());
            uint32_t numNodeAnimations = (uint32_t)animation.m_nodeAnimations.size();
            auto* animationProtocol = new Protocol<
                char*,
                double,
                double,
                std::vector<float>,
                uint32_t,
                uint32_t>(
                    nameField,
                    animation.m_ticksPerSecond,
                    animation.m_durationInTicks,
                    animation.m_times,
                    animation.m_numNonBones,
                    numNodeAnimations
                    );

            // Add sub-children for node animations
            for (NodeAnimation& nodeAnimation : animation.m_nodeAnimations) {
                auto* nodeAnimProtocol = new Protocol<
                    std::vector<Vector3>,
                    std::vector<Quaternion>,
                    std::vector<Vector3>>(
                        nodeAnimation.translations(),
                        nodeAnimation.rotations(),
                        nodeAnimation.scales());
                animationProtocol->addChild(nodeAnimProtocol);
            }

            animationsProtocol->addChild(animationProtocol);
        }

        protocol.addChild(animationsProtocol);
    }

    // Write binary to file --------------------------------------------
    protocol.write(fileStream);

    fileStream.close();

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Model::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    //object.insert("name", m_name.c_str());
    //object.insert("type", m_modelType);
    object.insert("flags", (int)m_modelFlags);

    // Don't need to cache other renderable settings, e.g. uniforms
    //object.insert("renderSettings", m_renderSettings.asJson());

    // Add meshes, materials, animations, skeleton
    QJsonArray meshes;
    QJsonArray materials;
    QJsonArray animations;
    for (const auto& handle : m_handle->children()) {
        if (handle->getResourceType() == ResourceType::kMesh) {
            meshes.append(handle->getName().c_str());
        }
        else if (handle->getResourceType() == ResourceType::kMaterial) {
            materials.append(handle->getName().c_str());
        }
        else if (handle->getResourceType() == ResourceType::kAnimation) {
            animations.append(handle->getName().c_str());
        }
        else if (handle->getResourceType() == ResourceType::kSkeleton) {
            object.insert("skeleton", handle->getName().c_str());
        }
        else {
            // TODO: Link to skeleton and animations
            throw("Not implemented");
        }
    }
    object.insert("meshes", meshes);
    object.insert("materials", materials);
    object.insert("animations", animations);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();
    if (object.contains("type")) {
        // Legacy, deprecated
        m_modelFlags = object["type"].toInt();
    }
    else {
        m_modelFlags = object["flags"].toInt();
    }

    //m_name = object["name"].toString();
    //m_handle->setName(m_name); // Good for names to match for intuition

    // Clear current attributes
    if (m_handle->children().size()) {
        m_handle->children().clear();
    }

    m_chunks.clear();
    if (object.contains("meshes")) {
        // Legacy load method, deprecated
        loadChunksFromJson(object);
    }

    // Add animations
    QJsonArray animations = object["animations"].toArray();
    for (const auto& animName : animations) {
        QString name = animName.toString();
        auto handle = m_handle->engine()->resourceCache()->getHandleWithName(name, ResourceType::kAnimation);
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(handle);
    }

    // Add skeleton
    if (object.contains("skeleton")) {
        QString skeletonName = object["skeleton"].toString();
        auto handle = m_handle->engine()->resourceCache()->getHandleWithName(skeletonName,
            ResourceType::kSkeleton);
        m_skeletonHandle = handle;
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(handle);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::checkMesh(const std::shared_ptr<ResourceHandle>& handle)
{
    Mesh* meshPtr = handle->resourceAs<Mesh>();

#ifdef DEBUG_MODE
    static Object printObj;
#endif

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        printObj.logInfo("Mesh is still loading, return");
#endif
        return false;
    }

    if (!handle->isConstructed()) {
#ifdef DEBUG_MODE
        throw("Mesh not yet constructed, returning");
#endif
        return false;
    }

    if (!meshPtr) {
#ifdef DEBUG_MODE
        throw("No mesh found, returning");
#endif
        return false;
    }

    if (!(handle->getResourceType() == ResourceType::kMesh)) {
        throw("Error, incorrect resource type assigned to mesh");
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::checkMaterial(const std::shared_ptr<ResourceHandle>& handle)
{
    Material* matPtr = handle->resourceAs<Material>();

#ifdef DEBUG_MODE
    static Object printObj;
#endif

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        printObj.logInfo("Material is still loading, return");
#endif
        return false;
    }

    if (!handle->isConstructed()) {
#ifdef DEBUG_MODE
        throw("Material not yet constructed, returning");
#endif
        return false;
    }

    if (!matPtr) {
#ifdef DEBUG_MODE
        throw("No material found, returning");
#endif
        return false;
    }

    if (!(handle->getResourceType() == ResourceType::kMaterial)) {
        throw("Error, incorrect resource type assigned to material");
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::loadChunksFromJson(const QJsonObject & object)
{
    // Get meshes
    QJsonArray meshes = object["meshes"].toArray();
    std::vector<std::shared_ptr<ResourceHandle>> meshHandles;
    for (const auto& meshID : meshes) {
        QString name = meshID.toString();
        auto handle = m_handle->engine()->resourceCache()->getHandleWithName(name, ResourceType::kMesh);
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(handle);
        meshHandles.push_back(handle);
    }

    // Get materials
    QJsonArray materials = object["materials"].toArray();
    std::vector<std::shared_ptr<ResourceHandle>> matHandles;
    for (const auto& matID : materials) {
        QString name = matID.toString();
        auto handle = m_handle->engine()->resourceCache()->getHandleWithName(name, ResourceType::kMaterial);
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
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
            throw("Error, expecting mesh to be loaded");
        }
        //m_chunks.back().setObjectBounds(mesh->objectBounds());
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
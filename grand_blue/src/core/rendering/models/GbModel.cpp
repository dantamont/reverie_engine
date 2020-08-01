#include "GbModel.h"

#include "../../utils/GbMemoryManager.h"
#include "../../readers/models/GbModelReader.h"
#include "../../resource/GbResource.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../geometry/GbBuffers.h"
#include "../shaders/GbShaders.h"
#include "../../readers/GbJsonReader.h"
#include "../../rendering/materials/GbMaterial.h"
#include "../geometry/GbSkeleton.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Model Chunk
/////////////////////////////////////////////////////////////////////////////////////////////
ModelChunk::ModelChunk(const std::shared_ptr<ResourceHandle>& mesh, const std::shared_ptr<ResourceHandle>& mtl):
    m_mesh(mesh),
    m_material(mtl)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ModelChunk::~ModelChunk()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ModelChunk::isLoaded() const
{
    return materialResource() && meshResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t ModelChunk::getSortID()
{
    std::shared_ptr<Material> mat = materialResource();
    if (mat) {
        return mat->getSortID();
    }
    else {
        return 0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::generateBounds()
{
    std::shared_ptr<Mesh> mesh = meshResource();
    if (!mesh) {
        // Mesh isn't loaded yet, will have to generate in draw call
        return;
    }

    // Initialize limits for bounding box of the mesh
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Update min/max vertex data
    for (const auto& vertex : mesh->vertexData().m_attributes.m_vertices) {
        minX = std::min(vertex.x(), minX);
        maxX = std::max(vertex.x(), maxX);
        minY = std::min(vertex.y(), minY);
        maxY = std::max(vertex.y(), maxY);
        minZ = std::min(vertex.z(), minZ);
        maxZ = std::max(vertex.z(), maxZ);
    }

    std::vector<AABB>& boxes = m_bounds.geometry();
    boxes.clear();
    boxes.push_back(AABB());
    boxes.back().setMinX(minX);
    boxes.back().setMaxX(maxX);
    boxes.back().setMinY(minY);
    boxes.back().setMaxY(maxY);
    boxes.back().setMinZ(minZ);
    boxes.back().setMaxZ(maxZ);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ModelChunk::asJson() const
{
    QJsonObject object;
    object.insert("meshName", m_mesh->getName());
    object.insert("matName", m_material->getName());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::loadFromJson(const QJsonValue & json)
{
    throw("Error, unimplemented");
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::preDraw()
{
    // Make sure that bounds are generated before geometry is drawn
    if (!m_bounds.geometry().size()) {
        if (meshResource()) {
            generateBounds();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const std::pair<QString, Uniform>& uniformPair : m_uniforms) {
        shaderProgram.setUniformValue(uniformPair.second);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::bindTextures(ShaderProgram* shaderProgram)
{
    QMutexLocker matLocker(&m_material->mutex());

    // Bind material
    std::shared_ptr<Material> mat = materialResource();
    if (mat) {
        if (!mat->isBound()) {
            mat->bind(*shaderProgram);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::releaseTextures(ShaderProgram* shaderProgram)
{
    QMutexLocker matLocker(&m_material->mutex());

    // Unbind material    
    std::shared_ptr<Material> mat = materialResource();
    if (mat) {
        if (mat->isBound()) {
            mat->release();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::drawGeometry(ShaderProgram & shaderProgram, RenderSettings * settings)
{
    QMutexLocker meshLocker(&m_mesh->mutex());

    int shapeMode = settings ? settings->shapeMode() : m_renderSettings.shapeMode();
    
    // Draw geometry
    std::shared_ptr<Mesh> mesh = meshResource();
    if (mesh) {
        mesh->vertexData().drawGeometry(shapeMode);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> ModelChunk::meshResource() const
{
    if (m_mesh->isConstructed()) {
        return m_mesh->resourceAs<Mesh>(false);
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Material> ModelChunk::materialResource() const
{
    if (m_material->isConstructed()) {
        return m_material->resourceAs<Material>(false);
    }
    else {
        return nullptr;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Model
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Model::createHandle(CoreEngine * engine)
{
    auto handle = ResourceHandle::create(engine, Resource::kModel);
    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Model::createHandle(CoreEngine * engine,
    const QString & modelName,
    ModelType type)
{
    // Create handle for material
    auto handle = ResourceHandle::create(engine, Resource::kModel);
    handle->setUserGenerated(true); // model is generated in engine
    handle->setName(modelName);

    // Create model
    auto model = std::make_shared<Model>(engine, modelName, type);
    handle->setResource(model, false);

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(CoreEngine * engine):
    Resource(kModel),
    m_engine(engine),
    m_modelType(kUndefined)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(CoreEngine* engine,
    const QString& uniqueName,
    ModelType type) :
    Model(engine, uniqueName, kModel, type)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(CoreEngine * engine, 
    const QString & uniqueName, 
    Resource::ResourceType resourceType,
    ModelType type):
    Resource(uniqueName, resourceType),
    m_engine(engine),
    m_modelType(type)
{
#ifdef DEBUG_MODE
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(ResourceHandle& handle, const QJsonValue& json) :
    Resource(kModel),
    m_engine(handle.engine()),
    m_modelType(kStaticMesh)
{
    m_handle = &handle;
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(ResourceHandle& handle) :
    Resource(kModel),
    m_engine(handle.engine()),
    m_modelType(kStaticMesh)
{
    m_handle = &handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Skeleton> Model::skeleton() const
{
    if (!m_skeletonHandle) {
        return nullptr;
    }
    else {
        return m_skeletonHandle->resourceAs<Skeleton>(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void Model::createDrawCommands(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
//    Camera & camera, 
//    ShaderProgram & shader)
//{
//}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Model::getMaterial(const QString& name)
{
    auto handle = m_engine->resourceCache()->getHandleWithName(name, Resource::kMaterial);
    auto childHandle = m_handle->getChild(handle->getUuid());
    if (!childHandle) {
        throw("Error, no handle found, but is not child of model");
    }
    return childHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mesh> Model::addMesh(const QString & meshName)
{
    // Create handle for mesh
    ResourceHandle::BehaviorFlags flags;
    flags.setFlag(ResourceHandle::kChild, true);
    auto handle = ResourceHandle::create(m_engine,
        m_handle->getPath(),
        Resource::kMesh,
        flags);
    handle->setName(meshName);

    // Create mesh
    auto mesh = std::make_shared<Mesh>(meshName);
    handle->setResource(mesh, false);

    // Add handle as child
    m_handle->addChild(handle);

    return mesh;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Material> Model::addMaterial(const QString & mtlName)
{
    // Create handle for material
    ResourceHandle::BehaviorFlags flags;
    flags.setFlag(ResourceHandle::kChild, true);
    auto handle = ResourceHandle::create(m_engine,
        m_handle->getPath(),
        Resource::kMaterial,
        flags);
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_shared<Material>(m_engine, mtlName);
    handle->setResource(mtl, false);

    // Add handle as child
    m_handle->addChild(handle);

    return mtl;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Animation> Model::addAnimation(const QString & animationName)
{
    // Create handle for material
    ResourceHandle::BehaviorFlags flags;
    flags.setFlag(ResourceHandle::kChild, true);
    auto handle = ResourceHandle::create(m_engine,
        m_handle->getPath(),
        Resource::kAnimation,
        flags);
    handle->setName(animationName);

    // Create animation
    auto anim = std::make_shared<Animation>(animationName);
    handle->setResource(anim, false);

    // Add handle as child
    m_handle->addChild(handle);

    return anim;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::addSkeleton()
{
    switch (m_modelType) {
    case kAnimatedMesh:
    case kStaticMesh:
        break;
    default:
        throw("Error, model type does not support skeleton");
    }
    QString skellyName = m_name + "_skeleton";
    ResourceHandle::BehaviorFlags flags;
    flags.setFlag(ResourceHandle::kChild, true);
    m_skeletonHandle = ResourceHandle::create(m_engine, Resource::kSkeleton, flags);
    m_skeletonHandle->setName(skellyName);
    m_skeletonHandle->setResource(std::make_shared<Skeleton>(skellyName), false);
    m_skeletonHandle->setIsLoading(true);
    m_handle->addChild(m_skeletonHandle);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::postConstruction()
{
    if (m_handle->reader()) {
        // Get model reader used to load model
        auto reader = S_CAST<ModelReader>(
            m_handle->reader()
            );

        // Iterate through chunks of model and set
        for (const ModelChunkData& chunkData : reader->chunks()) {
            const std::shared_ptr<ResourceHandle>& meshHandle =
                m_engine->resourceCache()->getHandle(chunkData.m_meshHandleID);

            const std::shared_ptr<ResourceHandle>& matHandle =
                m_engine->resourceCache()->getHandle(chunkData.m_matHandleID);

            m_chunks.push_back(std::make_shared<ModelChunk>(
                meshHandle,
                matHandle
                ));

            // Set bounding box for chunk
            m_chunks.back()->boundingBoxes().geometry().push_back(std::move(chunkData.m_boundingBox));
        }

        // Delete the model reader
        m_handle->setReader(nullptr);
    }
    else {
        // Load from resource JSON if no reader
        if (m_handle->resourceJson().isEmpty()) {
            throw("Error, need either JSON data or a reader");
        }
        loadFromJson(m_handle->resourceJson());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Model::asJson() const
{
    QJsonObject object;
    object.insert("name", m_name);
    object.insert("type", m_modelType);

    // Don't need to cache other renderable settings, e.g. uniforms
    //object.insert("renderSettings", m_renderSettings.asJson());

    // Add meshes, materials, animations, skeleton
    QJsonArray meshes;
    QJsonArray materials;
    QJsonArray animations;
    for (const auto& handle : m_handle->children()) {
        if (handle->getResourceType() == Resource::kMesh) {
            meshes.append(handle->getName());
        }
        else if (handle->getResourceType() == Resource::kMaterial) {
            materials.append(handle->getName());
        }
        else if (handle->getResourceType() == Resource::kAnimation) {
            animations.append(handle->getName());
        }
        else if (handle->getResourceType() == Resource::kSkeleton) {
            object.insert("skeleton", handle->getName());
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
void Model::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_modelType = ModelType(json["type"].toInt());

    m_name = object["name"].toString();
    m_handle->setName(m_name); // Good for names to match for intuition

    // Clear current attributes
    if (m_handle->children().size()) {
        m_handle->children().clear();
    }

    m_chunks.clear();
    if (object.contains("meshes")) {
        // Legacy load method, deprecated
        loadChunksFromJson(object);
    }
//    else {
//        QJsonArray chunks = object["chunks"].toArray();
//        for (const auto& chunkJson : chunks) {
//            QJsonObject chunkObject = chunkJson.toObject();
//
//            // Load mesh
//            QString meshName = chunkObject["meshName"].toString();
//            auto meshHandle = m_engine->resourceCache()->getHandleWithName(meshName, Resource::kMesh);
//#ifdef DEBUG_MODE
//            if (!meshHandle) throw("Error, handle not found for given name");
//#endif
//            m_handle->addChild(meshHandle);
//
//            // Load material and set as child resource if not loaded already
//            QString matName = chunkObject["matName"].toString();
//            auto matHandle = m_engine->resourceCache()->getHandleWithName(matName, Resource::kMaterial);
//#ifdef DEBUG_MODE
//            if (!matHandle) throw("Error, handle not found for given name");
//#endif
//            if (!m_handle->getChild(matHandle->getUuid())) {
//                m_handle->addChild(matHandle);
//            }
//
//            // Create chunk
//            auto chunk = std::make_shared<ModelChunk>(
//                meshHandle->resourceAs<Mesh>(false),
//                matHandle->resourceAs<Material>(false)
//                );
//            m_chunks.push_back(chunk);
//        }
//    }


    // Add animations
    QJsonArray animations = object["animations"].toArray();
    for (const auto& animName : animations) {
        QString name = animName.toString();
        auto handle = m_engine->resourceCache()->getHandleWithName(name, Resource::kAnimation);
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(handle);
    }

    // Add skeleton
    if (object.contains("skeleton")) {
        QString skeletonName = object["skeleton"].toString();
        auto handle = m_engine->resourceCache()->getHandleWithName(skeletonName, 
            Resource::kSkeleton);
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
    std::shared_ptr<Mesh> meshPtr = handle->resourceAs<Mesh>(false);

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        logInfo("Mesh is still loading, return");
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

    if (!(handle->getResourceType() == Resource::kMesh)) {
        throw("Error, incorrect resource type assigned to mesh");
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::checkMaterial(const std::shared_ptr<ResourceHandle>& handle)
{
    std::shared_ptr<Material> matPtr = handle->resourceAs<Material>(false);

    // Return if still loading
    if (handle->isLoading()) {
#ifdef DEBUG_MODE
        logInfo("Material is still loading, return");
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

    if (!(handle->getResourceType() == Resource::kMaterial)) {
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
        auto handle = m_engine->resourceCache()->getHandleWithName(name, Resource::kMesh);
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
        auto handle = m_engine->resourceCache()->getHandleWithName(name, Resource::kMaterial);
#ifdef DEBUG_MODE
        if (!handle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(handle);
        matHandles.push_back(handle);
    }

    // Load meshes and materials as chunks
    size_t i = 0;
    for (i = 0; i < meshHandles.size(); i++) {
        Vec::EmplaceBack(m_chunks, 
            std::make_shared<ModelChunk>(meshHandles[i],
                matHandles[i]));
    
        // Generate bounding boxes from the model chunk mesh
        m_chunks.back()->generateBounds();
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
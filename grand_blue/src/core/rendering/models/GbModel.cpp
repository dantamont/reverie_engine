#include "GbModel.h"

#include "../materials/GbCubeMap.h"
#include "../../resource/GbResource.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../geometry/GbBuffers.h"
#include "../shaders/GbShaders.h"
#include "../../readers/GbJsonReader.h"
#include "../../rendering/materials/GbMaterial.h"

namespace Gb {   

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Model> Model::create(CoreEngine * engine, const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    ModelType type = ModelType(object.value("type").toInt());
    std::shared_ptr<Model> model;
    switch (type) {
    case ModelType::kCubeMap:
    {
        model = std::make_shared<CubeMap>(engine, json);
        break;
    }
    case ModelType::kStaticMesh:
    {
        model = std::make_shared<Model>(engine, json);
        break;
    }
    case ModelType::kAnimatedMesh:
    default:
        throw("create:: Error, this type of model is not implemented");
        break;
    }
    return model;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(CoreEngine* engine, const QString& uniqueName, ModelType type) :
    Object(uniqueName),
    m_engine(engine),
    m_modelType(type),
    m_meshHandle(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(CoreEngine* engine, const QJsonValue& json) :
    Object(),
    m_engine(engine),
    m_modelType(kStaticMesh),
    m_meshHandle(nullptr)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::Model(const QString& uniqueName, const std::shared_ptr<Gb::ResourceHandle>& meshHandle) :
    Object(uniqueName),
    m_engine(meshHandle->engine()),
    m_meshHandle(meshHandle),
    m_modelType(kStaticMesh)
{
    //createShapesFromMesh();
    //if (material) {
    //    addMaterial(material);
    //}
    checkMesh();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
const QString& Model::getFilePath() const
{
    return m_meshHandle->getPath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::isFromFile()
{
    return !m_meshHandle->getPath().isEmpty();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::rename(const QString & name)
{
    // TODO: Make this a common mixin for resource-likes
    if (name == m_name) return;

    // Delete from resource cache map
    if (inResourceCache()) {
        auto pointer_to_this = m_engine->resourceCache()->getModel(m_name);
        m_engine->resourceCache()->removeModel(m_name);

        // Set the name
        setName(name);

        // Add back to resource cache map
        m_engine->resourceCache()->addModel(pointer_to_this);
    }
    else {
        setName(name);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::inResourceCache() const
{
    return m_engine->resourceCache()->hasModel(m_name);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::draw(const std::shared_ptr<ShaderProgram>& shaderProgram,
    RenderSettings* settings)
{
    // TODO: Maybe sort models by material?

    // Return if still loading
    if (m_meshHandle->getIsLoading()) {
#ifdef DEBUG_MODE
        logInfo("Mesh is still loading, return");
#endif
        return;
    }

    // Lock mutex to mesh handle
    QMutexLocker locker(&m_meshHandle->mutex());

    // Check that geometry is loaded
    if (!checkMesh()) {
        return;
    }

    Renderable::draw(shaderProgram, settings);

}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Model::asJson() const
{
    QJsonObject object;
    object.insert("name", m_name);
    object.insert("type", m_modelType);

    // Don't need to cache uniforms right now
    object.insert("renderSettings", m_renderSettings.asJson());

    // Add mesh
    object.insert("mesh", m_meshHandle->asJson());

    // Add materials
    QJsonObject materials;
    for (const std::pair<QString, QString>& mtlNamePair : m_materialNames) {
        // Just use unique material name, since materials are loaded by resource cache
        materials.insert(mtlNamePair.first, mtlNamePair.second);
    }
    object.insert("materials", materials);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_modelType = ModelType(json["type"].toInt());

    // Rename the model, moving in resource cache if necessary
    rename(object["name"].toString());

    // Don't need to cache uniforms right now
    m_renderSettings.loadFromJson(object.value("renderSettings"));

    // Load mesh
#ifdef DEBUG_MODE
    QString jsonString = JsonReader::getJsonValueAsQString(object.value("mesh"));
#endif
    m_meshHandle = m_engine->resourceCache()->getResourceHandle(object.value("mesh"));

    // Load materials used by model
    const QJsonObject& materials = object["materials"].toObject();
    for (const QString& meshName : materials.keys()) {
        const QString& mtlName = materials.value(meshName).toString();
        Map::Emplace(m_materialNames, meshName, mtlName);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Model::checkMesh()
{
    std::shared_ptr<Mesh> meshPtr = mesh();

    bool isValid = true;
    if (!meshPtr) {
#ifdef DEBUG_MODE
        logInfo("No mesh found, returning");
#endif
        return false;
    }

    if (!(meshPtr->getType() == Resource::kMesh)) {
        throw("Error, incorrect resource type assigned to mesh");
    }

    if (!meshPtr->isConstructed()) {
#ifdef DEBUG_MODE
        logInfo("Mesh not yet constructed, returning");
#endif
        return false;
    }

    return isValid;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::bindTextures()
{    
    // Load materials onto the mesh 
     mesh()->setMaterialNames(m_materialNames);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::releaseTextures()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Model::drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram,
    RenderSettings* settings)
{
    // Draw meshes
    mesh()->draw(m_engine, shaderProgram,
        settings ? settings->shapeMode() : m_renderSettings.shapeMode());
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
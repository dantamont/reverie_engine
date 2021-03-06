#include "GModelChunk.h"

#include "../GGLFunctions.h"
#include "../geometry/GBuffers.h"
#include "../geometry/GSkeleton.h"
#include "../geometry/GMesh.h"
#include "../../utils/GMemoryManager.h"
#include "../../readers/models/GModelReader.h"
#include "../../resource/GResource.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../shaders/GShaderProgram.h"
#include "../../readers/GJsonReader.h"
#include "../../rendering/materials/GMaterial.h"
#include "../../rendering/renderer/GRenderContext.h"
#include "../../rendering/lighting/GLightSettings.h"
#include "../../containers/GFlags.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Model Chunk
/////////////////////////////////////////////////////////////////////////////////////////////
ModelChunk::ModelChunk(const std::shared_ptr<ResourceHandle>& mesh, const std::shared_ptr<ResourceHandle>& mtl)
{
    m_meshHandle = mesh;
    m_matHandle = mtl;
}
/////////////////////////////////////////////////////////////////////////////////////////////
ModelChunk::~ModelChunk()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool ModelChunk::isLoaded() const
{
    return materialResource() && mesh();
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t ModelChunk::getSortID()
{
    Material* mat = materialResource();
    if (mat) {
        return mat->getSortID();
    }
    else {
        return 0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ModelChunk::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("meshName", m_meshHandle->getName().c_str());
    object.insert("matName", m_matHandle->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(json)
    Q_UNUSED(context)
    throw("Error, unimplemented");
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::preDraw()
{
    // Make sure that bounds are generated before geometry is drawn
    //if (!m_bounds.geometry().size()) {
    //    if (meshResource()) {
    //        generateBounds();
    //    }
    //}
    //if (!m_objectBounds.count()) {
    //    std::shared_ptr<Mesh> mesh = meshResource();
    //    if (mesh) {
    //        m_objectBounds = mesh->objectBounds();
    //    }
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const Uniform& uniform : m_uniforms) {
        shaderProgram.setUniformValue(uniform);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::bindTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    QMutexLocker matLocker(&m_matHandle->mutex());

    // Bind shadow map textures
    if (context) {
        std::array<std::shared_ptr<Texture>, 3>& shadowTextures = context->lightingSettings().shadowTextures();
        size_t size = shadowTextures.size();
        for (size_t i = 0; i < size; i++) {
            const std::shared_ptr<Texture>& shadowTexture = shadowTextures[i];
            shadowTexture->bind(i);
        }
    }

    // Bind material
    Material* mat = materialResource();
    if (mat) {
        if (!mat->isBound(*context)) {
            mat->bind(*shaderProgram, *context);
        }
        //else {
        //    Logger::LogInfo(GString::Format("Material %s not bound onto mesh %s",
        //        m_material->getName().c_str(), m_meshHandle->getName().c_str()).c_str());
        //}
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::releaseTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    Q_UNUSED(shaderProgram);

    QMutexLocker matLocker(&m_matHandle->mutex());

    // Release shadow map textures
    if (context) {
        for (const std::shared_ptr<Texture>& shadowTexture : context->lightingSettings().shadowTextures()) {
            shadowTexture->release();
        }
    }

    // Unbind material    
    Material* mat = materialResource();
    if (mat) {
        if (mat->isBound(*context)) {
            mat->release(*context);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ModelChunk::drawGeometry(ShaderProgram & shaderProgram, RenderSettings * settings)
{
    Q_UNUSED(shaderProgram);

    QMutexLocker meshLocker(&m_meshHandle->mutex());

    PrimitiveMode shapeMode = settings ? settings->shapeMode() : m_renderSettings.shapeMode();
    
    // Draw geometry
    Mesh* m = mesh();
    if (m) {
        m->vertexData().drawGeometry(shapeMode);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material* ModelChunk::materialResource() const
{
    if (m_matHandle->isConstructed()) {
        return m_matHandle->resourceAs<Material>();
    }
    else {
        return nullptr;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
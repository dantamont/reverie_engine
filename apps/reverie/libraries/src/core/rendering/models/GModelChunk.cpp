#include "core/rendering/models/GModelChunk.h"

#include "core/rendering/GGLFunctions.h"
#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/geometry/GMesh.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "core/readers/models/GModelReader.h"
#include "core/resource/GResourceHandle.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "fortress/json/GJson.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {



// Model Chunk

ModelChunk::ModelChunk(const std::shared_ptr<ResourceHandle>& mesh, const std::shared_ptr<ResourceHandle>& mtl)
{
    m_meshHandle = mesh;
    m_matHandle = mtl;
}

ModelChunk::~ModelChunk()
{
}

bool ModelChunk::isLoaded() const
{
    return materialResource() && mesh();
}

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

void to_json(json& orJson, const ModelChunk& korObject)
{
    orJson["meshName"] = korObject.m_meshHandle->getName().c_str();
    orJson["matName"] = korObject.m_matHandle->getName().c_str();
}

void from_json(const json& korJson, ModelChunk& orObject)
{
    Q_UNUSED(korJson)
    Q_UNUSED(orObject)
    Logger::Throw("Error, unimplemented");
}

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

void ModelChunk::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const Uniform& uniform : m_uniforms) {
        shaderProgram.setUniformValue(uniform);
    }
}

void ModelChunk::bindTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    QMutexLocker matLocker(&m_matHandle->mutex());

    // Bind shadow map textures
    if (context) {
        constexpr size_t numShadowTextures = NUM_SHADOW_MAP_TEXTURES;
        std::array<std::shared_ptr<Texture>, numShadowTextures>& shadowTextures = context->lightingSettings().shadowTextures();
        for (size_t i = 0; i < numShadowTextures; i++) {
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

void ModelChunk::drawGeometry(ShaderProgram & shaderProgram, RenderSettings * settings)
{
    Q_UNUSED(shaderProgram);

    QMutexLocker meshLocker(&m_meshHandle->mutex());

    PrimitiveMode shapeMode = settings ? settings->shapeMode() : m_renderSettings.shapeMode();
    
    // Draw geometry
    Mesh* m = mesh();
    if (m) {
        m->vertexData().drawGeometry(shapeMode, settings->instanceCount());
    }
}

Material* ModelChunk::materialResource() const
{
    if (m_matHandle->isConstructed()) {
        return m_matHandle->resourceAs<Material>();
    }
    else {
        return nullptr;
    }
}



// End namespaces
}
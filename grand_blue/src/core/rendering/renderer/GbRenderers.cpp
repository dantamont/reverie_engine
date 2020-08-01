#include "GbRenderers.h"

#include <typeinfo> 

#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../geometry/GbBuffers.h"
#include "../shaders/GbShaders.h"
#include "../../components/GbCanvasComponent.h"
#include "../../components/GbModelComponent.h"

namespace Gb {   

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(): OpenGLFunctions()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//void Renderer::draw(const std::vector<Renderable*>& renderables, const std::shared_ptr<ShaderProgram>& shaderProgram)
//{
//    // Toggle GL Settings
//    m_renderSettings.bind();
//
//    // Bind shader
//    if (m_renderSettings.hasShaderFlag(RenderSettings::kBind)) {
//        shaderProgram->bind();
//    }
//
//    // Set shader uniforms
//    bindUniforms(shaderProgram);
//    shaderProgram->updateUniforms();
//
//    // Render models
//    std::vector<Renderable*>::const_iterator iR;
//    for (iR = renderables.begin(); iR != renderables.end(); iR++) {
//        (*iR)->draw(shaderProgram, &m_renderSettings);
//    }
//
//    // Release shader uniforms
//    releaseUniforms(shaderProgram);
//
//    // Release shader
//    if (m_renderSettings.hasShaderFlag(RenderSettings::kRelease)) {
//        shaderProgram->release();
//    }
//
//    // Untoggle GL settings
//    m_renderSettings.release();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Renderer::asJson() const
{
    QJsonObject object = Renderable::asJson().toObject();
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    Renderable::loadFromJson(object);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::bindUniforms(ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
#include "GRenderContext.h"

#include <typeinfo> 

#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "GMainRenderer.h"
#include "../../view/GL/GGLWidget.h"
#include "../lighting/GLightSettings.h"
#include "../materials/GTexture.h"

namespace rev {   

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

RenderContext::RenderContext(MainRenderer* renderer):
    m_context(renderer->widget()->context()),
    m_surface(renderer->widget()->context()->surface())
{
    if (!isCurrent()) {
        makeCurrent();
    }

#ifdef DEBUG_MODE
    ////Initialize debug output
    //GL::OpenGLFunctions& gl = *GL::OpenGLFunctions::Functions();
    //gl.glEnable(GL_DEBUG_OUTPUT);
    //gl.glDebugMessageCallback(GL::OpenGLFunctions::printMessageCallBack, 0);
#endif

    // Create blank (white) texture
    m_blankTexture = new Texture(1, 1,
        TextureTargetType::k2D,
        TextureUsageType::kNone,
        TextureFilter::kNearest,
        TextureFilter::kNearest);
    m_blankTexture->postConstruction();
    m_blankTexture->setData(Vector3::Ones().data(), PixelFormat::kRGB, PixelType::kFloat32);
    //std::array<Vector3, 1> outBlank;
    //m_blankTexture->getData(outBlank.data(), PixelFormat::kRGB, PixelType::kFloat32);


    // There are 14 possible GL buffer types, so resize buffer vector
    m_boundBuffers.resize(14);
    for (auto& bufferID : m_boundBuffers) {
        bufferID = Uuid(false);
    }

    // Create light settings, designating a maximum number of lights
    constexpr size_t maxLights = 512;//1024;
    m_lightSettings = std::make_shared<LightingSettings>(*this, maxLights);
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderContext::~RenderContext()
{
    delete m_blankTexture;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderContext::reset()
{
    // TODO: Figure out why simply clearing the lights is not enough to make lights
    // work properly when scenarios are switched
    // Original FIXME: Lights don't toggle on when switching scenarios
    //m_lightSettings->clearLights();
    constexpr size_t maxLights = 512;//1024;
    m_lightSettings = std::make_shared<LightingSettings>(*this, maxLights);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderContext::flushBuffers()
{
    m_lightSettings->lightBuffers().flushBuffer();
    m_lightSettings->pointLightMatrixBuffers().flushBuffer();
    m_lightSettings->pointLightAttributeBuffers().flushBuffer();
    m_lightSettings->shadowBuffers().flushBuffer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderContext::swapBuffers()
{
    m_lightSettings->lightBuffers().swapBuffers();
    m_lightSettings->pointLightMatrixBuffers().swapBuffers();
    m_lightSettings->pointLightAttributeBuffers().swapBuffers();
    m_lightSettings->shadowBuffers().swapBuffers();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool RenderContext::isCurrent() const
{
    return QOpenGLContext::currentContext() == m_context;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void RenderContext::makeCurrent()
{
    m_context->makeCurrent(m_surface);
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t RenderContext::bufferTypeIndex(const GL::BufferType & type)
{
    auto iter = std::find(s_bufferTypes.begin(), 
        s_bufferTypes.end(),
        type);

#ifdef DEBUG_MODE
    if (iter == s_bufferTypes.end()) {
        throw("Error, buffer type not found");
    }
#endif

    return iter - s_bufferTypes.begin();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<GL::BufferType> RenderContext::s_bufferTypes = {
    GL::BufferType::kUniformBuffer,
    GL::BufferType::kShaderStorage
};


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
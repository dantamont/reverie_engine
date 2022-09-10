#include "core/rendering/renderer/GRenderContext.h"

#include <typeinfo> 

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/materials/GTexture.h"

namespace rev {   

RenderContext::RenderContext(QOpenGLContext* context, QSurface* surface):
    m_context(context),
    m_surface(surface)
{
    if (!isCurrent()) {
        makeCurrent();
    }

//#ifdef DEBUG_MODE
//    toggleDebugOutput();
//#endif

    // Create blank (white) texture
    m_blankTexture = new Texture(1, 1,
        TextureTargetType::k2D,
        TextureUsageType::kNone,
        TextureFilter::kNearest,
        TextureFilter::kNearest);
    m_blankTexture->postConstruction(ResourcePostConstructionData());
    m_blankTexture->setData(Vector3::Ones().data(), PixelFormat::kRGB, PixelType::kFloat32);

    // There are 14 possible GL buffer types, so resize buffer vector
    m_boundBuffers.resize(14);
    for (auto& bufferID : m_boundBuffers) {
        bufferID = Uuid(false);
    }

    // Create light settings, designating a maximum number of lights
    resetLights();
}

RenderContext::~RenderContext()
{
    delete m_blankTexture;
}

void RenderContext::reset()
{
    resetLights();
}

void RenderContext::flushBuffers()
{
    m_lightSettings->lightBuffers().flushBuffer();
    m_lightSettings->pointLightMatrixBuffers().flushBuffer();
    m_lightSettings->pointLightAttributeBuffers().flushBuffer();
    m_lightSettings->shadowBuffers().flushBuffer();
}

void RenderContext::swapBuffers()
{
    m_lightSettings->lightBuffers().swapBuffers();
    m_lightSettings->pointLightMatrixBuffers().swapBuffers();
    m_lightSettings->pointLightAttributeBuffers().swapBuffers();
    m_lightSettings->shadowBuffers().swapBuffers();
}

bool RenderContext::isCurrent() const
{
    return QOpenGLContext::currentContext() == m_context;
}

void RenderContext::makeCurrent()
{
    m_context->makeCurrent(m_surface);
}

void RenderContext::toggleDebugOutput()
{
    // Initialize debug output
    gl::OpenGLFunctions& gl = *gl::OpenGLFunctions::Functions();
    gl.glEnable(GL_DEBUG_OUTPUT);
    gl.glDebugMessageCallback(gl::OpenGLFunctions::printMessageCallBack, 0);
}

void RenderContext::resetLights()
{
    // TODO: Figure out why simply clearing the lights is not enough to make lights
    // work properly when scenarios are switched
    // Original FIXME: Lights don't toggle on when switching scenarios
    //m_lightSettings->clearLights();
    constexpr Uint32_t s_maxLights = 512;//1024;

    m_lightSettings = nullptr;
    m_lightSettings = std::make_unique<LightingSettings>(*this, s_maxLights);
}


// End namespaces
}
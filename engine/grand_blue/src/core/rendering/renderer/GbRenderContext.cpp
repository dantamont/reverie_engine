#include "GbRenderContext.h"

#include <typeinfo> 

#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "GbMainRenderer.h"
#include "../../view/GL/GbGLWidget.h"
#include "../lighting/GbLight.h"

namespace Gb {   

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

RenderContext::RenderContext(MainRenderer* renderer):
    m_context(renderer->widget()->context()),
    m_surface(renderer->widget()->context()->surface())
{
    m_lightSettings = std::make_shared<LightingSettings>(*this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderContext::~RenderContext()
{
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
// End namespaces
}
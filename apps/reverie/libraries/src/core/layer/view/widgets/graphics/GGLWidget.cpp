// Internal
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"

#include <QScreen>

#include "core/GCoreEngine.h"
#include "core/rendering/shaders/GShaderProgram.h"

#include "core/scene/GScenario.h"
#include "core/components/GCameraComponent.h"
#include "core/loop/GSimLoop.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

namespace rev { 

GLWidget::GLWidget(const QString& name, CoreEngine* engine, QWidget * parent) :
    GLWidgetInterface(name, std::make_unique<InputHandler>(this), parent),
    m_engine(engine)
{
    // Initialize all signal/slot connections
    initializeConnections();
}

GLWidget::~GLWidget()
{
}

void GLWidget::addRenderProjection(RenderProjection * rp)
{
    m_renderProjections.push_back(rp);
    rp->resizeProjection(width(), height());
}

void GLWidget::clear()
{
    m_renderProjections.clear();
    m_renderer->requestResize();
}

void GLWidget::requestResize()
{
    m_renderer->requestResize();
}

void GLWidget::initializeGL()
{
	// Needed to checkValidity GL
	m_renderer = std::make_shared<OpenGlRenderer>(m_engine, this);
	m_renderer->initialize();

    // Emit signal that GL context is initialized
    emit initializedContext();
}

void GLWidget::resizeGL(int w, int h)
{
    resizeProjections(w, h);
    if (m_engine->scenario()) {
        m_renderer->requestResize();
        m_renderer->render();
    }
}

void GLWidget::paintGL()
{
    if (m_engine->scenario()) {
        // This is called from the main simloop's update routine (GLWidget::update())
        // TODO: Separate processScenes and render logic into different loops
        m_renderer->processScenes(); 
        m_renderer->render();
    }
}

void GLWidget::resizeProjections(int width, int height) {
    for (const auto& rp : m_renderProjections) {
        rp->resizeProjection(width, height);
    }
}

void GLWidget::resizeProjections() {
    resizeProjections(width(), height());
}

void GLWidget::initializeConnections()
{
    // Why wasn't I just doing this in onResize?
    //connect(this, &GLWidget::resized, this, static_cast<void (GLWidget::*)(int w, int h)>(&GLWidget::resizeProjections));
    connect(m_engine, &CoreEngine::scenarioNeedsRedraw, this, static_cast<void (GLWidget::*)()>(&GLWidget::resizeProjections));
}



} // end namespacing
// Internal
#include "GGLWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/rendering/shaders/GShaderProgram.h"

#include "../../core/scene/GScenario.h"
#include "../../core/components/GCameraComponent.h"
#include "../../core/loop/GSimLoop.h"
#include "../../core/rendering/renderer/GMainRenderer.h"

namespace rev { 
namespace View {

/////////////////////////////////////////////////////////////////////////////////////////////
GLWidget::GLWidget(const QString& name, CoreEngine* engine,
    QWidget * parent) :
    //AbstractService(name),
    Nameable(name),
    QOpenGLWidget(parent),
    m_engine(engine),
    m_inputHandler(this)
    //m_lastHeight(0),
    //m_lastWidth(0)
{
    // Initialize all signal/slot connections
    initializeConnections();

    // Ensure that mouse is tracked even when button is not clicked
    setMouseTracking(true);

    // Ensure that widget accepts keyboard focus
    setFocusPolicy(Qt::StrongFocus);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GLWidget::~GLWidget()
{
    // Make context current for cleanup
    makeCurrent();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::addRenderProjection(RenderProjection * rp)
{
    m_renderProjections.push_back(rp);
    rp->resizeProjection(width(), height());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::clear()
{
    m_renderProjections.clear();
    m_renderer->requestResize();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::resizeEvent(QResizeEvent * event)
{
    // Perform base class resize event
    QOpenGLWidget::resizeEvent(event);

    // Emit resize signal (connects to RenderProjection class)
    emit resized(width(), height());

#ifdef DEBUG_MODE
    //int w = width();
    //int h = height();
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::initializeGL()
{
	// Needed to checkValidity GL
	m_renderer = std::make_shared<MainRenderer>(m_engine, this);
	m_renderer->initialize();

    // Emit signal that GL context is initialized
    emit initializedContext();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    resizeProjections(w, h);
    if (m_engine->scenario()) {
        m_renderer->requestResize();
        m_renderer->render();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::paintGL()
{
    if (m_engine->scenario()) {
        // This is called from the main simloop's update routine (GLWidget::update())
        // TODO: Separate processScenes and render logic into different loops
        m_renderer->processScenes(); 
        m_renderer->render();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::resizeProjections(int width, int height) {
    for (const auto& rp : m_renderProjections) {
        rp->resizeProjection(width, height);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::resizeProjections() {
    resizeProjections(width(), height());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::contextMenuEvent(QContextMenuEvent * event)
{
    Q_UNUSED(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::keyPressEvent(QKeyEvent * event)
{
    m_inputHandler.handleEvent(event);
    QOpenGLWidget::keyPressEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::keyReleaseEvent(QKeyEvent * event)
{
    m_inputHandler.handleEvent(event);
    QOpenGLWidget::keyReleaseEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::wheelEvent(QWheelEvent * event)
{
    m_inputHandler.handleEvent(event);
    QOpenGLWidget::wheelEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    m_inputHandler.handleEvent(event);
    //QOpenGLWidget::mouseDoubleClickEvent(event); // Don't send another press event
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::mouseMoveEvent(QMouseEvent * event)
{
    m_inputHandler.handleEvent(event);
    //event->ignore();
    QOpenGLWidget::mouseMoveEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::mousePressEvent(QMouseEvent * event)
{
    m_inputHandler.handleEvent(event);
    QOpenGLWidget::mousePressEvent(event);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::mouseReleaseEvent(QMouseEvent * event)
{
    m_inputHandler.handleEvent(event);
    QOpenGLWidget::mouseReleaseEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::initializeConnections()
{
    // Why wasn't I just doing this in onResize?
    //connect(this, &GLWidget::resized, this, static_cast<void (GLWidget::*)(int w, int h)>(&GLWidget::resizeProjections));
    connect(m_engine, &CoreEngine::scenarioNeedsRedraw, this, static_cast<void (GLWidget::*)()>(&GLWidget::resizeProjections));
}



}} // end namespacing
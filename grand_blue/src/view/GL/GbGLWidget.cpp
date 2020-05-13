// Internal
#include "GbGLWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/rendering/shaders/GbShaders.h"

#include "../../core/scene/GbScenario.h"
#include "../../core/components/GbCamera.h"
#include "../../core/loop/GbSimLoop.h"
#include "../../core/rendering/renderer/GbRenderers.h"
#include "../../core/rendering/renderer/GbMainRenderer.h"

namespace Gb { 
namespace View {

/////////////////////////////////////////////////////////////////////////////////////////////
GLWidget::GLWidget(const QString& name, CoreEngine* engine,
    QWidget * parent) :
    AbstractService(name),
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
    rp->updateAspectRatio(width(), height());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::clear()
{
    m_renderProjections.clear();
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
	m_renderer = std::make_shared<GL::MainRenderer>(m_engine, this);
	m_renderer->initialize();

    // Emit signal that GL context is initialized
    emit initializedContext();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    if (m_engine->scenario()) {
        m_renderer->render();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::paintGL()
{
    if (m_engine->scenario()) {
        m_renderer->render();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::updateAspectRatios(int width, int height) {
    for (const auto& rp : m_renderProjections) {
        rp->updateAspectRatio(width, height);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GLWidget::updateAspectRatios() {
    updateAspectRatios(width(), height());
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
    connect(this, &GLWidget::resized, this, static_cast<void (GLWidget::*)(int w, int h)>(&GLWidget::updateAspectRatios));
    connect(m_engine, &CoreEngine::scenarioNeedsRedraw, this, static_cast<void (GLWidget::*)()>(&GLWidget::updateAspectRatios));
}



}} // end namespacing
// Internal
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"

#include <QScreen>

namespace rev {

GLWidgetInterface::GLWidgetInterface(const QString& name, std::unique_ptr<InputHandlerInterface>&& inputHandler, QWidget * parent) :
    rev::NameableInterface(name.toStdString()),
    QOpenGLWidget(parent),
    m_inputHandler(std::move(inputHandler))
{


    // Ensure that mouse is tracked even when button is not clicked
    setMouseTracking(true);

    // Ensure that widget accepts keyboard focus
    setFocusPolicy(Qt::StrongFocus);
}

GLWidgetInterface::~GLWidgetInterface()
{
    // Make context current for cleanup
    makeCurrent();
}


uint32_t GLWidgetInterface::pixelWidth() const
{
    uint32_t dpr = screen()->devicePixelRatio();
    return width() * dpr;
}
uint32_t GLWidgetInterface::pixelHeight() const
{
    uint32_t dpr = screen()->devicePixelRatio();
    return height() * dpr;
}

void GLWidgetInterface::resizeEvent(QResizeEvent * event)
{
    // Perform base class resize event
    QOpenGLWidget::resizeEvent(event);

    // Emit resize signal (connects to RenderProjection class)
    // For high-resolution devices, the device-pixel-ratio will be greater than 1.0,
    // so need to scale for true pixel value
    emit resized(pixelWidth(), pixelHeight());

#ifdef DEBUG_MODE
    //int w = width();
    //int h = height();
#endif
}

void GLWidgetInterface::contextMenuEvent(QContextMenuEvent * event)
{
    Q_UNUSED(event);
}

void GLWidgetInterface::keyPressEvent(QKeyEvent * event)
{
    m_inputHandler->handleEvent(event);
    QOpenGLWidget::keyPressEvent(event);
}

void GLWidgetInterface::keyReleaseEvent(QKeyEvent * event)
{
    m_inputHandler->handleEvent(event);
    QOpenGLWidget::keyReleaseEvent(event);
}

void GLWidgetInterface::wheelEvent(QWheelEvent * event)
{
    m_inputHandler->handleEvent(event);
    QOpenGLWidget::wheelEvent(event);
}

void GLWidgetInterface::mouseDoubleClickEvent(QMouseEvent * event)
{
    m_inputHandler->handleEvent(event);
    //QOpenGLWidget::mouseDoubleClickEvent(event); // Don't send another press event
}

void GLWidgetInterface::mouseMoveEvent(QMouseEvent * event)
{
    m_inputHandler->handleEvent(event);
    //event->ignore();
    QOpenGLWidget::mouseMoveEvent(event);
}

void GLWidgetInterface::mousePressEvent(QMouseEvent * event)
{
    m_inputHandler->handleEvent(event);
    QOpenGLWidget::mousePressEvent(event);
}

void GLWidgetInterface::mouseReleaseEvent(QMouseEvent * event)
{
    m_inputHandler->handleEvent(event);
    QOpenGLWidget::mouseReleaseEvent(event);
}


} // end namespacing
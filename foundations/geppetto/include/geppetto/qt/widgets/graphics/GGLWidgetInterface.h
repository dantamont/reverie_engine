#pragma once

// QT
#include <QOpenGLWidget>
//#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

// Internal
#include "fortress/containers/math/GVector.h"
#include "fortress/types/GNameable.h"
#include "geppetto/qt/input/GInputHandlerInterface.h"

namespace rev {

class GLWidgetInterface : public QOpenGLWidget, public rev::NameableInterface 
{
    Q_OBJECT
public:

    GLWidgetInterface(const QString& name, std::unique_ptr<InputHandlerInterface>&& inputHandler, QWidget *parent);
    virtual ~GLWidgetInterface();


    /// @brief Return the position of the mouse relative to this widget
    Vector2 widgetMousePosition() const {
        return m_inputHandler->mouseHandler().widgetMousePosition();
    }

    InputHandlerInterface& inputHandler() { return *m_inputHandler; }

    /// @brief Reset widget (on engine clear)
    virtual void clear() = 0;

    /// @brief Request a resize of the widget
    virtual void requestResize() = 0;

    /// @brief Obtain true dimensions, as scaled with device-pixel-ratio
    uint32_t pixelWidth() const;
    uint32_t pixelHeight() const;
    
    /// @name QWidget Overrides
    /// @{

    /// @brief Override resize event to send out signal
    virtual void resizeEvent(QResizeEvent* event) override;

    /// @}
signals:
    /// @brief Sent on resize
    void resized(int width, int height);

    /// @brief Send on initialization
    void initializedContext();

protected:

    friend class InputHandlerInterface;

    /// @brief Override contextMenuEvent
    void contextMenuEvent(QContextMenuEvent *event) override;

    /// @brief Input handlers
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    /// @brief Input handler for the GL widget
    std::unique_ptr<InputHandlerInterface> m_inputHandler{ nullptr };

};

} // End rev


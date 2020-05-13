#ifndef GB_GLOBE_GL_WIDGET_H
#define GB_GLOBE_GL_WIDGET_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// QT
#include <QOpenGLWidget>
//#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

// Internal
#include "../../core/service/GbService.h"
#include "../../core/GbObject.h"
#include "../../core/input/GbInputHandler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations 
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
    class CoreEngine; 
    class RenderProjection;

	namespace GL {
		class MainRenderer;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes  
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Gb { 
namespace View {

class GLWidget : public QOpenGLWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    GLWidget(const QString& name, Gb::CoreEngine* engine, QWidget *parent);
    ~GLWidget();
    /// @}

    //---------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Return the position of the mouse relative to this widget
    Vector2g widgetMousePosition() const {
        return m_inputHandler.mouseHandler().widgetMousePosition();
    }

    std::shared_ptr<GL::MainRenderer> renderer() { return m_renderer; }

    InputHandler& inputHandler() { return m_inputHandler; }

    //int lastWidth() const { return m_lastWidth; }
    //int lastHeight() const { return m_lastHeight; }

    /// @}

    //---------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void addRenderProjection(RenderProjection* rp);

    /// @brief Reset widget (on engine clear)
    void clear();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "GLWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::GLWidget"; }

    /// @brief Indicates whether this Service has a UI or not.
    virtual bool hasUi() const { return true; };

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const { return false; };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
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

public slots:
    /// @brief Update aspect ratios for all render projections
    void updateAspectRatios(int width, int height);
    void updateAspectRatios();

protected:
    //---------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class InputHandler;

    /// @}
    //---------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

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

    /// @brief GL-specific overrides
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

    /// @brief set up signal/slot connections to render manager
    void initializeConnections();

    /// @}

	//---------------------------------------------------------------------------------------------
	/// @name Protected Members
	/// @{

	/// @brief core engine
	Gb::CoreEngine* m_engine;

	/// @brief 3D rendering object
	std::shared_ptr<GL::MainRenderer> m_renderer;

    /// @brief Render projections associated with this widget
    std::vector<RenderProjection*> m_renderProjections;

    /// @brief Input handler for the GL widget
    InputHandler m_inputHandler;

    /// @brief Last width and height from gl resize
    //int m_lastWidth;
    //int m_lastHeight;

	/// @}
};


///////////////////////////////////////////////////////////////////////////////////////////
}}

#endif 

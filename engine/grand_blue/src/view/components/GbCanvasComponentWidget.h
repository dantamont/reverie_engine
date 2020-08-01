#ifndef GB_CANVAS_COMPONENT_WIDGET_H
#define GB_CANVAS_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"
#include "../parameters/GbRenderableWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CanvasComponent;

namespace View {
class CanvasGlyphWidget;
///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CanvasSubWidget
class CanvasSubWidget : public ParameterWidget {
    Q_OBJECT
public:
    CanvasSubWidget(CoreEngine* core, CanvasComponent* camera, QWidget* parent = nullptr);

protected:

    CanvasComponent* m_canvasComponent;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CanvasViewportWidget
class CanvasViewportWidget : public CanvasSubWidget {
    Q_OBJECT
public:
    CanvasViewportWidget(CoreEngine* core, CanvasComponent* c, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QLineEdit* m_depth;
    QLineEdit* m_xCoordinate;
    QLineEdit* m_yCoordinate;
    QLineEdit* m_width;
    QLineEdit* m_height;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CanvasProjectionWidget
class CanvasProjectionWidget : public CanvasSubWidget {
    Q_OBJECT
public:
    CanvasProjectionWidget(CoreEngine* core, CanvasComponent* c, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QComboBox* m_projectionType;
    QStackedWidget* m_stackedWidget;

    // Perspective
    QLineEdit* m_fov;
    QLineEdit* m_near;
    QLineEdit* m_far;

    // Orthographic
    QLineEdit* m_left;
    QLineEdit* m_right;
    QLineEdit* m_bottom;
    QLineEdit* m_top;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CanvasComponentWidget
/// @brief Widget representing a character controller component
class CanvasComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CanvasComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~CanvasComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a shader component
    CanvasComponent* canvasComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    CanvasComponent* m_canvasComponent;

    CanvasGlyphWidget* m_glyphs;

    RenderableWidget<CanvasComponent>* m_renderable;

    CanvasViewportWidget* m_viewport;

    CanvasProjectionWidget* m_projection;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
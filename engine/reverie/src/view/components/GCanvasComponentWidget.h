#ifndef GB_CANVAS_COMPONENT_WIDGET_H
#define GB_CANVAS_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidgets.h"
#include "../parameters/GRenderableWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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
/// @class BillboardFlagsWidget
class BillboardFlagsWidget : public CanvasSubWidget {
    Q_OBJECT
public:
    BillboardFlagsWidget(CoreEngine* core, CanvasComponent* glyph, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    friend class CanvasComponentWidget;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QScrollArea* m_area;
    QWidget* m_areaWidget;
    std::vector<QCheckBox*> m_checkBoxes;
};

// Deprecated, canvas no longer has viewport
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///// @class CanvasViewportWidget
//class CanvasViewportWidget : public CanvasSubWidget {
//    Q_OBJECT
//public:
//    CanvasViewportWidget(CoreEngine* core, CanvasComponent* c, QWidget* parent = nullptr);
//
//    virtual void update() override;
//
//protected:
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    QLineEdit* m_depth;
//    QLineEdit* m_xCoordinate;
//    QLineEdit* m_yCoordinate;
//    QLineEdit* m_width;
//    QLineEdit* m_height;
//};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///// @class CanvasProjectionWidget
//class CanvasProjectionWidget : public CanvasSubWidget {
//    Q_OBJECT
//public:
//    CanvasProjectionWidget(CoreEngine* core, CanvasComponent* c, QWidget* parent = nullptr);
//
//    virtual void update() override;
//
//protected:
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    QComboBox* m_projectionType;
//    QStackedWidget* m_stackedWidget;
//
//    // Perspective
//    QLineEdit* m_fov;
//    QLineEdit* m_near;
//    QLineEdit* m_far;
//
//    // Orthographic
//    QLineEdit* m_left;
//    QLineEdit* m_right;
//    QLineEdit* m_bottom;
//    QLineEdit* m_top;
//};


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

    QComboBox* m_glyphMode;

    BillboardFlagsWidget* m_billboardFlags;

    CanvasGlyphWidget* m_glyphs;

    //CanvasViewportWidget* m_viewport;

    //CanvasProjectionWidget* m_projection;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
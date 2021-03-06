#ifndef GB_CAMERA_COMPONENT_WIDGET_H
#define GB_CAMERA_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidgets.h"
#include "../parameters/GColorWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CameraComponent;

namespace View {

class RenderLayerSelectWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CameraSubWidget
class CameraSubWidget : public ParameterWidget {
    Q_OBJECT
public:
    CameraSubWidget(CoreEngine* core, CameraComponent* camera, QWidget* parent = nullptr);

protected:

    CameraComponent* m_cameraComponent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CameraOptionsWidget
class CameraOptionsWidget : public CameraSubWidget {
    Q_OBJECT
public:
    CameraOptionsWidget(CoreEngine* core, CameraComponent* camera, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QScrollArea* m_area;
    QWidget* m_areaWidget;
    std::vector<QCheckBox*> m_checkBoxes;
    ColorWidget* m_clearColor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CameraViewportWidget
class CameraViewportWidget : public CameraSubWidget {
    Q_OBJECT
public:
    CameraViewportWidget(CoreEngine* core, CameraComponent* camera, QWidget* parent = nullptr);

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
/// @class CameraProjectionWidget
class CameraProjectionWidget : public CameraSubWidget {
    Q_OBJECT
public:
    CameraProjectionWidget(CoreEngine* core, CameraComponent* camera, QWidget* parent = nullptr);

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
/// @class CameraComponentWidget
/// @brief Widget representing a character controller component
class CameraComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CameraComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~CameraComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a shader component
    CameraComponent* cameraComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    CameraOptionsWidget* m_cameraOptions;
    CameraViewportWidget* m_viewport;
    CameraProjectionWidget* m_projection;
    RenderLayerSelectWidget* m_renderLayers;
    QComboBox* m_cubeMaps;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
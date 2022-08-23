#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"
#include "geppetto/qt/widgets/types/GColorWidget.h"
#include "enums/GCameraOptionEnum.h"

#include "ripple/network/messages/GModifyCameraClearColorMessage.h"
#include "ripple/network/messages/GModifyCameraOptionFlagsMessage.h"
#include "ripple/network/messages/GModifyCameraViewportMessage.h"
#include "ripple/network/messages/GModifyCameraRenderProjectionMessage.h"
#include "ripple/network/messages/GModifyCameraCubemapMessage.h"
#include "ripple/network/messages/GRequestCubemapsDataMessage.h"
#include "ripple/network/messages/GCubemapsDataMessage.h"

namespace rev {

class RenderLayerSelectWidget;
class WidgetManager;

/// @class CameraSubWidget
class CameraSubWidget : public ParameterWidget {
public:
    CameraSubWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

protected:

    bool checkCameraOptionFlag(ECameraOption option) const;
    json& getRenderLayers();
    ECameraOptions getCameraOptions() const;

    json& m_cameraComponentJson;
    Int32_t m_sceneObjectId { -1 };
};

/// @class CameraOptionsWidget
class CameraOptionsWidget : public CameraSubWidget {
    Q_OBJECT
public:
    CameraOptionsWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    Color getCameraClearColor();

    Color m_cachedClearColor; ///< Clear color for the color widget

    QScrollArea* m_area{ nullptr };
    QWidget* m_areaWidget{ nullptr };
    std::vector<QCheckBox*> m_checkBoxes;
    ColorWidget* m_clearColor{ nullptr };

    GModifyCameraClearColorMessage m_clearColorMessage;
    GModifyCameraOptionFlagsMessage m_optionsMessage;
};

/// @class CameraViewportWidget
class CameraViewportWidget : public CameraSubWidget {
public:
    CameraViewportWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    json& getViewportJson();
    json& getViewportX();
    json& getViewportY();
    json& getViewportWidth();
    json& getViewportHeight();
    json& getViewportDepth();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    QLineEdit* m_depth{ nullptr };
    QLineEdit* m_xCoordinate{ nullptr };
    QLineEdit* m_yCoordinate{ nullptr };
    QLineEdit* m_width{ nullptr };
    QLineEdit* m_height{ nullptr };
    GModifyCameraViewportMessage m_viewportMessage;
};

/// @class CameraProjectionWidget
class CameraProjectionWidget : public CameraSubWidget {
public:
    CameraProjectionWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    json& getProjectionJson();
    json& getFovX();
    json& getNearClipPlane();
    json& getFarClipPlane();
    json& getLeftBound();
    json& getTopBound();
    json& getRightBound();
    json& getBottomBound();
    json& getProjectionType();

    QComboBox* m_projectionType{ nullptr };
    QStackedWidget* m_stackedWidget{ nullptr };

    // Perspective
    QLineEdit* m_fov{ nullptr };
    QLineEdit* m_near{ nullptr };
    QLineEdit* m_far{ nullptr };

    // Orthographic
    QLineEdit* m_left{ nullptr };
    QLineEdit* m_right{ nullptr };
    QLineEdit* m_bottom{ nullptr };
    QLineEdit* m_top{ nullptr };

    GModifyCameraRenderProjectionMessage m_projectionMessage;
};

/// @class CameraComponentWidget
/// @brief Widget representing a character controller component
class CameraComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    CameraComponentWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~CameraComponentWidget();

    /// @}

protected slots:

    void populateCubemapWidget(const GCubemapsDataMessage& message);

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void requestCubemapsData();

    /// @}

    /// @name Private Members
    /// @{

    GModifyCameraCubemapMessage m_modifyCubemapMessage;

    CameraOptionsWidget* m_cameraOptions{ nullptr };
    CameraViewportWidget* m_viewport{ nullptr };
    CameraProjectionWidget* m_projection{ nullptr };
    RenderLayerSelectWidget* m_renderLayers{ nullptr };
    QComboBox* m_cubeMaps{ nullptr };

    /// @}
};


// End namespaces        
}

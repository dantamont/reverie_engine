#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"
#include "geppetto/qt/widgets/types/GVectorWidget.h"
#include "geppetto/qt/widgets/types/GColorWidget.h"

#include "ripple/network/messages/GUpdateLightComponentMessage.h"
#include "ripple/network/messages/GSetLightComponentTypeMessage.h"
#include "ripple/network/messages/GToggleLightComponentShadowsMessage.h"
#include "ripple/network/messages/GRequestRenderSettingsInfoMessage.h"
#include "ripple/network/messages/GRenderSettingsInfoMessage.h"

namespace rev {

/// @brief For widgets to each have required update messages
class LightWidgetBaseInterface {
public:
    LightWidgetBaseInterface(Uint32_t sceneObjectId);

protected:

    virtual void sendUpdateLightMessage(WidgetManager* wm) = 0;

    GUpdateLightComponentMessage m_updateLightComponentMessage;
};

/// @brief For widgets to each have required update messages
class LightSubWidgetInterface: public LightWidgetBaseInterface {
public:
    LightSubWidgetInterface(Uint32_t sceneObjectId, json& lightComponentJson);

protected:

    void sendUpdateLightMessage(WidgetManager* wm) override;

    json& m_lightComponentJson;
};

/// @brief For parent light widget required update messages
class LightWidgetInterface: public LightWidgetBaseInterface {
public:
    LightWidgetInterface(Uint32_t sceneObjectId, const json& lightComponentJson);

protected:

    void sendUpdateLightMessage(WidgetManager* wm) override;

    json m_lightComponentJson;
};

/// @class LightDiffuseColorWidget
class LightDiffuseColorWidget : public ColorWidget, public LightSubWidgetInterface {
public:
    LightDiffuseColorWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateColor() override;

    Color m_cachedColor; ///< Cached color so that color widget may be used with JSON
    json& m_lightComponentJson;
};


/// @class LightAmbientColorWidget
class LightAmbientColorWidget : public ColorWidget, public LightSubWidgetInterface {
public:
    LightAmbientColorWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateColor() override;


    Color m_cachedColor; ///< Cached color so that color widget may be used with JSON
    json& m_lightComponentJson;
};

/// @class LightSpecularColorWidget
class LightSpecularColorWidget : public ColorWidget, public LightSubWidgetInterface {
public:
    LightSpecularColorWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateColor() override;

    Color m_cachedColor; ///< Cached color so that color widget may be used with JSON
    json& m_lightComponentJson;
};

/// @class LightDirectionWidget
class LightDirectionWidget : public VectorWidget<float, 4>, public LightSubWidgetInterface {
public:
    LightDirectionWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateVectorFromWidget() override;

    Vector4f m_cachedVector; ///< Cached vector so that vector widget may be used with JSON
    json& m_lightComponentJson;
};

/// @class LightAttenuationWidget
class LightAttenuationWidget : public VectorWidget<float, 4>, public LightSubWidgetInterface {
public:
    LightAttenuationWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateVectorFromWidget() override;

    Vector4f m_cachedVector; ///< Cached vector so that vector widget may be used with JSON
    json& m_lightComponentJson;
};

/// @class LightCutoffWidget
class LightCutoffWidget : public VectorWidget<float, 4>, public LightSubWidgetInterface {
public:
    LightCutoffWidget(WidgetManager* wm, json& lightComponentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateVectorFromWidget() override;

    Vector4f m_cachedVector; ///< Cached vector so that vector widget may be used with JSON
    json& m_lightComponentJson;
};


/// @class LightComponentWidget
/// @brief Widget representing a light component
class LightComponentWidget : public SceneObjectComponentWidget, public LightWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    LightComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~LightComponentWidget();

    /// @}

    void update(const GRenderSettingsInfoMessage& message);

private:
    /// @name Private Methods
    /// @{

    /// @brief Use cached JSON to determine whether or not shadows can be added for the current light type
    bool canAddShadowsToLight() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Toggle widgets based on light type
    void toggleWidgets();

    /// @}

    /// @name Private Members
    /// @{

    /// @brief Light type widget
    QComboBox* m_lightType;
    QLineEdit* m_lightIntensity;
    QLineEdit* m_lightRange;
    QLineEdit* m_minBias;
    QLineEdit* m_maxBias;
    QLineEdit* m_nearPlane;
    QCheckBox* m_castShadows;

    LightDirectionWidget* m_lightDirection; // for directional and spot lights
    LightAttenuationWidget* m_lightAttenuation; // for point light
    LightCutoffWidget* m_lightCutoffs; // for spot light

    LightDiffuseColorWidget*  m_diffuseColorWidget;
    LightAmbientColorWidget*  m_ambientColorWidget;
    LightSpecularColorWidget* m_specularColorWidget;

    GSetLightComponentTypeMessage m_setLightcomponentTypeMessage;
    GToggleLightComponentShadowsMessage m_toggleLightcomponentShadowsMessage;
    GRequestRenderSettingsInfoMessage m_requestRenderSettingsInfoMessage;

    GRenderSettingsInfoMessage m_renderSettingsInfoMessage; ///< Updated when updated render settings are received

    /// @}
};


// End namespaces
}
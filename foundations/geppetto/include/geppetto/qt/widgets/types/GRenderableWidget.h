#pragma once

// Project
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"
#include "fortress/containers/GSortingLayer.h"

//#include "ripple/network/messages/GRequestUniformValueMessage.h"
//#include "ripple/network/messages/GUniformValueMessage.h"
//#include "ripple/network/messages/GUpdateUniformMessage.h"
#include "ripple/network/messages/GUpdateRenderSettingsMessage.h"

namespace rev {

class RenderUniformsWidget;
class RenderLayerSelectItem;
class RenderLayerSelectWidget;

///// @class UniformWidget
//class UniformWidget : public ParameterWidget {
//    Q_OBJECT
//public:
//    UniformWidget(WidgetManager* wm, const json& uniformJson, Uint32_t sceneObjectId, RenderUniformsWidget* parent = nullptr);
//
//    virtual void update() override;
//
//    void update(const GUniformValueMessage& message);
//
//protected:
//
//    virtual void mousePressEvent(QMouseEvent* event) override;
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    void requestUpdateUniform();
//
//    GUpdateUniformMessage m_updateUniformMessage;
//    RenderUniformsWidget* m_parent{ nullptr };
//    Int32_t m_sceneObjectId{ -1 };
//    json m_uniformJson;
//    QLineEdit* m_uniformName{ nullptr };
//    JsonWidget* m_uniformValue{ nullptr };
//};
//
//
///// @class RenderUniformWidget
//class RenderUniformsWidget : public ParameterWidget {
//    Q_OBJECT
//public:
//    RenderUniformsWidget(CoreEngine* core, Shadable& renderable, QWidget* parent = nullptr);
//    virtual ~RenderUniformsWidget();
//
//    virtual void update() override;
//
//    void setCurrentUniformWidget(UniformWidget* uniform)
//    {
//        m_currentUniformWidget = uniform;
//    }
//
//protected:
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    void clearUniforms();
//    void repopulateUniforms();
//
//    void addUniform();
//    void removeCurrentUniform();
//
//#ifndef QT_NO_CONTEXTMENU
//    /// @brief Generates a context menu, overriding default implementation
//    /// @note Context menus can be executed either asynchronously using the popup() function or 
//    ///       synchronously using the exec() function
//    void contextMenuEvent(QContextMenuEvent *event) override;
//
//#endif // QT_NO_CONTEXTMENU
//
//    Shadable& m_renderable;
//    QAction* m_addUniform;
//    QAction* m_removeUniform;
//
//    UniformWidget* m_currentUniformWidget = nullptr;
//
//    QScrollArea* m_area;
//    QWidget* m_areaWidget;
//    std::vector<UniformWidget*> m_uniformWidgets;
//};


/// @class TransparencyTypeWidget
class TransparencyTypeWidget : public ParameterWidget {
    Q_OBJECT
public:
    TransparencyTypeWidget(WidgetManager* wm, Uint32_t transparencyType, QWidget* parent = nullptr);
    ~TransparencyTypeWidget();

    virtual void update() override;

protected:

    void requestUpdateRenderSettings();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;


    Int32_t m_transparencyType{ -1 };

    // Transparency type
    QComboBox* m_transparencyTypeWidget{ nullptr };
};


enum class RenderSettingsOwnerType {
    kInvalid = -1,
    kModelComponent = 0 ///< The render settings are associated with a model component
};

/// @class RenderSettingsWidget
class RenderSettingsWidget : public ParameterWidget {
    Q_OBJECT
public:
    RenderSettingsWidget(WidgetManager* wm, json& settings, RenderSettingsOwnerType ownerType, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    friend class TransparencyTypeWidget;

    void requestUpdateRenderSettings();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    int getDepthMode(int modeWidgetIndex) const;
    int getCulledFace(int faceWidgetIndex) const;

    json& getTransparencyModeJson();
    json& getDepthModeJson();
    json& getDepthTestEnabledJson();
    json& getCullFaceJson();
    json& getCulledFaceJson();

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;

#endif // QT_NO_CONTEXTMENU

    GUpdateRenderSettingsMessage m_updateMessage;

    TransparencyTypeWidget* m_transparency{ nullptr }; ///< Transparency widget
    RenderSettingsOwnerType m_ownerType{ RenderSettingsOwnerType::kInvalid };
    json& m_renderSettingsJson;

    // Blend Settings
    QComboBox* m_blendMode{ nullptr };

    // Depth Settings
    QComboBox* m_depthMode{ nullptr };
    QCheckBox* m_depthTest{ nullptr };

    // Cull settings
    QComboBox* m_culledFace{ nullptr };
    QCheckBox* m_cullFace{ nullptr };
};

//
///// @class RenderableWidget
//template <typename T>
//class RenderableWidget : public ParameterWidget {
//public:
//
//    /// @name Constructors and Destructors
//    /// @{
//    RenderableWidget(CoreEngine* core, T* r, QWidget* parent = nullptr) :
//        ParameterWidget(core, parent),
//        m_renderable(r)
//    {
//        initialize();
//    }
//    virtual ~RenderableWidget() {
//        delete m_uniforms;
//        delete m_renderSettings;
//    }
//
//    /// @}
//
//    /// @name Public Methods
//    /// @{
//
//    virtual void update() override {
//    }
//
//    /// @}
//
//protected:
//
//    /// @name Protected Methods
//    /// @{
//
//    virtual void initializeWidgets() override {
//        m_uniforms = new RenderUniformsWidget(m_engine, *m_renderable);
//        m_renderSettings = new RenderSettingsWidget(m_engine, m_renderable->renderSettings());
//        //m_transparency = new TransparencyTypeWidget(m_engine, m_renderable->transparencyType());
//    }
//    virtual void initializeConnections() override {
//    }
//    virtual void layoutWidgets() override {
//        m_mainLayout = new QVBoxLayout();
//        m_mainLayout->setSpacing(0);
//        m_mainLayout->addWidget(new QLabel("Uniforms:"));
//        m_mainLayout->addWidget(m_uniforms);
//        m_mainLayout->addWidget(new QLabel("Render Settings:"));
//        m_mainLayout->addWidget(m_renderSettings);
//    }
//
//    /// @}
//
//
//    /// @name Protected Members
//    /// @{
//
//    T* m_renderable;
//
//    /// @brief Uniform combobox
//    RenderUniformsWidget* m_uniforms;
//
//    /// @brief Render settings widget
//    RenderSettingsWidget* m_renderSettings;
//
//    /// @brief Render layer combobox
//    RenderLayerSelectWidget* m_renderLayers;
//
//    /// @}
//
//};


} // rev

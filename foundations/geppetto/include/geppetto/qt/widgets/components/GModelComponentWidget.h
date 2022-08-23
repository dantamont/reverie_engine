#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"

#include "ripple/network/messages/GSetSceneObjectModelMessage.h"
#include "ripple/network/messages/GRequestModelDataMessage.h"
#include "ripple/network/messages/GModelDataMessage.h"


namespace rev {

/// @class ModelSelectWidget
class ModelSelectWidget : public ParameterWidget {
    Q_OBJECT
public:
    ModelSelectWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent = nullptr);

    virtual void update() override;

public slots:
    void populateModels(const GModelDataMessage& message);

protected:

    void requestPopulateModels();

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    GSetSceneObjectModelMessage m_setModelMessage;
    GRequestModelDataMessage m_requestModelMessage;
    Int32_t m_sceneObjectId{ -1 };
    json& m_componentJson;
    QComboBox* m_modelSelect{ nullptr };
};


/// @class ModelComponentWidget
/// @brief Widget representing a character controller component
class ModelComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    ModelComponentWidget(WidgetManager* wm, json& component, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~ModelComponentWidget();

    /// @}

    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @}

public slots:
signals:
private slots:
private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    RenderSettingsWidget* m_renderSettings{ nullptr };
    ModelSelectWidget* m_model{ nullptr };

    /// @}
};


// End namespaces        
}
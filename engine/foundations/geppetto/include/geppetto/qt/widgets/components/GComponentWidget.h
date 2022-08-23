#pragma once

// Qt
#include <QtWidgets>

// External
#include "fortress/json/GJson.h"

#include "enums/GSceneComponentTypeEnum.h"
#include "enums/GSceneObjectComponentTypeEnum.h"
#include "enums/GSceneTypeEnum.h"

#include "ripple/network/messages/GToggleComponentMessage.h"
#include "ripple/network/gateway/GMessageGateway.h"

// Internal
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

/// Forward declarations
namespace rev {

/// @class ComponentWidget
/// @brief Base class for all component widgets
template<ESceneType TSceneType>
class ComponentWidget : public ParameterWidget {
public:
    /// @name Constructors/Destructor
    /// @{

    ComponentWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneOrObjectId = -1, QWidget *parent = 0) :
        ParameterWidget(wm, parent),
        m_componentJson(componentJson),
        m_sceneOrObjectId(sceneOrObjectId)
    {
        m_toggleMessage.setSceneType(static_cast<GSceneType>(TSceneType));
        m_toggleMessage.setComponentId(getComponentId());
        m_toggleMessage.setComponentType(getComponentType());
        m_toggleMessage.setSceneOrObjectId(sceneOrObjectId);
    }
    ~ComponentWidget() = default;

    /// @}

protected:
    /// @name Private Methods
    /// @{

    Uuid getComponentId() const {
        return m_componentJson["id"].get<Uuid>();
    }

    Int32_t getComponentType() const {
        return m_componentJson["type"].get<Int32_t>();
    }

    bool isComponentEnabled() const {
        return m_componentJson["isEnabled"].get<bool>();
    }

    const GString getComponentClassName() const {
        if constexpr (ESceneType::eSceneObject == TSceneType) {
            return GSceneObjectComponentType::ToString(GSceneObjectComponentType(getComponentType())) + "Component";
        }
        else {
            return GSceneComponentType::ToString(GSceneComponentType(getComponentType())) + "Component";
        }
    }

    virtual void initializeWidgets() override {
        m_toggle = new QCheckBox(nullptr);
        m_toggle->setChecked(isComponentEnabled());
        m_typeLabel = new QLabel(getComponentClassName().c_str());
        m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    virtual void initializeConnections() override {
        connect(m_toggle, &QCheckBox::stateChanged, this, [this](int state) {
            m_toggleMessage.setToggleState(static_cast<bool>(state));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_toggleMessage);
            });

    }

    virtual void layoutWidgets() override {
        m_mainLayout = new QVBoxLayout;

        QBoxLayout* layout = new QHBoxLayout;
        layout->setMargin(0);
        layout->addWidget(m_toggle);
        layout->addWidget(m_typeLabel);
        layout->setAlignment(Qt::AlignCenter);

        m_mainLayout->addLayout(layout);
    }


    /// @}

    /// @name Private Members
    /// @{

    QCheckBox* m_toggle{ nullptr };
    QLabel* m_typeLabel{ nullptr };
    Int32_t m_sceneOrObjectId{ -1 }; ///< Scene or scene object ID
    json m_componentJson; ///< The JSON representing the component
    GToggleComponentMessage m_toggleMessage; ///< Message for toggling component

    /// @}
};

typedef ComponentWidget<ESceneType::eSceneObject> SceneObjectComponentWidget;
typedef ComponentWidget<ESceneType::eScene> SceneComponentWidget;

// End namespaces
}

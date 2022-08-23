#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"

#include "ripple/network/messages/GRequestAnimationStateMachinesMessage.h"
#include "ripple/network/messages/GAnimationStateMachinesMessage.h"
#include "ripple/network/messages/GSetAnimationComponentStateMachineMessage.h"
#include "ripple/network/messages/GRenameAnimationStateMachineMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineMessage.h"

namespace rev {

class BoneAnimationComponent;
class AnimationNodeWidget;
class EditableComboBox;

/// @class AnimationComponentWidget
/// @brief Generic component widget that uses JSON data to modify components
class AnimationComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    AnimationComponentWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~AnimationComponentWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Accessor for the node widget
    AnimationNodeWidget* nodeWidget() const { return m_animationNodeWidget; }

    /// @brief Remove node widget upon close
    void clearNodeWidget();

    void update(const GAnimationStateMachinesMessage& message);

    /// @}

protected:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    EditableComboBox* m_stateMachineSelect{ nullptr };
    QPushButton* m_launchGraphButton{ nullptr };
    AnimationNodeWidget* m_animationNodeWidget{ nullptr };

    GRequestAnimationStateMachinesMessage m_requestAnimationStateMachinesMessage;
    GSetAnimationComponentStateMachineMessage m_setComponentStateMachineMessage;
    GRenameAnimationStateMachineMessage m_renameMachineMessage;
    GAddAnimationStateMachineMessage m_addStateMachineMessage;

    /// @}
};



// End namespaces        
}

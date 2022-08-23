#pragma once

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "ripple/network/messages/GModifyAnimationStateMachineTransitionMessage.h"

namespace rev {

class AnimationClipTreeWidget;
class AnimationNodeWidget;

/// @class AnimStateWidget
class AnimStateWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimStateWidget(WidgetManager* wm, json& stateJson, json& animationComponentState, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent = nullptr);
    virtual ~AnimStateWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override;

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Refresh messages from JSON
    void refreshMessageFromJson();

    /// @}

private:
    /// @name Private Members
    /// @{

    json& m_stateJson;
    json& m_animationComponentJson;
    Uint32_t m_sceneObjectId;

    QLabel* m_label{ nullptr };
    AnimationNodeWidget* m_nodeWidget{ nullptr }; ///< Animation node widget, which may or may not be the parent
    AnimationClipTreeWidget* m_clipWidget{ nullptr };

    QLabel* m_transitionInfo{ nullptr };
    QComboBox* m_transitionTypes{ nullptr };
    QLineEdit* m_fadeInTimeSec{ nullptr };
    QLineEdit* m_fadeInBlendWeight{ nullptr };
    QLineEdit* m_fadeOutTimeSec{ nullptr };
    QLineEdit* m_fadeOutBlendWeight{ nullptr };

    GModifyAnimationStateMachineTransitionMessage m_modifyTransitionMessage;

    /// @}

};

} // rev

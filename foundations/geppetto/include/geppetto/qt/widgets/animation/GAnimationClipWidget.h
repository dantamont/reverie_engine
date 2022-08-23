#pragma once

// Includes
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "ripple/network/messages/GModifyAnimationClipMessage.h"
#include "ripple/network/messages/GRequestAnimationDataMessage.h"
#include "ripple/network/messages/GAnimationDataMessage.h"

namespace rev {

class AnimationNodeWidget;
class AnimationTreeWidget;

/// @class AnimClipWidget
class AnimClipWidget : public ParameterWidget {
public:
    /// @name Constructors and Destructors
    /// @{
    AnimClipWidget(WidgetManager* wm, json& animationClipJson, Uuid stateId, Uuid stateMachineId, AnimationNodeWidget* nodeWidget, QWidget* parent = nullptr);
    virtual ~AnimClipWidget();

    /// @}

    /// @name Public Methods
    /// @{

    void update(const GAnimationDataMessage& message);

    virtual void update() override;

    /// @}

protected:
    
    /// @name Protected Methods
    /// @{

    QString getClipName() const;
    GString getAnimationName() const;

    Float64_t getSpeedFactor() const;
    Float64_t getDuration() const;
    Float64_t getTickDelay() const;
    Float64_t getTimeDelay() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Request animation info from the main application
    void requestAnimationInfo();

    /// @}

private:
    /// @name Private Members
    /// @{

    json& m_clipJson; ///< The JSON representing the clip
    Uuid m_stateId;
    Uuid m_stateMachineId;
    AnimationNodeWidget* m_nodeWidget; ///< Node widget for connecting signals/slots

    Float64_t m_animationDurationSec{ -1 };
    QLineEdit* m_nameEdit{ nullptr };
    QLineEdit* m_speedFactor{ nullptr }; ///< Speed scaling of the clip
    QLineEdit* m_playbackDuration{ nullptr }; ///< In seconds
    QLineEdit* m_tickDelay{ nullptr };
    QLineEdit* m_timeDelay{ nullptr };

    GModifyAnimationClipMessage m_modifyClipMessage;
    GRequestAnimationDataMessage m_requestDataMessage;

    /// @}

};


} // End namespace rev

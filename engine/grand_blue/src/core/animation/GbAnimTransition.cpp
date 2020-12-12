#include "GbAnimTransition.h"
#include "GbAnimMotion.h"
#include "GbBlendQueue.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"
#include "../resource/GbResource.h"

#include "../geometry/GbMatrix.h"
#include "../geometry/GbTransform.h"
#include "../utils/GbInterpolation.h"
#include "../utils/GbParallelization.h"
#include "../processes/GbAnimationProcess.h"
#include "../processes/GbProcessManager.h"

#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"


namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// TransitionSettings
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue TransitionSettings::asJson() const
{
    QJsonObject object;
    object.insert("transitionType", (int)m_transitionType);
    object.insert("fadeInTime", m_fadeInTimeSec);
    object.insert("fadeInWeight", m_fadeInBlendWeight);
    object.insert("fadeOutTime", m_fadeOutTimeSec);
    object.insert("fadeOutWeight", m_fadeOutBlendWeight);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void TransitionSettings::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    Q_UNUSED(context);
    QJsonObject object = json.toObject();
    m_transitionType = (AnimationTransitionType)object["transitionType"].toInt();
    m_fadeInTimeSec = object["fadeInTime"].toDouble();
    m_fadeInBlendWeight = object["fadeInWeight"].toDouble();
    m_fadeOutTimeSec = object["fadeOutTime"].toDouble();
    m_fadeOutBlendWeight = object["fadeOutWeight"].toDouble();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationTransition
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition::AnimationTransition(AnimationStateMachine* stateMachine, AnimationState* start, AnimationState* end):
    BaseAnimationState(stateMachine),
    m_start(start),
    m_end(end)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition::~AnimationTransition()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::getClips(const Motion& motion, std::vector<AnimationPlayData>& outData, std::vector<float>& outWeights) const
{
    float blendWeight;
    for (const AnimationClip& clip : m_start->clips()) {
        // Clips that are fading out
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeOutBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            m_start->playbackMode(),
            motion.timer(),
            (size_t)AnimPlayStatusFlag::kFadingOut
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec};
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
    for (const AnimationClip& clip : m_end->clips()) {
        // Clips that are fading in
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeInBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            m_start->playbackMode(),
            motion.timer(),
            (size_t)AnimPlayStatusFlag::kFadingIn
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec };
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::onEntry(Motion& motion) const
{
    // Restart the timer
    m_timer.restart();
    
    // Don't need to cache, since not restarting motion's timer
    //// Cache timer from motion's current playing state
    //m_previousTimer = motion.timer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::onExit(Motion& motion) const
{
    // Start motion timer to match transition's
    motion.setTimer(m_timer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationTransition::asJson() const
{
    QJsonObject object = BaseAnimationState::asJson().toObject();
    object.insert("settings", m_settings.asJson());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    BaseAnimationState::loadFromJson(json, context);
    QJsonObject object = json.toObject();
    m_settings.loadFromJson(object["settings"]);
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

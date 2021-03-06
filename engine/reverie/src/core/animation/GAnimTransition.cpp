#include "GAnimTransition.h"
#include "GAnimMotion.h"
#include "GAnimStateMachine.h"
#include "GBlendQueue.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"
#include "../resource/GResource.h"

#include "../geometry/GMatrix.h"
#include "../geometry/GTransform.h"
#include "../utils/GInterpolation.h"
#include "../threading/GParallelLoop.h"
#include "../processes/GAnimationProcess.h"
#include "../processes/GProcessManager.h"

#include "../rendering/geometry/GMesh.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../rendering/models/GModel.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/renderer/GRenderCommand.h"


namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// TransitionSettings
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue TransitionSettings::asJson(const SerializationContext& context) const
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
AnimationTransition::AnimationTransition(AnimationStateMachine * stateMachine):
    BaseAnimationState(stateMachine)
{
    m_playbackMode = AnimationPlaybackMode::kSingleShot;
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition::AnimationTransition(AnimationStateMachine* stateMachine, int connectionIndex):
    BaseAnimationState(stateMachine)
{
    m_connections.push_back(connectionIndex);
    m_playbackMode = AnimationPlaybackMode::kSingleShot;
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationTransition::~AnimationTransition()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::getClips(const Motion& motion, std::vector<AnimationPlayData>& outData, std::vector<float>& outWeights) const
{
    float blendWeight;
    const AnimationState* start = AnimationTransition::start();
    const AnimationState* end = AnimationTransition::end();
    for (const AnimationClip& clip : start->clips()) {
        // Clips that are fading out
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeOutBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            start->playbackMode(),
            motion.timer(),
            (size_t)AnimPlayStatusFlag::kFadingOut
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec};
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
    for (const AnimationClip& clip : end->clips()) {
        // Clips that are fading in
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeInBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            end->playbackMode(),
            motion.timer(),
            (size_t)AnimPlayStatusFlag::kFadingIn
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec };
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
const AnimationState * AnimationTransition::start() const
{
    return dynamic_cast<AnimationState*>(connection().start(m_stateMachine));
}
/////////////////////////////////////////////////////////////////////////////////////////////
const AnimationState * AnimationTransition::end() const
{
    return dynamic_cast<AnimationState*>(connection().end(m_stateMachine));
}
/////////////////////////////////////////////////////////////////////////////////////////////
StateConnection & AnimationTransition::connection() const
{
    if (m_connections.empty()) {
        throw("Error, no connection set");
    }
    return m_stateMachine->getConnection(m_connections[0]);
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
QJsonValue AnimationTransition::asJson(const SerializationContext& context) const
{
    QJsonObject object = BaseAnimationState::asJson(context).toObject();

    // Save name to Json
    //object.insert("name", m_name.c_str());

    //object.insert("name", m_name.c_str());
    object.insert("settings", m_settings.asJson());
    object.insert("start", start()->getName().c_str());
    object.insert("end", end()->getName().c_str());
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTransition::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    BaseAnimationState::loadFromJson(json, context);
    
    QJsonObject object = json.toObject();
    if (object.contains("name")) {
        setName(object["name"].toString());
    }
    
    AnimationState* start = dynamic_cast<AnimationState*>(m_stateMachine->getState(object["start"].toString()));
    AnimationState* end = dynamic_cast<AnimationState*>(m_stateMachine->getState(object["end"].toString()));
    int connectionIndex;
    bool connects = start->connectsTo(end, m_stateMachine, &connectionIndex);
    if (!connects) {
        throw("Error, connection not found between transition states");
    }

    m_connections.push_back(connectionIndex);
    m_settings.loadFromJson(object["settings"]);
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

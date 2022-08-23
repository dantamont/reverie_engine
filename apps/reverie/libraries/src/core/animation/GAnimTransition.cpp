#include "core/animation/GAnimTransition.h"
#include "core/animation/GAnimMotion.h"
#include "core/animation/GAnimStateMachine.h"
#include "core/animation/GBlendQueue.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/resource/GResource.h"

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/math/GInterpolation.h"
#include "fortress/thread/GParallelLoop.h"
#include "core/processes/GAnimationProcess.h"
#include "core/processes/GProcessManager.h"

#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GSkeleton.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderCommand.h"


namespace rev {


// TransitionSettings

void to_json(json& orJson, const TransitionSettings& korObject)
{
    orJson["transitionType"] = (int)korObject.m_transitionType;
    orJson["fadeInTime"] = korObject.m_fadeInTimeSec;
    orJson["fadeInWeight"] = korObject.m_fadeInBlendWeight;
    orJson["fadeOutTime"] = korObject.m_fadeOutTimeSec;
    orJson["fadeOutWeight"] = korObject.m_fadeOutBlendWeight;
}

void from_json(const json& korJson, TransitionSettings& orObject)
{
    orObject.m_transitionType = (AnimationTransitionType)korJson["transitionType"].get<Int32_t>();
    korJson["fadeInTime"].get_to(orObject.m_fadeInTimeSec);
    korJson["fadeInWeight"].get_to(orObject.m_fadeInBlendWeight);
    korJson["fadeOutTime"].get_to(orObject.m_fadeOutTimeSec);
    korJson["fadeOutWeight"].get_to(orObject.m_fadeOutBlendWeight);
}





// AnimationTransition

AnimationTransition::AnimationTransition(AnimationStateMachine * stateMachine):
    BaseAnimationState(stateMachine)
{
    m_playbackMode = AnimationPlaybackMode::kSingleShot;
}

AnimationTransition::AnimationTransition(AnimationStateMachine* stateMachine, int connectionIndex):
    BaseAnimationState(stateMachine)
{
    m_connections.push_back(connectionIndex);
    m_playbackMode = AnimationPlaybackMode::kSingleShot;
}

AnimationTransition::~AnimationTransition()
{
}

void AnimationTransition::getClips(const Motion& motion, std::vector<AnimationPlayData>& outData, std::vector<float>& outWeights, ResourceCache* cache) const
{
    float blendWeight;
    const AnimationState* start = AnimationTransition::start();
    const AnimationState* end = AnimationTransition::end();
    for (const AnimationClip& clip : start->clips()) {
        // Check if clip has animation handle loaded yet
        // If no animation handle, was not created at time of AnimationClip::loadFromJson, so try to load again
        bool hasHandle = clip.verifyLoaded(cache);

        if (!hasHandle) {
            // Skip if handle not yet loaded
            continue;
        }

        // Clips that are fading out
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeOutBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            start->playbackMode(),
            motion.timer(),
            (uint32_t)AnimPlayStatusFlag::kFadingOut
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec};
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
    for (const AnimationClip& clip : end->clips()) {
        // Check if clip has animation handle loaded yet
        // If no animation handle, was not created at time of AnimationClip::loadFromJson, so try to load again
        bool hasHandle = clip.verifyLoaded(cache);

        if (!hasHandle) {
            // Skip if handle not yet loaded
            continue;
        }

        // Clips that are fading in
        blendWeight = clip.settings().m_blendWeight;
        blendWeight *= m_settings.m_fadeInBlendWeight;
        Vec::EmplaceBack(outData,
            clip.animationHandle(),
            clip.settings(),
            end->playbackMode(),
            motion.timer(),
            (uint32_t)AnimPlayStatusFlag::kFadingIn
        );
        outData.back().m_transitionData = { transitionTime(), m_settings.m_fadeInTimeSec, m_settings.m_fadeOutTimeSec };
        outData.back().m_transitionTimer = m_timer;
        outWeights.push_back(blendWeight);
    }
}

const AnimationState * AnimationTransition::start() const
{
    return dynamic_cast<AnimationState*>(connection().start(m_stateMachine));
}

const AnimationState * AnimationTransition::end() const
{
    return dynamic_cast<AnimationState*>(connection().end(m_stateMachine));
}

StateConnection & AnimationTransition::connection() const
{
    if (m_connections.empty()) {
        Logger::Throw("Error, no connection set");
    }
    return m_stateMachine->getConnection(m_connections[0]);
}

void AnimationTransition::onEntry(Motion& motion) const
{
    // Restart the timer
    m_timer.restart();
    
    // Don't need to cache, since not restarting motion's timer
    //// Cache timer from motion's current playing state
    //m_previousTimer = motion.timer();
}

void AnimationTransition::onExit(Motion& motion) const
{
    // Start motion timer to match transition's
    motion.setTimer(m_timer);
}

void to_json(json& orJson, const AnimationTransition& korObject)
{
    ToJson<BaseAnimationState>(orJson, korObject);
    orJson["settings"] = korObject.m_settings;
    orJson["start"] = korObject.start()->getName().c_str();
    orJson["end"] = korObject.end()->getName().c_str();
}

void from_json(const json& korJson, AnimationTransition& orObject)
{
    FromJson<BaseAnimationState>(korJson, orObject);
    
    if (korJson.contains(JsonKeys::s_name)) {
        orObject.setName(korJson[JsonKeys::s_name].get_ref<const std::string&>().c_str());
    }
    
    GString startStr = korJson["start"].get_ref<const std::string&>().c_str();
    GString endStr = korJson["end"].get_ref<const std::string&>().c_str();
    AnimationState* start = dynamic_cast<AnimationState*>(orObject.m_stateMachine->getState(startStr));
    AnimationState* end = dynamic_cast<AnimationState*>(orObject.m_stateMachine->getState(endStr));
    int connectionIndex;
    bool connects = start->connectsTo(end, orObject.m_stateMachine, &connectionIndex);
    if (!connects) {
        Logger::Throw("Error, connection not found between transition states");
    }

    orObject.m_connections.push_back(connectionIndex);
    korJson["settings"].get_to(orObject.m_settings);
}


} // End namespaces

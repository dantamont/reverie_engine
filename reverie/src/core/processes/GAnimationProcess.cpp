#include "GAnimationProcess.h"

#include "../GCoreEngine.h"
#include "../processes/GProcessManager.h"
#include "../events/GEventManager.h"
#include "../loop/GSimLoop.h"
#include "../rendering/geometry/GMesh.h"
#include "../rendering/models/GModel.h"
#include "../rendering/geometry/GSkeleton.h"
#include "../animation/GAnimationController.h"
#include "../geometry/GTransform.h"

#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::AnimationProcess::AnimationProcess(CoreEngine* engine,
    AnimationController* controller,
    const Transform* transform):
    Process(engine, 
        engine->processManager()->sortingLayers().getLayer(DEFAULT_SORTING_LAYER),
        engine->processManager()),
    m_controller(controller),
    m_transform(transform)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::AnimationProcess::~AnimationProcess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationProcess::onInit()
{
    // Check that all required resources are in itialized
    if (!m_controller->motions().size()) return;
    if (!m_controller->getModel()) return;
    if (!m_controller->getModel()->skeleton()) return;
    if (m_controller->getModel()->skeleton()->handle()->isLoading()) return;

    //static std::mutex SkeletonMutex;
    //std::unique_lock lock(SkeletonMutex);
    m_controller->getModel()->skeleton()->identityPose(m_transforms);
    Process::onInit();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void rev::AnimationProcess::onUpdate(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationProcess::onFixedUpdate(unsigned long deltaMs)
{
    // FIXME: Loop breaks on scenario reload, need to restart thread nicely
    // Lock mutex
    std::unique_lock lock(m_mutex);

    // Don't need to check for resources, since won't be initialized until everything is hunky dory
    if (!m_controller->motions().size()) return;

    // Get animation time and corresponding pose
    m_elapsedTimeSec += float(deltaMs) / 1000.0;

    // Check if any animations finished, and update motions accordingly -----------------
    m_controller->updateMotions();

    // Check if clips need to be updated ------------------------------------------------
    // TODO: Only do this when motions are updated in the state machine
    Skeleton* skeleton = m_controller->getModel()->skeleton();
    m_controller->blendQueue().updateCurrentClips(m_elapsedTimeSec, skeleton);

    // Do this every frame, for transitions
    m_controller->blendQueue().updateClipWeights();

    // Check if in view -----------------------------------------------------------------
    const SceneObject& so = *m_controller->sceneObject();

    // Return if no bounding boxes (scene object is uninitialized for blueprints)
    const std::vector<AABB>& bounds = so.worldBounds().geometry();
    if (!bounds.size()) {
        return;
    }

    const AABB& worldboundingBox = bounds.back(); // FIXME: Might be threading issue with accessing geometry
    bool inView = m_engine->scenario()->isVisible(worldboundingBox);
    if (!inView) { 
        // Return if not in view
        //Logger::LogInfo("Skeleton not in view");
        return; 
    }

    // Blend animations for rendering if in view ----------------------------------------
    // Obtain current animation frame
    m_controller->blendQueue().updateAnimationFrame(
        m_elapsedTimeSec,
        skeleton,
        m_transforms);

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void rev::AnimationProcess::onSuccess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationProcess::onFail()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationProcess::onAbort()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
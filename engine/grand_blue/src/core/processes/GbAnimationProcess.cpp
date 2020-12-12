#include "GbAnimationProcess.h"

#include "../GbCoreEngine.h"
#include "../processes/GbProcessManager.h"
#include "../events/GbEventManager.h"
#include "../loop/GbSimLoop.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../animation/GbAnimationController.h"
#include "../geometry/GbTransform.h"

#include "../scene/GbScenario.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::AnimationProcess::AnimationProcess(CoreEngine* engine,
    AnimationController* controller,
    const Transform* transform):
    Process(engine, 
        engine->processManager()->getSortingLayer(DEFAULT_SORTING_LAYER),
        engine->processManager()),
    m_controller(controller),
    m_transform(transform)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::AnimationProcess::~AnimationProcess()
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
void Gb::AnimationProcess::onUpdate(unsigned long deltaMs)
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
    Skeleton* skeleton = m_controller->getModel()->skeleton().get();
    m_controller->blendQueue().updateCurrentClips(m_elapsedTimeSec, skeleton);

    // Do this every frame, for transitions
    m_controller->blendQueue().updateClipWeights();

    // Check if in view -----------------------------------------------------------------
    // TODO: Use component's updateBounds method, since bounds are already calculated
    // FIXME: Same as TODO
    const AABB& objectBoundingBox = skeleton->boundingBox();
    AABB worldBoundingBox;
    objectBoundingBox.recalculateBounds(m_transform->worldMatrix(), worldBoundingBox);
    bool inView = m_engine->scenario()->isVisible(worldBoundingBox);
    if (!inView) { 
        // Return if not in view
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
void Gb::AnimationProcess::onSuccess()
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
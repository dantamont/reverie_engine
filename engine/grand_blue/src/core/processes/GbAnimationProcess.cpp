#include "GbAnimationProcess.h"

#include "../GbCoreEngine.h"
#include "../processes/GbProcessManager.h"
#include "../events/GbEventManager.h"
#include "../loop/GbSimLoop.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/models/GbModel.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../animation/GbAnimation.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::AnimationProcess::AnimationProcess(CoreEngine* engine,
    AnimationController* controller,
    ProcessManager* manager):
    Process(engine, 
        engine->processManager()->sortingLayers().at("default"),
        manager),
    m_controller(controller),
    m_pose(std::make_shared<SkeletonPose>())
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::AnimationProcess::~AnimationProcess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationProcess::onInit()
{
    if (!m_controller->m_currentState) return;
    if (!m_controller->getModel()) return;
    if (!m_controller->getModel()->skeleton()) return;
    if (m_controller->getModel()->skeleton()->handle()->isLoading()) return;

    m_transforms = m_controller->getModel()->skeleton()->defaultPose();
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
    Q_UNUSED(deltaMs)

    if (!m_controller->m_currentState) return;
    if (!m_controller->getModel()) return;
    if (!m_controller->isPlaying()) return;
    if (m_controller->getModel()->handle()->isLoading()) {
        return;
    }

    // Get animation time and corresponding pose
    float timeElapsed = float(m_engine->simulationLoop()->elapsedTime()) / 1000.0;
    bool done;
    m_controller->m_currentState->getAnimationFrame(
        m_controller->getModel().get(),
        timeElapsed,
        done,
        *m_pose);
    m_pose->getBoneTransforms(m_transforms, true); // force blend
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
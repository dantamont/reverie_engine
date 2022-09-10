#include "core/processes/GSpriteAnimationProcess.h"

#include "core/GCoreEngine.h"
#include "core/processes/GProcessManager.h"
#include "core/events/GEventManager.h"
#include "core/loop/GSimLoop.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/models/GModel.h"
#include <core/canvas/GSprite.h>
#include <core/components/GCanvasComponent.h>
#include "heave/kinematics/GTransform.h"

#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

namespace rev {

SpriteAnimationProcess::SpriteAnimationProcess(CoreEngine* engine,
    Sprite* sprite):
    Process(
        engine->processManager()->animationThread().sortingLayers().getLayer(SortingLayer::s_defaultSortingLayer).id(),
        &engine->processManager()->animationThread()),
    m_engine(engine),
    m_sprite(sprite)
{
}

SpriteAnimationProcess::~SpriteAnimationProcess()
{
}

void SpriteAnimationProcess::onInit()
{
    // Check that all required resources are in itialized
    if (!m_sprite->materialHandle()){
        return;
    }
    if (m_sprite->materialHandle()->isLoading()) {
        return;
    }
    Process::onInit();
}

void SpriteAnimationProcess::onUpdate(double deltaSec)
{
    Q_UNUSED(deltaSec);
}

void SpriteAnimationProcess::onFixedUpdate(double deltaSec)
{
    // Lock mutex
    std::unique_lock lock(m_mutex);

    // Get total elapsed time
    m_elapsedTimeSec += deltaSec;

    // Update the animation index
    m_sprite->m_animation.updateFrame(m_sprite, m_elapsedTimeSec);
}

void SpriteAnimationProcess::onSuccess()
{
}

void SpriteAnimationProcess::onFail()
{
}

void SpriteAnimationProcess::onAbort()
{
}



}
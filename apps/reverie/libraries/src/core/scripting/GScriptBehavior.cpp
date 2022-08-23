#include "core/scripting/GScriptBehavior.h"

#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"
#include "core/components/GTransformComponent.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"

namespace rev {


ScriptBehavior::ScriptBehavior(SceneObject& sceneObject):
    m_sceneObject(&sceneObject)
{
}

ScriptBehavior::~ScriptBehavior()
{
}

InputHandler * ScriptBehavior::inputHandler()
{
    return &static_cast<InputHandler&>(engine()->widgetManager()->mainGLWidget()->inputHandler());
}

CoreEngine * ScriptBehavior::engine()
{
    return m_sceneObject->scene()->engine();
}

ResourceCache * ScriptBehavior::resourceCache()
{
    return &ResourceCache::Instance();
}

const TransformComponent * ScriptBehavior::transform()
{
    return &m_sceneObject->transform();
}

Scene * ScriptBehavior::scene()
{
    return m_sceneObject->scene();
}

void ScriptBehavior::initialize()
{
}

void ScriptBehavior::update(double deltaSec)
{
    Q_UNUSED(deltaSec);
//#ifdef DEBUG_MODE
//    logInfo("Updating a script behavior");
//#endif
}

void ScriptBehavior::lateUpdate(double deltaSec)
{
    Q_UNUSED(deltaSec);
//#ifdef DEBUG_MODE
//    logInfo("Late updating a script behavior");
//#endif
}

void ScriptBehavior::fixedUpdate(double deltaSec)
{
    Q_UNUSED(deltaSec);
}

void ScriptBehavior::onSuccess()
{
}

void ScriptBehavior::onFail()
{
}

void ScriptBehavior::onAbort()
{
}

bool ScriptBehavior::s_logMessages = true;




}
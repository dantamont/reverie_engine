#include "GScriptBehavior.h"

#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"
#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"
#include "../input/GInputHandler.h"
#include "../components/GTransformComponent.h"

#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
ScriptBehavior::ScriptBehavior(SceneObject& sceneObject):
    m_sceneObject(&sceneObject)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
ScriptBehavior::~ScriptBehavior()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
InputHandler * ScriptBehavior::inputHandler()
{
    return &(engine()->widgetManager()->mainGLWidget()->inputHandler());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
CoreEngine * ScriptBehavior::engine()
{
    return m_sceneObject->scene()->engine();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
ResourceCache * ScriptBehavior::resourceCache()
{
    return engine()->resourceCache();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent * ScriptBehavior::transform()
{
    return &m_sceneObject->transform();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Scene * ScriptBehavior::scene()
{
    return m_sceneObject->scene();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::initialize()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::update(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
//#ifdef DEBUG_MODE
//    logInfo("Updating a script behavior");
//#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::lateUpdate(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
//#ifdef DEBUG_MODE
//    logInfo("Late updating a script behavior");
//#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::fixedUpdate(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::onSuccess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::onFail()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptBehavior::onAbort()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScriptBehavior::s_logMessages = true;



//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
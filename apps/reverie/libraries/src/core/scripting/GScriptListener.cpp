#include "core/scripting/GScriptListener.h"

#include "core/events/GEvent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/components/GTransformComponent.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"

namespace rev {


ScriptListener::ScriptListener(SceneObject& sceneObject):
    m_sceneObject(&sceneObject)
{
}

ScriptListener::~ScriptListener()
{
}

InputHandler * ScriptListener::inputHandler()
{
    return &static_cast<InputHandler&>(engine()->widgetManager()->mainGLWidget()->inputHandler());
}

CoreEngine * ScriptListener::engine()
{
    return m_sceneObject->scene()->engine();
}

ResourceCache * ScriptListener::resourceCache()
{
    return &ResourceCache::Instance();
}

TransformComponent * ScriptListener::transform()
{
    return &m_sceneObject->transform();
}

Scene * ScriptListener::scene()
{
    return m_sceneObject->scene();
}

bool ScriptListener::eventTest(const CustomEvent& ev)
{
    Q_UNUSED(ev);
    return true;
}

void ScriptListener::perform(const CustomEvent& ev)
{
    Q_UNUSED(ev);
    Logger::LogInfo("Performing event EVENT EVENT");
}

bool ScriptListener::s_logMessages = true;




}
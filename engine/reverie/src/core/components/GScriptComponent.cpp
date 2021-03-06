#include "GScriptComponent.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"

#include "../loop/GSimLoop.h"
#include "../processes/GProcessManager.h"
#include "../scripting/GPythonScript.h"
#include "../processes/GScriptedProcess.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include <chrono>

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ScriptComponent::ScriptComponent(const std::shared_ptr<SceneObject>& object):
    Component(object, ComponentType::kPythonScript)
{
    //setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ScriptComponent::~ScriptComponent()
{
    if (!m_scriptProcess->isAborted()) {
        // Will already be aborted of this is done on a new scenario load, but not if
        // just deleting the script component directly
        m_scriptProcess->abort();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::reset()
{
    // Pause  process
    bool wasPaused = m_scriptProcess->isPaused();
    if (!wasPaused) {
        m_scriptProcess->pause();
    }

    // Reload script
    m_script->reload();
    m_scriptProcess->refresh();

    // Unpause process
    if (!wasPaused) {
        m_scriptProcess->unPause();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::initializeBehavior(const QString& filepath)
{
    // Return if no filepath
    QFile file(filepath);
    if (!file.exists()) {
#ifdef DEBUG_MODE
        logWarning("Warning, filepath to script not found:" + filepath);
#endif
        return;
    }

    // Set script for this component
    // Note: Adds script to the scenario (and python) if not present
    ResourceCache* cache = sceneObject()->scene()->engine()->resourceCache();
    auto scriptHandle = cache->guaranteeHandleWithPath(m_path, ResourceType::kPythonScript);
    m_path = filepath;
    m_script = scriptHandle->resourceAs<PythonClassScript>();
    if (!m_script) {
        if (scriptHandle->isLoading()) {
            throw("Error, script still loading");
        }
        else {
            throw("Script not found and not loading");
        }
    }

    // Start the process for this script component
    m_scriptProcess = std::make_shared<ScriptedProcess>(
        sceneObject()->scene()->engine(),
        this,
        sceneObject()->scene()->engine()->simulationLoop()->processManager().get());
    sceneObject()->scene()->engine()->simulationLoop()->processManager()->attachProcess(
        m_scriptProcess);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::enable()
{
    //if (m_scriptProcess) {
    //    m_scriptProcess->unPause();
    //}
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::disable()
{
    //if (m_scriptProcess) {
    //    m_scriptProcess->pause();
    //}
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue ScriptComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    object.insert("path", m_path);
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Component::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();
    if (object.contains("path")) {
        m_path = object.value("path").toString();
    }
    if (m_path != "") {
        initializeBehavior(m_path);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
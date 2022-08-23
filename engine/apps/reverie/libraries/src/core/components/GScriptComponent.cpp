#include "core/components/GScriptComponent.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"

#include "core/loop/GSimLoop.h"
#include "core/processes/GProcessManager.h"
#include "core/scripting/GPythonScript.h"
#include "core/processes/GScriptedProcess.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include <chrono>

namespace rev {

rev::ScriptComponent::ScriptComponent(const std::shared_ptr<SceneObject>& object):
    Component(object, ComponentType::kPythonScript)
{
    //setSceneObject(sceneObject());
    sceneObject()->setComponent(this);
}

rev::ScriptComponent::~ScriptComponent()
{
    if (!m_scriptProcess->isAborted()) {
        // Will already be aborted of this is done on a new scenario load, but not if
        // just deleting the script component directly
        m_scriptProcess->abort();
    }
}

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

void ScriptComponent::initializeBehavior(const QString& filepath)
{
    // Return if no filepath
    QFile file(filepath);
    if (!file.exists()) {
#ifdef DEBUG_MODE
        Logger::LogWarning("Warning, filepath to script not found:" + filepath.toStdString());
#endif
        return;
    }

    // Set script for this component
    // Note: Adds script to the scenario (and python) if not present
    ResourceCache* cache = &ResourceCache::Instance();
    auto scriptHandle = cache->guaranteeHandleWithPath(m_path.toStdString(), EResourceType::ePythonScript);
    m_path = filepath;
    m_script = scriptHandle->resourceAs<PythonClassScript>();
    if (!m_script) {
        if (scriptHandle->isLoading()) {
            Logger::Throw("Error, script still loading");
        }
        else {
            Logger::Throw("Script not found and not loading");
        }
    }

    // Start the process for this script component
    m_scriptProcess = std::make_shared<ScriptedProcess>(this,
        sceneObject()->scene()->engine()->processManager());
    sceneObject()->scene()->engine()->processManager()->processQueue().attachProcess(
        m_scriptProcess);

}

void ScriptComponent::enable()
{
    //if (m_scriptProcess) {
    //    m_scriptProcess->unPause();
    //}
    Component::enable();
}
 
void ScriptComponent::disable()
{
    //if (m_scriptProcess) {
    //    m_scriptProcess->pause();
    //}
    Component::disable();
}

void to_json(json& orJson, const ScriptComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["path"] = korObject.m_path.toStdString();
}

void from_json(const json& korJson, ScriptComponent& orObject)
{
    FromJson<Component>(korJson, orObject);

    if (korJson.contains("path")) {
        orObject.m_path = korJson.at("path").get_ref<const std::string&>().c_str();
    }
    if (orObject.m_path != "") {
        orObject.initializeBehavior(orObject.m_path);
    }
}


} // end namespacing
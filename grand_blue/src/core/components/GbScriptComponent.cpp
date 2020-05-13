#include "GbScriptComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResourceCache.h"

#include "../loop/GbSimLoop.h"
#include "../processes/GbProcessManager.h"
#include "../scripting/GbPythonScript.h"
#include "../processes/GbScriptedProcess.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ScriptComponent::ScriptComponent(const std::shared_ptr<SceneObject>& object):
    Component(object, kPythonScript)
{
    setSceneObject(sceneObject());
    sceneObject()->addComponent(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ScriptComponent::~ScriptComponent()
{
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
    // If no base behavior defined, checkValidity it
    QString baseScriptPath;
#ifndef DEBUG_MODE
    baseScriptPath = ":scripts/base_behavior.py"
#else
    baseScriptPath = QFileInfo(QFile("py_scripts:base_behavior.py")).absoluteFilePath();
#endif
    m_engine->resourceCache()->getScript(baseScriptPath);

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
    m_path = filepath;
    m_script = m_engine->resourceCache()->getScript(filepath);

    // Start the process for this script component
    m_scriptProcess = std::make_shared<ScriptedProcess>(
        m_engine,
        this,
        m_engine->simulationLoop()->processManager().get());
    m_engine->simulationLoop()->processManager()->attachProcess(
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
QJsonValue ScriptComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    object.insert("path", m_path);
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptComponent::loadFromJson(const QJsonValue & json)
{
    Component::loadFromJson(json);
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
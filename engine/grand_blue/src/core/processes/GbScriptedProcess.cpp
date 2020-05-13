#include "GbScriptedProcess.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"
#include "../scripting/GbPythonAPI.h"
#include "../scripting/GbPythonScript.h"
#include "../scene/GbSceneObject.h"
#include "../components/GbScriptComponent.h"

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ScriptedProcess::ScriptedProcess(CoreEngine* engine,
    ScriptComponent* component,
    ProcessManager* manager):
    Process(engine, component->script()->sortingLayer(), manager),
    m_component(component),
    m_ranUpdate(false)
{
    m_behavior = m_component->script()->instantiate(m_component->sceneObject());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::ScriptedProcess::~ScriptedProcess()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::refresh()
{
    m_behavior = m_component->script()->instantiate(m_component->sceneObject());
    onInit();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::onInit()
{
    // TODO: Fix bug where processes are broken on scenario resize
    // to recreate, simply open a scenario twice that has the same process
    //m_behavior.call("checkValidity", QVariantList());
    //PythonAPI::get()->call(m_behavior.object(), "checkValidity");
    QVariant initOut = m_behavior.call("initialize");
    int succeeded = initOut.toInt();
    if (!succeeded) {
        QString stdErr = PythonAPI::get()->getStdErr();
        logError("onUpdate:: Failed to initialize behavior: " + stdErr);
        PythonAPI::get()->clearStdErr();
    }
    Process::onInit();
#ifdef DEBUG_MODE
    PythonAPI::get()->logStdOut();
    PythonAPI::get()->printAndClearErrors();
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void Gb::ScriptedProcess::onUpdate(unsigned long deltaMs)
{
    if (!componentIsActive()) return;

    if(!m_ranUpdate){
        // Run update
        QVariantList args = QVariantList() << unsigned int(deltaMs);
        QVariant upOut = m_behavior.call("update", args);
        int succeeded = upOut.toInt();
        if (!succeeded) {
            QString stdErr = PythonAPI::get()->getStdErr();
            logError("onUpdate:: Failed to update behavior: " + stdErr);
            PythonAPI::get()->clearStdErr();
        }
#ifdef DEBUG_MODE
        PythonAPI::get()->logStdOut();
#endif
        m_ranUpdate = true;
    }
    else {
        // Run late update
        QVariantList args = QVariantList() << unsigned int(deltaMs);
        QVariant upOut = m_behavior.call("late_update", args);
        int succeeded = upOut.toInt();
        if (!succeeded) {
            QString stdErr = PythonAPI::get()->getStdErr();
            logError("onLateUpdate:: Failed to update behavior: " + stdErr);
            PythonAPI::get()->clearStdErr();
        }
#ifdef DEBUG_MODE
        PythonAPI::get()->logStdOut();
#endif
        m_ranUpdate = false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::onFixedUpdate(unsigned long deltaMs)
{
    if (!componentIsActive()) return;

    QVariantList args = QVariantList() << unsigned int(deltaMs);
    QVariant upOut = m_behavior.call("fixed_update", args);
    Process::onFixedUpdate(deltaMs);
    int succeeded = upOut.toInt();
    if (!succeeded) {
        QString stdErr = PythonAPI::get()->getStdErr();
        logError("onFixedUpdate:: Failed to update behavior" + stdErr);
        PythonAPI::get()->clearStdErr();
    }
#ifdef DEBUG_MODE
    PythonAPI::get()->logStdOut();
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void Gb::ScriptedProcess::onSuccess()
{
    if (!componentIsActive()) return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::onFail()
{
    if (!componentIsActive()) return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::onAbort()
{
    // No guarantees that component is still in memory scope by abort time
    //if (!componentIsActive()) return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScriptedProcess::componentIsActive() const
{
    return m_component->isEnabled();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
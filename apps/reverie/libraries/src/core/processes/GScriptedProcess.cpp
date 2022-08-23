#include "core/processes/GScriptedProcess.h"

#include <core/processes/GProcessManager.h>
#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/scripting/GPythonAPI.h"
#include "core/scripting/GPythonScript.h"
#include "core/scene/GSceneObject.h"
#include "core/components/GScriptComponent.h"

namespace rev {

rev::ScriptedProcess::ScriptedProcess(ScriptComponent* component, ProcessManager* manager):
    Process(component->script()->sortingLayer(), &manager->processQueue()),
    m_component(component)
{
    m_behavior = m_component->script()->instantiate(m_component->sceneObject());
}

rev::ScriptedProcess::~ScriptedProcess()
{
}

void ScriptedProcess::refresh()
{
    m_behavior = m_component->script()->instantiate(m_component->sceneObject());
    onInit();
}

void ScriptedProcess::onInit()
{
    if (!componentIsActive()) return;

    /// \see https://pybind11.readthedocs.io/en/stable/advanced/pycpp/object.html#calling-python-functions
    // TODO: Fix bug where processes are broken on scenario resize
    // to recreate, simply open a scenario twice that has the same process
    //m_behavior.call("checkValidity", QVariantList());
    //PythonAPI::Instance().call(m_behavior.object(), "checkValidity");
    try {
        m_behavior.attr("initialize")();
    }
    catch(py::error_already_set& err){
        //QString stdErr = PythonAPI::Instance().getStdErr();
        GString oErr = err.what();
        GString className = m_component->script()->getClassName().toStdString();
        Logger::LogError(className + "onInit:: Failed to initialize behavior: " + oErr);
        //PythonAPI::Instance().clearStdErr();
    }
    Process::onInit();
#ifdef DEBUG_MODE
    //PythonAPI::Instance().logStdOut();
    //PythonAPI::Instance().printAndClearErrors();
#endif
}

void rev::ScriptedProcess::onUpdate(double deltaSec)
{
    if (!componentIsActive()) return;

    try {
        m_behavior.attr("update")(deltaSec);
    }
    catch (py::error_already_set& err) {
        GString className = m_component->script()->getClassName().toStdString();
        GString errG(err.what());
        Logger::LogError(className + ": onUpdate:: Failed to update behavior " + className + ": " + errG);
    }
}

void ScriptedProcess::onLateUpdate(double deltaSec)
{
    if (!componentIsActive()) return;

    try {
        m_behavior.attr("late_update")(deltaSec);
    }
    catch (py::error_already_set& err) {
        GString errG(err.what());
        GString className = m_component->script()->getClassName().toStdString();
        Logger::LogError(className + ": onLateUpdate:: Failed to late update behavior " + className + ": " + errG);
    }
}

void ScriptedProcess::onPostUpdate(double)
{
    /// \todo Implement on python side
    //if (!componentIsActive()) return;

    //try {
    //    m_behavior.attr("post_update")(deltaSec);
    //}
    //catch (py::error_already_set& err) {
    //    GString errG(err.what());
    //    GString className = m_component->script()->getClassName().toStdString();
    //    Logger::LogError(className + ": onPostUpdate:: Failed to post update behavior " + className + ": " + errG);
    //}
}

void ScriptedProcess::onFixedUpdate(double deltaSec)
{
    if (!componentIsActive()) { return; }
    Process::onFixedUpdate(deltaSec);

    try {
        m_behavior.attr("fixed_update")(deltaSec);
    }
    catch (py::error_already_set& err) {
        GString errG(err.what());
        GString className = m_component->script()->getClassName().toStdString();
        Logger::LogError(className + ": onFixedUpdate:: Failed to update behavior" + errG);
    }
}

void rev::ScriptedProcess::onSuccess()
{
    if (!componentIsActive()) return;
}

void ScriptedProcess::onFail()
{
    if (!componentIsActive()) return;
}

void ScriptedProcess::onAbort()
{
    // No guarantees that component is still in memory scope by abort time
    //if (!componentIsActive()) return;
}

bool ScriptedProcess::componentIsActive() const
{
    return m_component->isEnabled();
}



}
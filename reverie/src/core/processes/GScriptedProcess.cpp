#include "GScriptedProcess.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include "../scripting/GPythonAPI.h"
#include "../scripting/GPythonScript.h"
#include "../scene/GSceneObject.h"
#include "../components/GScriptComponent.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ScriptedProcess::ScriptedProcess(CoreEngine* engine,
    ScriptComponent* component,
    ProcessManager* manager):
    Process(engine, component->script()->sortingLayer(), manager),
    m_component(component),
    m_ranUpdate(false)
{
    m_behavior = m_component->script()->instantiate(m_component->sceneObject());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ScriptedProcess::~ScriptedProcess()
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
    if (!componentIsActive()) return;

    // See: https://pybind11.readthedocs.io/en/stable/advanced/pycpp/object.html#calling-python-functions
    // TODO: Fix bug where processes are broken on scenario resize
    // to recreate, simply open a scenario twice that has the same process
    //m_behavior.call("checkValidity", QVariantList());
    //PythonAPI::get()->call(m_behavior.object(), "checkValidity");
    try {
        m_behavior.attr("initialize")();
    }
    catch(py::error_already_set& err){
        //QString stdErr = PythonAPI::get()->getStdErr();
        Logger::LogError(GString(m_component->script()->getClassName() + "onInit:: Failed to initialize behavior: ") + err.what());
        //PythonAPI::get()->clearStdErr();
    }
    Process::onInit();
#ifdef DEBUG_MODE
    //PythonAPI::get()->logStdOut();
    //PythonAPI::get()->printAndClearErrors();
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void rev::ScriptedProcess::onUpdate(unsigned long deltaMs)
{
    if (!componentIsActive()) return;

    if(!m_ranUpdate){
        try {
            m_behavior.attr("update")(unsigned int(deltaMs));
        }
        catch (py::error_already_set& err) {
            const GString& className = m_component->script()->getClassName();
            GString errG(err.what());
            Logger::LogError(GString(m_component->script()->getClassName()) + ": onUpdate:: Failed to update behavior " + GString(className) + ": " + errG);
        }
        m_ranUpdate = true;
    }
    else {
        try {
            m_behavior.attr("late_update")(unsigned int(deltaMs));
        }
        catch (py::error_already_set& err) {
            GString errG(err.what());
            const QString& className = m_component->script()->getClassName();
            Logger::LogError(GString(m_component->script()->getClassName()) + ": onLateUpdate:: Failed to late update behavior " + GString(className) + ": " + errG);
        }
        m_ranUpdate = false;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ScriptedProcess::onFixedUpdate(unsigned long deltaMs)
{
    if (!componentIsActive()) return;
    Process::onFixedUpdate(deltaMs);

    try {
        m_behavior.attr("fixed_update")(unsigned int(deltaMs));// .cast<int>();
    }
    catch (py::error_already_set& err) {
        GString errG(err.what());
        Logger::LogError(GString(m_component->script()->getClassName()) + ": onFixedUpdate:: Failed to update behavior" + errG);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void rev::ScriptedProcess::onSuccess()
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
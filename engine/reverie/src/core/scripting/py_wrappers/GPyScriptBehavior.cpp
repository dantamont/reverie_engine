#include "GPyScriptBehavior.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../input/GInputHandler.h"
#include "../../components/GTransformComponent.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyScriptBehavior::PyInitialize(pybind11::module_ & m)
{
    // Initialize ScriptBehavior class, with trampoline helper
    py::class_<ScriptBehavior, PyScriptBehavior>(m, "ScriptBehavior", py::dynamic_attr())
        .def(py::init<SceneObject&>())
        .def("initialize", &ScriptBehavior::initialize)
        .def("update", &ScriptBehavior::update)
        .def("late_update", &ScriptBehavior::lateUpdate)
        .def("fixed_update", &ScriptBehavior::fixedUpdate)
        .def("on_success", &ScriptBehavior::onSuccess)
        .def("on_fail", &ScriptBehavior::onFail)
        .def("on_abort", &ScriptBehavior::onAbort)
        .def("log_info",
                [](const ScriptBehavior& sb, const char* str) {
            static Object printObj;
            if (sb.s_logMessages) {
                //py::print(str);
                printObj.logInfo(str);
            }
        }
        )
        .def_property("scene_object", &ScriptBehavior::sceneObject, &ScriptBehavior::setSceneObject)
        .def_property_readonly("scene", &ScriptBehavior::scene)
        .def_property_readonly("engine", &ScriptBehavior::engine)
        .def_property_readonly("input", &ScriptBehavior::inputHandler)
        .def_property_readonly("resources", &ScriptBehavior::resourceCache)
        .def_property_readonly("transform", &ScriptBehavior::transform);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
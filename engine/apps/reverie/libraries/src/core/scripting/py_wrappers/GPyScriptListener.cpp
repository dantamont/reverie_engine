#include "core/scripting/py_wrappers/GPyScriptListener.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/layer/view/widgets/graphics/GInputHandler.h"
#include "core/components/GTransformComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

namespace rev {


void PyScriptListener::PyInitialize(pybind11::module_ & m)
{
    // Initialize ScriptListener class, with trampoline helper
    py::class_<ScriptListener, PyScriptListener>(m, "ScriptListener", py::dynamic_attr())
        .def(py::init<SceneObject&>())
        .def("event_test", &ScriptListener::eventTest)
        .def("perform", &ScriptListener::perform)
        .def("log_info",
                [](const ScriptListener& sb, const char* str) {
            if (sb.s_logMessages) {
                //py::print(str);
                Logger::LogInfo(str);
            }
        }
        )
        .def_property("scene_object", &ScriptListener::sceneObject, &ScriptListener::setSceneObject)
        .def_property_readonly("scene", &ScriptListener::scene)
        .def_property_readonly("engine", &ScriptListener::engine)
        .def_property_readonly("input", &ScriptListener::inputHandler)
        .def_property_readonly("resources", &ScriptListener::resourceCache)
        .def_property_readonly("transform", &ScriptListener::transform);
}



}
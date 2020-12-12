#include "GbPyScriptListener.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../input/GbInputHandler.h"
#include "../../components/GbTransformComponent.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
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
                py::print(str);
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
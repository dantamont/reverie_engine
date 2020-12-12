#include "GbPyCamera.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyCamera::PyInitialize(py::module_ & m)
{
    // Initialize Scene class
    //py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
    //    .def("get_scene_object", [](Scene& s, const char* name) {return s.getSceneObjectByName(name); })
    //    .def_property("name",
    //        [](const Scene& s) {return s.getName().c_str(); },
    //        &Scene::setName)
    //    //.def("__repr__", [](const Scene& s) {return s.asQString().toStdString(); })
    //    ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
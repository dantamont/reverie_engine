#include "GPyResourceCache.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../rendering/shaders/GShaderProgram.h"


namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyResourceCache::PyInitialize(py::module_ & m)
{
    py::class_<ResourceCache>(m, "ResourceCache")
        .def("__repr__", [](const ResourceCache& r) {return r.asQString().toStdString(); })
        .def("get_shader_program", [](const ResourceCache& r, const char* name) {
        auto handle = r.getHandleWithName(name, ResourceType::kShaderProgram);
        if (!handle) {
            return static_cast<ShaderProgram*>(nullptr);
        }
        else {
            return handle->resourceAs<ShaderProgram>();
        }
    })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
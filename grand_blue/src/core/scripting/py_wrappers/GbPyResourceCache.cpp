#include "GbPyResourceCache.h"
#include "../../scene/GbScenario.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScene.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../rendering/shaders/GbShaders.h"


namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyResourceCache::PyInitialize(py::module_ & m)
{
    py::class_<ResourceCache>(m, "ResourceCache")
        .def("__repr__", [](const ResourceCache& r) {return r.asQString().toStdString(); })
        .def("get_shader_program", [](const ResourceCache& r, const char* name) {
        auto handle = r.getHandleWithName(name, Resource::kShaderProgram);
        if (!handle) {
            return static_cast<std::shared_ptr<ShaderProgram>>(nullptr);
        }
        else {
            return handle->resourceAs<ShaderProgram>();
        }
    })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
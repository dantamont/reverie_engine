#include "core/scripting/py_wrappers/GPyResourceCache.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"


namespace rev {


void PyResourceCache::PyInitialize(py::module_ & m)
{
    py::class_<ResourceCache>(m, "ResourceCache")
        .def("__repr__", 
            [](const ResourceCache& r) {
                json myJson;
                to_json(myJson, r);
                return myJson.dump();
            }
        )
        .def("get_shader_program", [](const ResourceCache& r, const char* name) {
        auto handle = r.getHandleWithName(name, EResourceType::eShaderProgram);
        if (!handle) {
            return static_cast<ShaderProgram*>(nullptr);
        }
        else {
            return handle->resourceAs<ShaderProgram>();
        }
    })
        ;
}


}
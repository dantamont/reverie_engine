#include "core/scripting/py_wrappers/GPyLight.h"
#include "core/scripting/GPythonAPI.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/GCoreEngine.h"
#include "core/components/GLightComponent.h"
#include "fortress/json/GJson.h"

namespace rev {


void PyLight::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<LightComponent, Component, std::unique_ptr<LightComponent, py::nodelete>>(m, "Light")
        .def(py::init<>(&PyComponent::Create<LightComponent>), py::return_value_policy::reference)
        .def("__str__", [](const LightComponent& l) {return GJson::ToString<std::string>(json{l}); })
        .def_property("diffuse_color", 
            [](const LightComponent& l) {    
                Color color(l.getDiffuseColor());
                return PythonAPI::Instance().toPyTuple(color.toStdVector<int>()); 
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::Instance().toVec<int>(color);
                Color c(vec);
                l.setDiffuseColor(c.toVector<Real_t, 4>());
            })
        .def_property("ambient_color",
            [](const LightComponent& l) {
                Color color(l.getAmbientColor());
                return PythonAPI::Instance().toPyTuple(color.toStdVector<int>());
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::Instance().toVec<int>(color);
                Color c(vec);
                l.setAmbientColor(c.toVector<Real_t, 4>());
            })
        .def_property("specular_color",
            [](const LightComponent& l) {
                Color color(l.getSpecularColor());
                return PythonAPI::Instance().toPyTuple(color.toStdVector<int>());
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::Instance().toVec<int>(color);
                Color c(vec);
                l.setSpecularColor(c.toVector<Real_t, 4>());
            })
        .def_property("intensity", &LightComponent::getIntensity,
            [](LightComponent& l, const py::object& intensity) {
                Real_t i = PythonAPI::Instance().cType<Real_t>(intensity.ptr());
                l.setIntensity(i);
            })
        .def_property("attenuations", &LightComponent::getAttenuations,
            [](LightComponent& l, const py::object& attenuation) {
                if (l.cachedLight().getType() == Light::kPoint) {
                    std::vector<float> vec = PythonAPI::Instance().toVec<Real_t>(attenuation);
                    l.setAttributes(Vector4(vec));
                }
            })
        .def_property("range", &LightComponent::getRange,
            [](LightComponent& l, const py::object& range) {
                Real_t i = PythonAPI::Instance().cType<Real_t>(range.ptr());
                l.setRange(i);
            })
        ;
}


}
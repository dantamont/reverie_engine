#include "GPyLight.h"
#include "../GPythonAPI.h"
#include "../../scene/GScenario.h"
#include "../../scene/GSceneObject.h"
#include "../../scene/GScene.h"
#include "../../GCoreEngine.h"
#include "../../components/GLightComponent.h"
#include "../../readers/GJsonReader.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyLight::PyInitialize(py::module_ & m)
{
    // Initialize Light class
    py::class_<LightComponent, Component, std::unique_ptr<LightComponent, py::nodelete>>(m, "Light")
        .def(py::init<>(&PyComponent::Create<LightComponent>), py::return_value_policy::reference)
        .def("__str__", [](const LightComponent& l) {return JsonReader::ToString<std::string>(l.asJson()); })
        .def_property("diffuse_color", 
            [](const LightComponent& l) {    
                Color color(l.getDiffuseColor());
                return PythonAPI::get()->toPyTuple(color.toVector4i()); 
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::get()->toVec<int>(color);
                Color c(vec);
                l.cachedLight().setDiffuseColor(c);
                l.updateLight(); // TODO, wrap this from light component
            })
        .def_property("ambient_color",
            [](const LightComponent& l) {
                Color color(l.getAmbientColor());
                return PythonAPI::get()->toPyTuple(color.toVector4i());
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::get()->toVec<int>(color);
                Color c(vec);
                l.cachedLight().setAmbientColor(c);
                l.updateLight(); // TODO, wrap this from light component
            })
        .def_property("specular_color",
            [](const LightComponent& l) {
                Color color(l.getSpecularColor());
                return PythonAPI::get()->toPyTuple(color.toVector4i());
            },
            [](LightComponent& l, const py::object& color) {
                std::vector<int> vec = PythonAPI::get()->toVec<int>(color);
                Color c(vec);
                l.cachedLight().setSpecularColor(c);
                l.updateLight(); // TODO, wrap this from light component
            })
        .def_property("intensity", &LightComponent::getIntensity,
            [](LightComponent& l, const py::object& intensity) {
                real_g i = PythonAPI::get()->cType<real_g>(intensity.ptr());
                l.cachedLight().setIntensity(i);
                l.updateLight(); // TODO, wrap this from light component
            })
        .def_property("attenuations", &LightComponent::getAttenuations,
            [](LightComponent& l, const py::object& attenuation) {
                if (l.cachedLight().getType() == Light::kPoint) {
                    std::vector<float> vec = PythonAPI::get()->toVec<real_g>(attenuation);
                    l.cachedLight().setAttributes(Vector3(vec));
                    l.updateLight(); // TODO, wrap this from light component
                }
            })
        .def_property("range", &LightComponent::getRange,
            [](LightComponent& l, const py::object& range) {
                real_g i = PythonAPI::get()->cType<real_g>(range.ptr());
                l.cachedLight().setRange(i);
                l.updateLight(); // TODO, wrap this from light component
            })
        ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
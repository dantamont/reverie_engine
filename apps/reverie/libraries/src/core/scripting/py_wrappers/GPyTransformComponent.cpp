#include "core/scripting/py_wrappers/GPyTransformComponent.h"
#include "core/components/GTransformComponent.h"
#include "fortress/json/GJson.h"
#include "core/scripting/GPythonAPI.h"

namespace rev {

void PyTransformComponent::PyInitialize(py::module_ & m)
{
    using namespace pybind11::literals;

    py::class_<RotationComponent<WorldMatrixVector>>(m, "Rotation")
        .def("__str__", [](const RotationComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        .def("__repr__", [](const RotationComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        .def_property_readonly("quaternion", &RotationComponent<WorldMatrixVector>::getQuaternion)
        //.def("add_rotation", &RotationComponent::addRotation, "eulerAngles"_a, "updateTransform"_a = true)
        //.def("set", py::overload_cast<const Quaternion&, bool>(&RotationComponent::setRotation),
        //    "quaternion"_a, "update_transform"_a = true)
        //.def("set_rotation", py::overload_cast<const Quaternion&, bool>(&RotationComponent::setRotation),
        //    "quaternion"_a, "update_transform"_a = true)
        //.def("set_rotation", py::overload_cast<const EulerAngles&, bool>(&RotationComponent::setRotation),
        //    "euler_angles"_a, "update_transform"_a = true)
        ;

    py::class_<ScaleComponent<WorldMatrixVector>>(m, "Scale")
        .def("__str__", [](const ScaleComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        .def("__repr__", [](const ScaleComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        .def_property_readonly("vec", &ScaleComponent<WorldMatrixVector>::getScale)
        .def_property_readonly("scale", &ScaleComponent<WorldMatrixVector>::getScale)
        //.def("set_scale", 
        //    [](ScaleComponent& s, const py::object& scale) {
        //        s.setScale(PythonAPI::Instance().toVec<Real_t>(scale));
        //    }
        //)
        //.def("set_scale", 
        //    [](ScaleComponent& s, double x, double y, double z) {
        //        s.setScale(x, y, z);
        //    }
        //)
        ;

    py::class_<TranslationComponent<WorldMatrixVector>>(m, "Translation")
        .def("__str__", [](const TranslationComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        .def("__repr__", [](const TranslationComponent<WorldMatrixVector>& t) { return GJson::ToString<std::string>(json{t}); })
        //.def_property("local_pos", &TranslationComponent::getPosition,
        //    [](TranslationComponent& t, const py::object& position) {
        //        return SetLocalPosition(t, position);
        //    }
        //    )
        ;

    py::class_<TransformComponent, Component, std::unique_ptr<TransformComponent, py::nodelete>>(m, "TransformComponent")
        .def("__str__", [](const TransformComponent& t) { return GJson::ToString<std::string>(json{t}); })
        .def("__repr__", [](const TransformComponent& t) { return GJson::ToString<std::string>(json{t}); })

        /* Scale */
        .def_property("scale", &TransformComponent::scale,
            [](TransformComponent& t, const Vector3& scale) {
                t.setScale(scale, true);
            }
        )
        .def("set_scale",
            [](TransformComponent& t, const py::object& scale) {
                t.setScale(PythonAPI::Instance().toVec<Real_t>(scale));
        }
        )
        .def("set_scale",
            [](TransformComponent& t, double x, double y, double z) {
                t.setScale(x, y, z);
            }
        )

        /* Translation */
        .def_property_readonly("translation", &TransformComponent::translation)
        .def_property("local_pos", &TransformComponent::getPosition,
            [](TransformComponent& t, const py::object& pos) {
                return SetLocalPosition(t, pos);
            }
        )
        .def_property("pos", 
            [](const TransformComponent& t) {return Vector3(t.worldMatrix().getColumn(3)); },
            &TransformComponent::setWorldPosition)

        /* Rotation
         TODO: Use rotation property for these, implementing polymorphism for the set method
         By having the body take a py::object and checking the type using pybind cast function */
        .def_property_readonly("rotation", &TransformComponent::rotation)
        .def("add_rotation", &TransformComponent::addRotation, "eulerAngles"_a, "updateTransform"_a = true)
        .def("set_rotation", py::overload_cast<const Quaternion&, bool>(&TransformComponent::setRotation),
            "quaternion"_a, "update_transform"_a = true)
        .def("set_rotation", py::overload_cast<const EulerAngles&, bool>(&TransformComponent::setRotation),
            "euler_angles"_a, "update_transform"_a = true)
        ;
}

void PyTransformComponent::SetLocalPosition(TransformComponent& transform, const py::object & position)
{
    static const py::module_& rev = PythonAPI::Instance().reverieModule();
    static py::object vecType = rev.attr("Vector3");
    if (PythonAPI::Instance().isListLike(position.ptr())) {
        std::vector<Real_t> pVec = PythonAPI::Instance().toVec<Real_t>(position);
        transform.setPosition(pVec);
    }
    else if (py::isinstance(position, vecType)) {
        transform.setPosition(position.cast<const Vector3&>());
    }
    else {
        GString className = PythonAPI::Instance().getClassName(position);
#ifdef DEBUG_MODE
        Logger::Throw("Error, position type not recognized");
#endif
        GString stdErr = PythonAPI::Instance().printAndClearErrors().toStdString();
        Logger::LogError("set_local_pos:: Error, position type " + className + " not recognized: " + stdErr);
    }
}




}
#include "GPyTransformComponent.h"
#include "../../components/GTransformComponent.h"
#include "../../readers/GJsonReader.h"
#include "../GPythonAPI.h"

namespace rev {
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyTransformComponent::PyInitialize(py::module_ & m)
{
    using namespace pybind11::literals;

    py::class_<RotationComponent>(m, "Rotation")
        .def("__str__", [](const RotationComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def("__repr__", [](const RotationComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def_property_readonly("quaternion", &RotationComponent::getQuaternion)
        //.def("add_rotation", &RotationComponent::addRotation, "eulerAngles"_a, "updateTransform"_a = true)
        //.def("set", py::overload_cast<const Quaternion&, bool>(&RotationComponent::setRotation),
        //    "quaternion"_a, "update_transform"_a = true)
        //.def("set_rotation", py::overload_cast<const Quaternion&, bool>(&RotationComponent::setRotation),
        //    "quaternion"_a, "update_transform"_a = true)
        //.def("set_rotation", py::overload_cast<const EulerAngles&, bool>(&RotationComponent::setRotation),
        //    "euler_angles"_a, "update_transform"_a = true)
        ;

    py::class_<ScaleComponent>(m, "Scale")
        .def("__str__", [](const ScaleComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def("__repr__", [](const ScaleComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def_property_readonly("vec", &ScaleComponent::getScale)
        .def_property_readonly("scale", &ScaleComponent::getScale)
        //.def("set_scale", 
        //    [](ScaleComponent& s, const py::object& scale) {
        //        s.setScale(PythonAPI::get()->toVec<real_g>(scale));
        //    }
        //)
        //.def("set_scale", 
        //    [](ScaleComponent& s, double x, double y, double z) {
        //        s.setScale(x, y, z);
        //    }
        //)
        ;

    py::class_<TranslationComponent>(m, "Translation")
        .def("__str__", [](const TranslationComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def("__repr__", [](const TranslationComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        //.def_property("local_pos", &TranslationComponent::getPosition,
        //    [](TranslationComponent& t, const py::object& position) {
        //        return SetLocalPosition(t, position);
        //    }
        //    )
        ;

    py::class_<TransformComponent, Component, std::unique_ptr<TransformComponent, py::nodelete>>(m, "TransformComponent")
        .def("__str__", [](const TransformComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })
        .def("__repr__", [](const TransformComponent& t) { return JsonReader::ToString<QString>(t.asJson()); })

        /* Scale */
        .def_property("scale", &TransformComponent::scale,
            [](TransformComponent& t, const Vector3& scale) {
                t.setScale(scale, true);
            }
        )
        .def("set_scale",
            [](TransformComponent& t, const py::object& scale) {
                t.setScale(PythonAPI::get()->toVec<real_g>(scale));
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void PyTransformComponent::SetLocalPosition(TransformComponent& transform, const py::object & position)
{
    static const py::module_& rev = PythonAPI::get()->reverieModule();
    static py::object vecType = rev.attr("Vector3");
    if (PythonAPI::get()->isListLike(position.ptr())) {
        std::vector<real_g> pVec = PythonAPI::get()->toVec<real_g>(position);
        transform.setPosition(pVec);
    }
    else if (py::isinstance(position, vecType)) {
        transform.setPosition(position.cast<const Vector3&>());
    }
    else {
        GString className = PythonAPI::get()->getClassName(position);
#ifdef DEBUG_MODE
        throw("Error, position type not recognized");
#endif
        QString stdErr = PythonAPI::get()->printAndClearErrors();
        Logger::LogError("set_local_pos:: Error, position type " + className + " not recognized: " + stdErr);
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
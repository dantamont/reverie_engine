#include "core/scripting/py_wrappers/GPyRotations.h"

#include "core/scripting/GPythonAPI.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/math/GQuaternion.h"
#include "fortress/json/GJson.h"
#include <pybind11/operators.h>

namespace rev {


void PyRotations::PyInitialize(py::module_ & m)
{
    // Euler angles
    py::class_<EulerAngles>(m, "EulerAngles")
        .def(py::init<>(&PyRotations::CreateEulerAngles))
        ;

    // Quaternions
    py::class_<Quaternion>(m, "Quaternion")
        .def(py::init<>())
        .def(py::init<double, double, double, double>())
        .def("__str__", [](const Quaternion& s) {return GJson::ToString<std::string>(json{s}); })
        .def("__repr__", [](const Quaternion& s) {return GJson::ToString<std::string>(json{s}); })
        .def_static("look_rotation", &Quaternion::fromDirection)
        .def_static("slerp", py::overload_cast<const Quaternion&, const Quaternion&, Real_t>(&Quaternion::Slerp))
        .def_static("from_axis_angle", py::overload_cast<const Vector3&, Real_t>(&Quaternion::fromAxisAngle))
        .def_static("from_euler_angles", [](double a1, double a2, double a3) {return Quaternion::fromEulerAngles(EulerAngles(a1, a2, a3, EulerAngles::kZYX, RotationSpace::kInertial)); })
        .def_static("from_euler_angles", py::overload_cast<const EulerAngles&>(&Quaternion::fromEulerAngles))
        .def_static("from_rotation_matrix", &Quaternion::fromRotationMatrix<double, 3>)
        .def_static("from_rotation_matrix", &Quaternion::fromRotationMatrix<double, 4>)
        .def_static("from_rotation_matrix", &Quaternion::fromRotationMatrix<float, 3>)
        .def_static("from_rotation_matrix", &Quaternion::fromRotationMatrix<float, 4>)
        .def_static("from_direction", &Quaternion::fromDirection)
        .def_static("from_direction", &Quaternion::fromDirection)
        .def(py::self == py::self)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self * Vector3())
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self *= py::self)
        .def("norm", &Quaternion::length)
        .def("normalize", &Quaternion::normalize)
        ;
}

EulerAngles PyRotations::CreateEulerAngles(float ax, float ay, float az, const py::object & rotationOrder, int rotationType)
{
    EulerAngles::Axes axes = ToAxes(rotationOrder);
    return EulerAngles(ax, ay, az, axes, RotationSpace(rotationType));
}

EulerAngles::Axes PyRotations::ToAxes(const py::object& o)
{
    std::vector<int> vec = PythonAPI::Instance().toVec<int>(o);
    EulerAngles::Axes axes;
    for (size_t i = 0; i < vec.size(); i++) {
        axes[i] = Axis(vec[i]);
    }
    return axes;
}


}
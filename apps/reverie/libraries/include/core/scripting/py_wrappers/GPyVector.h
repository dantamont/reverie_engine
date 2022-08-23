#pragma once

// External
#include <pybind11/operators.h>

// Internal
#include "core/scripting/GPythonWrapper.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

/// @brief Class to encapsulate initialization of a Vector pybind wrapper
class PyVector{
public:

    template<typename T, size_t N>
    static Vector<T, N> Create(const py::object& po) {
        using VecClass = Vector<T, N>;
        VecClass vec = VecClass::EmptyVector();
        PythonAPI::Instance().toVec(po, vec);
        return vec;
    }

    /// @brief Wrap a behavior for Python
    template<typename T, size_t N>
    static void PyInitialize(py::module_& m, const char* className) {
        using VecClass = Vector<T, N>;
        auto vecClass = py::class_<VecClass>(m, className)
            .def(py::init<>())
            .def(py::init<const VecClass&>())
            .def(py::init<>(&PyVector::Create<T, N>))
            .def("lerp", &VecClass::lerp)
            .def("__str__", [](const VecClass& vec) {return std::string(vec); })
            .def("__repr__", [](const VecClass& vec) {return std::string(vec); })
            .def(py::self + py::self)
            .def(py::self + T())
            .def(py::self + int())
            .def("__add__", [](const VecClass& vec, const py::object& po) {
            VecClass v2 = VecClass::EmptyVector();
            PythonAPI::Instance().toVec(po, v2);
            return vec + v2;
        })
            .def(py::self += py::self)
            .def(py::self += T())
            .def(py::self - py::self)
            .def("__sub__", [](const VecClass& vec, const py::object& po) {
            VecClass v2 = VecClass::EmptyVector();
            PythonAPI::Instance().toVec(po, v2);
            return vec - v2;
        })
            .def(-py::self)
            .def(py::self -= py::self)
            .def(py::self -= T())
            .def(py::self * py::self)
            .def(py::self * T())
            .def(T() * py::self)
            .def(py::self * int())
            .def("__mul__", [](const VecClass& vec, const py::object& po) {
                if (PythonAPI::Instance().isListLike(po.ptr())) {
                    VecClass v2 = VecClass::EmptyVector();
                    PythonAPI::Instance().toVec(po, v2);
                    v2 *= vec;
                    return v2;
                }
                else {
                    return vec * PythonAPI::Instance().cType<T>(po.ptr());
                }
            })
            .def("__getitem__", [](const VecClass &v, size_t i) {
                if (i >= v.size()) {
                    throw py::index_error();
                }
                return v[i];
            }) // See: https://github.com/pybind/pybind11/blob/master/tests/test_sequences_and_iterators.cpp
            .def(py::self *= py::self)
            .def(py::self *= T())
            .def(py::self == py::self)
            .def(py::self != py::self)
            .def("to_list", [](const VecClass& v) { return PythonAPI::Instance().toPyTuple(v.asStdVector()); })
            .def("length", &VecClass::length)
            .def("length_squared", &VecClass::lengthSquared)
            .def("normalized", &VecClass::normalized)
            .def("normalize", &VecClass::normalize)
            ;

        // Type-specific specializations
        if constexpr (N >= 1) {
            vecClass.def_property("x", &VecClass::x, &VecClass::setX);
        }
        if constexpr (N == 2) {
            vecClass.def(py::init<T, T>());
        }
        if constexpr (N >= 2) {
            vecClass.def_property("y", &VecClass::y, &VecClass::setY);
        }   
        if constexpr (N >= 3) {
            vecClass.def_property("z", &VecClass::z, &VecClass::setZ);
        }
        if constexpr (N == 4) {
            vecClass.def(py::init<T, T, T, T>());
        }
        if constexpr (N >= 4) {
            vecClass.def_property("w", &VecClass::w, &VecClass::setW);
        }

        if constexpr (N == 3) {
            vecClass.def(py::init<T, T, T>());

            vecClass.def_static("left", VecClass::Left);
            vecClass.def_static("right", VecClass::Right);
            vecClass.def_static("up", VecClass::Up);
            vecClass.def_static("down", VecClass::Down);
            vecClass.def_static("forward", VecClass::Forward);
            vecClass.def_static("back", VecClass::Back);

            vecClass.def("cross", &VecClass::cross);
            vecClass.def("dot", &VecClass::dot);
        }
    }

};

} // End namespaces

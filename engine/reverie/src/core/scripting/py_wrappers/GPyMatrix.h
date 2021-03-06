/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PY_MATRIX
#define GB_PY_MATRIX

// External
#include "../GPythonWrapper.h"
#include "../../geometry/GMatrix.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class to encapsulate initialization of a shader program pybind wrapper
class PyMatrix{
public:

    template<typename T, size_t N>
    static SquareMatrix<T, N> Create(const py::object& po) {
        using MatClass = SquareMatrix<T, N>;
        MatClass mat = MatClass::EmptyMatrix();
        PythonAPI::get()->toMatrix(po.ptr(), mat);
        return mat;
    }

    /// @brief Wrap a behavior for Python
    template<typename T, size_t N>
    static void PyInitialize(py::module_& mod, const char* className) {
        using MatClass = SquareMatrix<T, N>;
        auto matClass = py::class_<MatClass>(mod, className)
            .def(py::init<>())
            .def(py::init<MatClass>())
            .def(py::init<>(&PyMatrix::Create<T, N>))
            .def("__str__", [](const MatClass& mat) {return QString(mat).toStdString(); })
            .def("__repr__", [](const MatClass& mat) {return QString(mat).toStdString(); })
            .def("at", py::overload_cast<size_t, size_t>(&MatClass::at))
            .def("at", py::overload_cast<size_t, size_t>(&MatClass::at, py::const_))
            .def("column", &MatClass::column)
            .def("transposed", &MatClass::transposed)
            .def("set_to_identity", &MatClass::setToIdentity)
            .def("diagonal", &MatClass::diagonal)
            .def("transpose_multiply", //&MatClass::transposeMultiply<N>
                [](const MatClass& m1, const MatClass& m2) {
                    // Need to return SquareMatrix, not base class
                    auto res = m1.transposeMultiply(m2);
                    auto squareRes = reinterpret_cast<const MatClass*>(&res);
                    return *squareRes;
                }
            )
            .def("determinant", &MatClass::determinant)
            .def("inversed", //&MatClass::inversed
                [](const MatClass& m) {
                    // Need to return SquareMatrix, not base class
                    auto res = m.inversed();
                    auto squareRes = reinterpret_cast<const MatClass*>(&res);
                    return *squareRes;
                }
            )
            .def(py::self + py::self)
            .def("__add__", [](const MatClass& m, const py::object& po) {
                MatClass m2 = MatClass::EmptyMatrix();
                PythonAPI::get()->toMatrix(po.ptr(), m2);
                return m + m2;
            })
            .def(py::self += py::self)
            .def(py::self - py::self)
            .def("__sub__", [](const MatClass& m, const py::object& po) {
                MatClass m2 = MatClass::EmptyMatrix();
                PythonAPI::get()->toMatrix(po.ptr(), m2);
                return m - m2;
            })
            .def(py::self -= py::self)
            .def(py::self * py::self)
            .def(py::self * T())
            .def(py::self * int())
            .def("__mul__", [](const MatClass& m, const py::object& po) {
                if (PythonAPI::get()->isListLike(po.ptr())) {
                    // If multiplying by a vector of vectors
                    MatClass m2 = MatClass(PythonAPI::get()->toVecOfVec<T>(po.ptr()));
                    return m * m2;
                }
                else {
                    // If multiplying by a value
                    return m * PythonAPI::get()->cType<T>(po.ptr());
                }
            })
            .def(py::self *= py::self)
            .def(py::self == py::self)
            .def("to_list", [](const MatClass& m) { return PythonAPI::get()->toPyTuple(m.asStdVector()); })
            ;

        // Type-specific specializations
        if constexpr (N == 4) {
            matClass.def("add_scale", &MatClass::addScale<3>);
            matClass.def("add_scale", &MatClass::addScale<4>);
            matClass.def("add_rotation", &MatClass::addRotate);
            matClass.def_property("translation", &MatClass::getTranslationVector, &MatClass::setTranslation);
        }
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
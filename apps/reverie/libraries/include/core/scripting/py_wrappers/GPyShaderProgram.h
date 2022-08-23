#pragma once

// External
#include "core/scripting/GPythonWrapper.h"

namespace rev {

class Uniform;

/// @brief Class to encapsulate initialization of a shader program pybind wrapper
class PyShaderProgram{
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

    //static PyObject* uniformToPyObject(const Uniform& uniform);
};


} // End namespaces

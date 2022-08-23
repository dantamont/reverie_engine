#pragma once

// External
#include "core/scripting/GPythonWrapper.h"

namespace rev {

class TransformComponent;

/// @brief Class to encapsulate initialization of Transform Component pybind wrapper
class PyTransformComponent {
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

    /// @brief Static wrapper method for setting local position from a python object
    static void SetLocalPosition(TransformComponent& tr, const py::object& po);
};


} // End namespaces

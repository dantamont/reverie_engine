/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PY_SCENARIO_H
#define GB_PY_SCENARIO_H

// External
#include "core/scripting/GPythonWrapper.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scene;
class Scenario;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class to encapsulate initialization of Scene pybind wrapper
class PyScenario{
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
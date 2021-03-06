/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PY_SCENE_H
#define GB_PY_SCENE_H

// External
#include "../GPythonWrapper.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class Scene;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class to encapsulate initialization of Scene pybind wrapper
class PyScene{
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
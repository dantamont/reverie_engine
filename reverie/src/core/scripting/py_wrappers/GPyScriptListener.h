/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PY_SCRIPT_LISTENER_H
#define GB_PY_SCRIPT_LISTENER_H

// External
#include "../GPythonWrapper.h"

// Internal
#include "../GScriptListener.h"
#include "../../events/GEvent.h"


namespace py = pybind11;

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief "Trampoline" class to inherit from ScriptListener in Python
class PyScriptListener : public ScriptListener {
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

    /* Inherit the constructors */
    using ScriptListener::ScriptListener;

    /// @brief "Trampoline" method for initialize
    bool eventTest(const CustomEvent& ev) override {
        PYBIND11_OVERRIDE(
            bool,            // Return type (ret_type)
            ScriptListener,  // Parent class (cname)
            eventTest,       // Name of function in C++ (must match Python name) (fn)
            ev
        );
    }
    /// @brief "Trampoline" method for update
    void perform(const CustomEvent& ev) override {
        PYBIND11_OVERRIDE(
            void,            // Return type (ret_type)
            ScriptListener,  // Parent class (cname)
            perform,          // Name of function in C++ (must match Python name) (fn)
            ev
        );
    }

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
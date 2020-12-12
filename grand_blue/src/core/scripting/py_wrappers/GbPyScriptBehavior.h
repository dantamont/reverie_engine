/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_PY_SCRIPT_BEHAVIOR_H
#define GB_PY_SCRIPT_BEHAVIOR_H

// External
#include "../GbPythonWrapper.h"

// Internal
#include "../GbScriptBehavior.h"


namespace py = pybind11;

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief "Trampoline" class to inherit from ScriptBehavior in Python
class PyScriptBehavior : public ScriptBehavior {
public:

    /// @brief Wrap a behavior for Python
    static void PyInitialize(py::module_& m);

    /* Inherit the constructors */
    using ScriptBehavior::ScriptBehavior;

    /// @brief "Trampoline" method for initialize
    void initialize() override {
        PYBIND11_OVERRIDE(
            void,            // Return type (ret_type)
            ScriptBehavior,  // Parent class (cname)
            initialize       // Name of function in C++ (must match Python name) (fn)
        );
    }
    /// @brief "Trampoline" method for update
    void update(unsigned long deltaMs) override {
        PYBIND11_OVERRIDE(
            void,            // Return type (ret_type)
            ScriptBehavior,  // Parent class (cname)
            update,          // Name of function in C++ (must match Python name) (fn)
            deltaMs
        );
    }

    /// @brief "Trampoline" method for fixed update
    void fixedUpdate(unsigned long deltaMs) override {
        PYBIND11_OVERRIDE_NAME( // _NAME required when function name different in C++ than Python
            void,            /* Return type (ret_type) */
            ScriptBehavior,  /* Parent class (cname) */
            "fixed_update",  /* Name of function in Python (name)*/
            fixedUpdate,     /* Name of function in C++  */
            deltaMs          /* Argument(s) */
        );
    }

    /// @brief "Trampoline" method for onSuccess
    void onSuccess() override {
        PYBIND11_OVERRIDE_NAME( // _NAME required when function name different in C++ than Python
            void,            /* Return type (ret_type) */
            ScriptBehavior,  /* Parent class (cname) */
            "on_success",  /* Name of function in Python (name)*/
            onSuccess
        );
    }

    /// @brief "Trampoline" method for onFail
    void onFail() override {
        PYBIND11_OVERRIDE_NAME( // _NAME required when function name different in C++ than Python
            void,            /* Return type (ret_type) */
            ScriptBehavior,  /* Parent class (cname) */
            "on_fail",  /* Name of function in Python (name)*/
            onFail
        );
    }

    /// @brief "Trampoline" method for onAbort
    void onAbort() override {
        PYBIND11_OVERRIDE_NAME( // _NAME required when function name different in C++ than Python
            void,            /* Return type (ret_type) */
            ScriptBehavior,  /* Parent class (cname) */
            "on_abort",      /* Name of function in Python (name)*/
            onAbort
        );
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
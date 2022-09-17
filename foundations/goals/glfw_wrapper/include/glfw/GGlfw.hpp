#pragma once

// External
#include "fortress/layer/framework/GSingleton.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace rev {

/// @class Glfw
/// @brief Wrapper for GLFW library
class Glfw: public SingletonInterface<Glfw>
{
public:

    /// @name Static
    /// @{
    /// @}

    /// @name Destructor
    /// @{

    ~Glfw() {
        glfwTerminate();
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Get primary monitor
    inline GLFWmonitor* getPrimaryMonitor() const {
        return glfwGetPrimaryMonitor();
    }

    /// @brief Get physical size from a monitor
    inline void getMonitorPhysicalSize(GLFWmonitor* monitor, int& outWidthMm, int& outHeightMm) const {
        glfwGetMonitorPhysicalSize(monitor, &outWidthMm, &outHeightMm);
    }

    /// @}

protected:
    /// @name Private methods
    /// @{

    /// @brief Error callback
    static void errorCallback(int error, const char* description) {
        fprintf(stderr, "Error: %s\n", description);
        if (error != 0) {
            assert(false && "Error in GLFW");
        }
    }

    /// @brief Constructor
    Glfw() {
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            assert(false && "Failed to initialize GLFW");
        }
    }

    /// @}

    /// @name Private members
    /// @{

    /// @}
};

} // End namespaces

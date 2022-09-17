#pragma once

#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @class Screen
/// @brief Class representing a screen
class Screen {
public:

    /// @brief Return the primary screen
    static Screen* PrimaryScreen() {
        static Screen s_primaryScreen;
        return &s_primaryScreen;
    }

    /// @todo Implement
    Float32_t logicalDotsPerInch() {
        return 1;
    }

    /// @todo Implement
    Float32_t logicalDotsPerInchX() {
        return 1;
    }    

    /// @todo Implement
    Float32_t logicalDotsPerInchY() {
        return 1;
    }

private:

};

} // End namespaces

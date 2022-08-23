#pragma once

// Std
#include "geppetto/qt/input/GInputHandlerInterface.h"

namespace rev {

/// @class InputHandler
/// @copydoc rev::InputHandlerInterface
class InputHandler: public InputHandlerInterface {
public:
    using InputHandlerInterface::InputHandlerInterface;

protected:

    /// @brief Dispatch event to any other entities that should process it
    void dispatchEvent(QInputEvent* inputEvent) const override;

};
Q_DECLARE_METATYPE(InputHandler)

} // End rev namespace

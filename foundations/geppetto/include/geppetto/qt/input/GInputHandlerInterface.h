#pragma once

// Std
#include <list>
#include <set>
#include <map>
#include <shared_mutex>
#include <thread>

// QT
#include <QKeyEvent>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QKeySequence>

// Internal
#include "fortress/containers/math/GVector.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {

class GLWidgetInterface;
class InputHandlerInterface;

/// @struct UserInput
/// @brief A generalization of an input
struct UserInput {

    enum PeripheralType {
        kUnrecognized = -1,
        kKey,
        kMouse
    };

    enum ActionType {
        kNoAction,
        kPress,
        kRelease,
        kScroll,
        kNUM_ACTION_TYPES
    };

    enum TimingType {
        kNone,
        kDoubleClick
    };

    UserInput(PeripheralType type);
    ~UserInput();

    /// @details Array default initializes to zero
    std::array<ulong, kNUM_ACTION_TYPES> m_timeStamps; ///< The most recent timestamps (in ms since arbitrary time) of each action type
    ActionType m_actionType = ActionType::kNoAction; ///< Type of event
    TimingType m_timingType = TimingType::kNone; ///< Type of timing
    PeripheralType m_inputType; /// < Type of peripheral used for input
};


/// @struct MouseInput
/// @brief A generalization of a mouse input
struct MouseInput : public UserInput {

    MouseInput();
    ~MouseInput();

    friend bool operator==(const MouseInput& lhs, const MouseInput& rhs);

    /// @brief If this input is a press
    bool isPress() const { return m_actionType == kPress; }

    /// @brief If this input is a release
    bool isRelease() const { return m_actionType == kRelease; }

    /// @brief If this input is a double click
    bool isDoubleClick() const { return m_timingType == kDoubleClick; }

    Qt::MouseButton m_mouseButton = Qt::MouseButton::NoButton; ///< The mouse button pressed
    Qt::MouseButtons m_activeButtons; ///< Other mouse buttons held down at time of this press
    Vector2 m_angleDelta; ///<  Scroll amount (in radians), x-component is only for horizontal-scroll enabled mice

    /// @}
};


/// @struct KeyInput
/// @brief A generalization of a key input
struct KeyInput: public UserInput {

    KeyInput();
    ~KeyInput();

    /// @brief Get QString representation of the key input
    QString getKeyString() const { return QKeySequence(m_key).toString(); }

    /// @brief Whether this input item has the same key as another
    bool hasSameKeys(const KeyInput& other) const;

    /// @brief If this input is a press
    bool isPress() const { return m_actionType == kPress; }

    /// @brief If this input is a release
    bool isRelease() const { return m_actionType == kRelease; }

    /// @brief If this input is a double click
    bool isDoubleClick() const { return m_timingType == kDoubleClick; }

    friend bool operator==(const KeyInput& lhs, const KeyInput& rhs);

    Qt::Key m_key = Qt::Key::Key_unknown; ///< The key pressed
    Qt::KeyboardModifiers m_modifiers; ///< Modifiers to input ("Shift key, ctrl key, alt key, etc")

};

/// @class MouseHandler
/// @brief Class that handles polling-style inputs
/// @see https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class MouseHandler {
public:

    static tsl::robin_map<QString, Qt::MouseButton> MOUSE_BUTTON_MAP;

    MouseHandler();
    MouseHandler(InputHandlerInterface* handler);
    ~MouseHandler();


    /// @brief Whether mouse button was pressed or not
    bool wasPressed(const Qt::MouseButton& btn);
    bool wasPressed(const QString& btnStr);

    bool wasReleased(const Qt::MouseButton& btn);
    bool wasReleased(const QString& btnStr);

    bool wasDoubleClicked(const Qt::MouseButton& key);
    bool wasDoubleClicked(const QString& key);

    /// @brief Whether the given input is held or not
    bool isHeld(const Qt::MouseButton& key);
    bool isHeld(const QString& key);

    /// @brief Whether or not the mouse wheel was scrolled
    bool wasScrolled() const { return m_scrolled; }

    /// @brief Whether the mouse was moved or not
    bool wasMoved() const { return m_moved; }

    /// @brief Get the change in scroll amount since the last frame
    const Vector2& scrollDelta() const { return m_scrollDelta; }

    /// @brief Get the change in mouse position since the last frame
    const Vector2& mouseDelta() const { return m_mouseDelta; }

    /// @brief Get normalized mouse position in widget space
    Vector2 normalizeMousePosition() const;

    /// @brief Get mouse position in widget space
    Vector2 widgetMousePosition() const;

    /// @brief Get global mouse position
    Vector2 globalMousePosition() const;


protected:

    friend class InputHandlerInterface;


    /// @brief Start input collection for a new frame
    void onUpdate(double deltaSec);

    /// @brief Update mouse position using input listener thread
    void onUpdateMoved();

    /// @brief Get the input corresponding to the given mouse button, creating if it doesn't exist
    MouseInput& getInput(const Qt::MouseButton& btn);

    /// @brief Determine if key was inputted since the last frame
    bool wasInput(const Qt::MouseButton& btn, 
        const MouseInput::ActionType& actionType,
        const MouseInput::TimingType& timing);

    /// @brief Handle a scroll event
    void handleScrollEvent(QWheelEvent* event);

    /// @brief Handle a mouse press event
    void handleMousePressEvent(QMouseEvent* event);

    /// @brief Handle a mouse release event
    void handleMouseReleaseEvent(QMouseEvent* event);

    /// @brief Handle a mouse release event
    void handleMouseDoubleClickEvent(QMouseEvent* event);

    /// @brief Handle a mouse release event
    void handleMouseMoveEvent(QMouseEvent* event);

    /// @brief Convert string to mouse button
    Qt::MouseButton toMouseButton(const QString& str);

private:

    std::list<MouseInput> m_polledInputs; ///< Input events since last frame
    std::list<MouseInput> m_inputsCache; ///< Cache to temporarily store input events since last frame
    tsl::robin_map<Qt::MouseButton, MouseInput> m_mouseInputs; ///< All new mouse inputs

    MouseInput m_scrollInput; ///< Most recent scroll input
    bool m_moved; ///< Whether the mouse was moved or not
    bool m_scrolled; ///< Whether the mouse was scrolled or not
    bool m_scrolledCache; ///< Cached value used to track if mouse was scrolled or not

    Vector2 m_prevScreenPosition; ///< Previous mouse position in NDC (-1, 1)
    Vector2 m_mouseDelta; ///< Change in mouse position since last frame (in NDC)
    Vector2 m_scrollDelta; ///< Amount scrolled since the last frame
    InputHandlerInterface* m_inputHandler; ///< Input handler 

};
Q_DECLARE_METATYPE(MouseHandler)


/// @class KeyHandler
/// @brief Class that handles polling-style inputs
/// @see https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class KeyHandler {
public:
    KeyHandler();
    KeyHandler(InputHandlerInterface* handler);
    ~KeyHandler();

    /// @brief Whether a key was pressed or released
    bool wasPressed(const Qt::Key& key);
    bool wasPressed(const QString& key);

    bool wasReleased(const Qt::Key& key);
    bool wasReleased(const QString& key);

    bool wasDoubleClicked(const Qt::Key& key);
    bool wasDoubleClicked(const QString& key);

    /// @brief Whether the given input is held or not
    bool isHeld(const Qt::Key& key) const;
    bool isHeld(const QString& key);

protected:

    friend class InputHandlerInterface;

    /// @brief Convert string to key
    Qt::Key toKey(const QString& str);

    /// @brief Start input collection for a new frame
    void onUpdate(double deltaSec);

    /// @brief Handle a key press event
    void handleKeyPressEvent(QKeyEvent* event) ;

    /// @brief Handle a key release event
    void handleKeyReleaseEvent(QKeyEvent* event);

    /// @brief Get the input corresponding to the given key, creating if it doesn't exist
    KeyInput& getInput(const Qt::Key& key);

    /// @brief Determine if key was inputted since the last frame
    bool wasInput(const Qt::Key& key, const KeyInput::ActionType& actionType, KeyInput::TimingType timingType = KeyInput::kNone);

private:

    std::list<KeyInput> m_polledInputs; ///< Input events since last frame
    std::list<KeyInput> m_inputsCache; ///< Cache to temporarily store input events since last frame
    tsl::robin_map<Qt::Key, KeyInput> m_keyInputs; ///< All keyboard inputs
    InputHandlerInterface* m_inputHandler; ///< Input handler

};
Q_DECLARE_METATYPE(KeyHandler)



/// @todo Add a fixedUpdate-compatible version of input handling
/// @class InputHandlerInterface
/// @brief Class that handles polling-style inputs
/// @see https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class InputHandlerInterface {
public:

    InputHandlerInterface();
    InputHandlerInterface(QWidget* widget);
    ~InputHandlerInterface();

    /// @brief Handle inputs from keyboard
    KeyHandler& keyHandler() { return m_keyHandler; }
    const KeyHandler& keyHandler() const { return m_keyHandler; }

    /// @brief Handle inputs from mouse
    MouseHandler& mouseHandler() { return m_mouseHandler; }
    const MouseHandler& mouseHandler() const { return m_mouseHandler; }

    /// @brief Cache all inputs since the last frame
    void update(double deltaSec);

    /// @brief Handle a Qt event
    /// @details Returns true if handling an input event
    bool handleEvent(QInputEvent* inputEvent);

protected:
    friend class MouseHandler;
    friend class KeyHandler;

    /// @brief Dispatch event to any other entities that should process it
    virtual void dispatchEvent(QInputEvent* inputEvent) const = 0;

    Uint64_t m_holdTime; ///< Time in ms for hold
    Uint64_t m_repeatClickTime; ///<  Time in ms for multi-click
    QElapsedTimer m_timer; ///< Timer for input events
    KeyHandler m_keyHandler; ///< Handle inputs from keyboard
    MouseHandler m_mouseHandler; ///< Handle inputs from mouse

    QWidget* m_widget{ nullptr }; ///< Pointer to the widget whose inputs are being handled

};

} // End rev namespace

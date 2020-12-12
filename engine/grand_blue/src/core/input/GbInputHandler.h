/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_INPUT_HANDLER_H
#define GB_INPUT_HANDLER_H

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
#include "../GbObject.h"
#include "../geometry/GbVector.h"
#include "../containers/GbContainerExtensions.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
namespace View{
class GLWidget;
}
class InputHandler;
/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct UserInput
/// @brief A generalization of an input
struct UserInput {
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

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

    /// @}

     //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    UserInput(PeripheralType type);
    ~UserInput();

    /// @}


    /// @brief The most recent timestamps (in ms since arbitrary time) of each action type
    /// @details Array default initializes to zero
    std::array<ulong, kNUM_ACTION_TYPES> m_timeStamps;

    /// @brief Type of event
    ActionType m_actionType = ActionType::kNoAction;

    /// @brief Type of timing
    TimingType m_timingType = TimingType::kNone;

    /// @brief Type of peripheral used for input
    PeripheralType m_inputType;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MouseInput
/// @brief A generalization of a mouse input
struct MouseInput : public UserInput {
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    MouseInput();
    ~MouseInput();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    friend bool operator==(const MouseInput& lhs, const MouseInput& rhs);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief If this input is a press
    bool isPress() const { return m_actionType == kPress; }

    /// @brief If this input is a release
    bool isRelease() const { return m_actionType == kRelease; }

    /// @brief If this input is a double click
    bool isDoubleClick() const { return m_timingType == kDoubleClick; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    /// @brief The mouse button pressed
    Qt::MouseButton m_mouseButton = Qt::MouseButton::NoButton;

    /// @brief Other mouse buttons held down at time of this press
    Qt::MouseButtons m_activeButtons;

    /// @brief Scroll amount (in radians), x-component is only for horizontal-scroll enabled mice
    Vector2 m_angleDelta;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct KeyInput
/// @brief A generalization of a key input
struct KeyInput: public UserInput {
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    KeyInput();
    ~KeyInput();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

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

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    friend bool operator==(const KeyInput& lhs, const KeyInput& rhs);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{

    /// @brief The key pressed
    Qt::Key m_key = Qt::Key::Key_unknown;

    /// @brief Modifiers to input ("Shift key, ctrl key, alt key, etc")
    Qt::KeyboardModifiers m_modifiers;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class MouseHandler
/// @brief Class that handles polling-style inputs
/// @note See:
/// https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class MouseHandler : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static tsl::robin_map<QString, Qt::MouseButton> MOUSE_BUTTON_MAP;

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    MouseHandler();
    MouseHandler(InputHandler* handler);
    ~MouseHandler();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Property
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "MouseHandler"; }
    virtual const char* namespaceName() const override { return "Gb::MouseHandler"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class InputHandler;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Start input collection for a new frame
    void onUpdate(unsigned long deltaMs);

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

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Input events since last frame
    std::list<MouseInput> m_polledInputs;

    /// @brief Cache to temporarily store input events since last frame
    std::list<MouseInput> m_inputsCache;

    /// @brief All new mouse inputs
    tsl::robin_map<Qt::MouseButton, MouseInput> m_mouseInputs;

    /// @brief Most recent scroll input
    MouseInput m_scrollInput;

    /// @brief Whether the mouse was moved or not
    bool m_moved;
    bool m_scrolled;
    bool m_scrolledCache;

    /// @brief Previous mouse position in NDC (-1, 1)
    Vector2 m_prevScreenPosition;

    /// @brief Change in mouse position since last frame (in NCD)
    Vector2 m_mouseDelta;

    /// @brief Amount scrolled since the last frame
    Vector2 m_scrollDelta;

    /// @brief The mutex used to manage mouse input retrieval
    //std::shared_mutex m_mouseInputMutex;

    /// @brief Mouse position used for tracking thread
    //Vector2 m_prevTrackingMousePosition;

    /// @brief Input handler 
    InputHandler* m_inputHandler;

    /// @}
};
Q_DECLARE_METATYPE(MouseHandler)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class KeyHandler
/// @brief Class that handles polling-style inputs
/// @note See:
/// https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class KeyHandler : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    KeyHandler();
    KeyHandler(InputHandler* handler);
    ~KeyHandler();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Property
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "KeyHandler"; }
    virtual const char* namespaceName() const override { return "Gb::KeyHandler"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class InputHandler;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Convert string to key
    Qt::Key toKey(const QString& str);

    /// @brief Start input collection for a new frame
    void onUpdate(unsigned long deltaMs);

    /// @brief Handle a key press event
    void handleKeyPressEvent(QKeyEvent* event) ;

    /// @brief Handle a key release event
    void handleKeyReleaseEvent(QKeyEvent* event);

    /// @brief Get the input corresponding to the given key, creating if it doesn't exist
    KeyInput& getInput(const Qt::Key& key);

    /// @brief Determine if key was inputted since the last frame
    bool wasInput(const Qt::Key& key, 
        const KeyInput::ActionType& actionType,
        KeyInput::TimingType timingType = KeyInput::kNone);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Input events since last frame
    std::list<KeyInput> m_polledInputs;

    /// @brief Cache to temporarily store input events since last frame
    std::list<KeyInput> m_inputsCache;

    /// @brief All keyboard inputs
    tsl::robin_map<Qt::Key, KeyInput> m_keyInputs;

    /// @brief Input handler 
    InputHandler* m_inputHandler;

    /// @}
};
Q_DECLARE_METATYPE(KeyHandler)


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Add a fixedUpdate-compatible version of input handling
/// @class InputHandler
/// @brief Class that handles polling-style inputs
/// @note See:
/// https://gamedev.stackexchange.com/questions/12146/polling-vs-event-driven-input
class InputHandler : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    InputHandler();
    InputHandler(View::GLWidget* widget);
    ~InputHandler();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Property
    /// @{

    /// @brief Handle inputs from keyboard
    KeyHandler& keyHandler() { return m_keyHandler; }
    const KeyHandler& keyHandler() const { return m_keyHandler; }


    /// @brief Handle inputs from mouse
    MouseHandler& mouseHandler() { return m_mouseHandler; }
    const MouseHandler& mouseHandler() const { return m_mouseHandler; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Cache all inputs since the last frame
    void update(unsigned long deltaMs);

    /// @brief Handle a Qt event
    /// @details Returns true if handling an input event
    bool handleEvent(QInputEvent* inputEvent);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "InputHandler"; }
    virtual const char* namespaceName() const override { return "Gb::InputHandler"; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class MouseHandler;
    friend class KeyHandler;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Callback for input listener thread
    //void inputThreadCallback();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Time in ms for hold
    size_t m_holdTime;

    /// @brief Time in ms for multi-click
    size_t m_repeatClickTime;

    /// @brief Timer for input events
    QElapsedTimer m_timer;

    /// @brief Handle inputs from keyboard
    KeyHandler m_keyHandler;

    /// @brief Handle inputs from mouse
    MouseHandler m_mouseHandler;

    /// @brief Pointer to the core engine
    View::GLWidget* m_widget;

    //bool m_threadRunning;

    ///// @brief The input listener thread, used only for mouse moves right now
    //static std::vector<std::thread> s_inputListenerThreads;
    //std::vector<std::shared_mutex> s_runningMutexes;

    /// @}
};
Q_DECLARE_METATYPE(InputHandler)

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
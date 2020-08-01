#include "GbInputHandler.h"
#include "../GbCoreEngine.h"
#include "../../view/GL/GbGLWidget.h"
#include "../events/GbEventManager.h"
#include "../GbConstants.h"
#include "../containers/GbContainerExtensions.h"

#include <QCursor>

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// UserInput
/////////////////////////////////////////////////////////////////////////////////////////////
UserInput::UserInput(PeripheralType type):
    m_inputType(type)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
UserInput::~UserInput()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MouseInput
/////////////////////////////////////////////////////////////////////////////////////////////
MouseInput::MouseInput():
    UserInput(kMouse)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MouseInput::~MouseInput()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const MouseInput & lhs, const MouseInput & rhs)
{
    return lhs.m_inputType == rhs.m_inputType &&
        lhs.m_mouseButton == rhs.m_mouseButton &&
        lhs.m_activeButtons == rhs.m_activeButtons &&
        lhs.m_actionType == rhs.m_actionType;
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// KeyInput
/////////////////////////////////////////////////////////////////////////////////////////////
KeyInput::KeyInput():
    UserInput(kKey)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
KeyInput::~KeyInput()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const KeyInput & lhs, const KeyInput & rhs)
{
    return lhs.m_inputType == rhs.m_inputType &&
        lhs.m_key == rhs.m_key &&
        lhs.m_modifiers == rhs.m_modifiers &&
        lhs.m_actionType == rhs.m_actionType;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyInput::hasSameKeys(const KeyInput & other) const
{
    return m_key == other.m_key && m_modifiers == other.m_modifiers;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Mouse Handler
/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<QString, Qt::MouseButton> MouseHandler::MOUSE_BUTTON_MAP = {
    {"left", Qt::LeftButton},
    {"right", Qt::RightButton},
    {"middle", Qt::MiddleButton}
};
/////////////////////////////////////////////////////////////////////////////////////////////
MouseHandler::MouseHandler()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MouseHandler::MouseHandler(InputHandler* handler):
    m_inputHandler(handler),
    m_moved(false),
    m_movedCache(false),
    m_scrolled(false),
    m_scrolledCache(false)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MouseHandler::~MouseHandler()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasPressed(const Qt::MouseButton & btn)
{
    return wasInput(btn, MouseInput::kPress, MouseInput::kNone);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasPressed(const QString & btnStr)
{
    return wasPressed(toMouseButton(btnStr));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasReleased(const Qt::MouseButton & btn)
{
    return wasInput(btn, MouseInput::kRelease, MouseInput::kNone);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasReleased(const QString & btnStr)
{
    return wasReleased(toMouseButton(btnStr));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasDoubleClicked(const Qt::MouseButton & btn)
{
    return wasInput(btn, MouseInput::kRelease, MouseInput::kDoubleClick);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasDoubleClicked(const QString & btnStr)
{
    return wasDoubleClicked(toMouseButton(btnStr));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::isHeld(const Qt::MouseButton& key) const
{
    bool held = false;
    if (m_mouseInputs.find(key) == m_mouseInputs.end()) {
        held = false;
    }
    else {
        const MouseInput& input = m_mouseInputs.at(key);
        if (input.isPress()) {
            long heldTime = m_inputHandler->m_timer.elapsed() - input.m_timeStamps.at(MouseInput::kPress);
            if (heldTime > long(m_inputHandler->m_holdTime)) {
                held = true;
            }
        }
    }
    return held;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::isHeld(const QString & btnStr)
{
    return isHeld(toMouseButton(btnStr));
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector2g MouseHandler::normalizeMousePosition() const
{
    double width = (double)m_inputHandler->m_widget->width();
    double height = (double)m_inputHandler->m_widget->height();
    Vector2g widgetPos = widgetMousePosition();
    
    // Convert coordinate range from 0-1 to -1-1
    double screenX = 2 * (widgetPos.x() / width) - 1.0;
    double screenY = 1.0 - 2 * (widgetPos.y() / height); // Flip, since widget-space origin is top-left
    return Vector2g(screenX, screenY);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector2g MouseHandler::widgetMousePosition() const
{
    // Get global mouse coordinate and map to widget space
    QPoint globalPos = QCursor::pos();
    QPoint pos = m_inputHandler->m_widget->mapFromGlobal(globalPos);
    return Vector2g(pos.x(), pos.y());
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector2g MouseHandler::globalMousePosition() const
{
    QPoint pos = QCursor::pos();
    return Vector2g(pos.x(), pos.y());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::onUpdate(unsigned long deltaMs)
{    
    Q_UNUSED(deltaMs)

    // Determine change in mouse position
    Vector2g currentMousePos = normalizeMousePosition();
    m_mouseDelta = currentMousePos - m_prevScreenPosition;
    m_prevScreenPosition = currentMousePos;

    // Move inputs from temporary cache to referenced map
    m_polledInputs.clear();
    m_polledInputs.splice(m_polledInputs.begin(), m_inputsCache);
    m_inputsCache.clear();
    m_moved = m_movedCache;
    m_movedCache = false;
    m_scrolled = m_scrolledCache;
    m_scrolledCache = false;

    if (m_scrolled) {
        // If scrolled since the last frame, update scroll amount
        m_scrollDelta = m_scrollInput.m_angleDelta;
    }
    else {
        // Set scroll amount to zero
        m_scrollDelta = Vector2g();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
MouseInput & MouseHandler::getInput(const Qt::MouseButton & btn)
{
    // Replace input in list if exists already
    if (m_mouseInputs.find(btn) == m_mouseInputs.end()) {
        m_mouseInputs[btn] = MouseInput();
        m_mouseInputs[btn].m_mouseButton = btn;
    }
    return m_mouseInputs[btn];
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool MouseHandler::wasInput(const Qt::MouseButton& btn, 
    const MouseInput::ActionType& actionType,
    const MouseInput::TimingType& timing)
{
    auto iter = std::find_if(m_polledInputs.begin(), m_polledInputs.end(),
        [&](const MouseInput& input) {
        return input.m_mouseButton == btn && 
            input.m_actionType == actionType  &&
            input.m_timingType == timing;
    });

    if (iter == m_polledInputs.end()) {
        return false;
    }
    else {
        return true;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::handleScrollEvent(QWheelEvent * event)
{
    // Get input item corresponding to mouse button
    Qt::MouseButtons buttons = event->buttons();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    MouseInput& input = m_scrollInput;

    // Update input attributes
    input.m_activeButtons = buttons;
    input.m_timingType = MouseInput::TimingType::kNone;
    input.m_actionType = MouseInput::kScroll;
    input.m_timeStamps[MouseInput::kScroll] = timeStamp;
    input.m_angleDelta = Vector2g(Constants::DEG_TO_RAD * event->angleDelta().x() / 8.0,
        Constants::DEG_TO_RAD * event->angleDelta().y() / 8.0);

    // Add input to cache for polling
    Vec::EmplaceBack(m_inputsCache, input);
    m_scrolledCache = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::handleMousePressEvent(QMouseEvent * event)
{
    // Get input item corresponding to mouse button
    Qt::MouseButton button = (Qt::MouseButton)event->button();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    MouseInput& input = getInput(button);

#ifdef DEBUG_MODE
    //logInfo("Clicked mouse at " + QString::number(timeStamp));
#endif

    // Update input attributes
    input.m_activeButtons = event->buttons();
    input.m_timingType = MouseInput::TimingType::kNone;
    input.m_actionType = MouseInput::kPress;
    input.m_timeStamps[MouseInput::kPress] = timeStamp;

    // Add input to cache for polling
    Vec::EmplaceBack(m_inputsCache, input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::handleMouseReleaseEvent(QMouseEvent * event)
{
    // Get input item corresponding to mouse button
    Qt::MouseButton button = (Qt::MouseButton)event->button();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    MouseInput& input = getInput(button);

    // Update input attributes
    input.m_activeButtons = event->buttons();
    input.m_actionType = MouseInput::kRelease;
    input.m_timeStamps[MouseInput::kRelease] = timeStamp;

    // Add input to cache for polling
    Vec::EmplaceBack(m_inputsCache, input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::handleMouseDoubleClickEvent(QMouseEvent * event)
{
    // Get input item corresponding to mouse button
    Qt::MouseButton button = (Qt::MouseButton)event->button();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    MouseInput& input = getInput(button);

#ifdef DEBUG_MODE
    //logInfo("Double-clicked mouse at " + QString::number(timeStamp));
#endif

    // Update input attributes
    input.m_activeButtons = event->buttons();
    input.m_actionType = MouseInput::kPress;
    input.m_timingType = MouseInput::TimingType::kDoubleClick;
    input.m_timeStamps[MouseInput::kPress] = timeStamp;

    // Add input to cache for polling
    Vec::EmplaceBack(m_inputsCache, input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MouseHandler::handleMouseMoveEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    m_movedCache = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Qt::MouseButton MouseHandler::toMouseButton(const QString & str)
{
    return MOUSE_BUTTON_MAP[str.toLower()];
}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// KeyHandler
/////////////////////////////////////////////////////////////////////////////////////////////
KeyHandler::KeyHandler()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
KeyHandler::KeyHandler(InputHandler* handler):
    m_inputHandler(handler)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
KeyHandler::~KeyHandler()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasPressed(const Qt::Key & key)
{
    return wasInput(key, KeyInput::kPress, KeyInput::kNone);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasPressed(const QString & key)
{
    return wasPressed(toKey(key));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasReleased(const Qt::Key & key)
{
    return wasInput(key, KeyInput::kRelease, KeyInput::kNone);

}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasReleased(const QString & key)
{
    return wasReleased(toKey(key));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasDoubleClicked(const Qt::Key & key)
{
    return wasInput(key, KeyInput::kRelease, KeyInput::kDoubleClick);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasDoubleClicked(const QString & key)
{
    return wasDoubleClicked(toKey(key));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::isHeld(const Qt::Key& key) const
{
    bool held = false;
    if (m_keyInputs.find(key) != m_keyInputs.end()) {
        const KeyInput& input = m_keyInputs.at(key);
        if (input.isPress()) {
            long heldTime = m_inputHandler->m_timer.elapsed() - input.m_timeStamps.at(KeyInput::kPress);
            if (heldTime > long(m_inputHandler->m_holdTime)) {
                held = true;
            }
        }
    }
    return held;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::isHeld(const QString & key)
{
    return isHeld(toKey(key));
}
/////////////////////////////////////////////////////////////////////////////////////////////
Qt::Key KeyHandler::toKey(const QString & str)
{
    QKeySequence seq(str);
    uint keyCode;

    // We should only working with a single key here
    if (seq.count() == 1)
        keyCode = seq[0];
    else {
        // Should be here only if a modifier key (e.g. Ctrl, Alt) is pressed.
        assert(seq.count() == 0);

        // Add a non-modifier key "A" to the picture because QKeySequence
        // seems to need that to acknowledge the modifier. We know that A has
        // a keyCode of 65 (or 0x41 in hex)
        seq = QKeySequence(str + "+A");
        assert(seq.count() == 1);
        assert(seq[0] > 65);
        keyCode = seq[0] - 65;
    }

    Qt::Key key = Qt::Key(keyCode);
    return key;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void KeyHandler::onUpdate(unsigned long deltaMs)
{
    Q_UNUSED(deltaMs);
    // Move inputs from temporary cache to referenced map
    m_polledInputs.clear();
    m_polledInputs.splice(m_polledInputs.begin(), m_inputsCache);
    m_inputsCache.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void KeyHandler::handleKeyPressEvent(QKeyEvent* event)
{
    // If is an auto-repeat (key hold, ignore)
    if (event->isAutoRepeat()) {
        return;
    }

    // Get input item corresponding to key
    Qt::Key key = (Qt::Key)event->key();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    KeyInput& input = getInput(key);

    // Determine action type
    if (input.isRelease()) {
        // If key was recently released
        ulong timeSinceRelease = ulong(timeStamp - input.m_timeStamps[KeyInput::kPress]);
        if (timeSinceRelease < m_inputHandler->m_repeatClickTime) {
            // If is a double click
            input.m_timingType = KeyInput::kDoubleClick;
#ifdef DEBUG_MODE
            //logInfo("Double clicked pressed");
#endif
        }
        else {
            // If is not a double click
            input.m_timingType = KeyInput::kNone;
#ifdef DEBUG_MODE
            //logInfo("Clicked");
#endif
        }
    }
    else {
        // If is a simple click
        input.m_timingType = KeyInput::kNone;
#ifdef DEBUG_MODE
        //logInfo("Clicked");
#endif
    }

    // Update input attributes
    input.m_modifiers = event->modifiers();
    input.m_actionType = KeyInput::kPress;
    input.m_timeStamps[KeyInput::kPress] = timeStamp;

    // Add input to cache for polling
    m_inputsCache.push_back(input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void KeyHandler::handleKeyReleaseEvent(QKeyEvent * event)
{
    // If is an auto-repeat (key hold, ignore)
    if (event->isAutoRepeat()) {
        return;
    }

    // Get input item corresponding to key
    Qt::Key key = (Qt::Key)event->key();
    ulong timeStamp = (ulong)m_inputHandler->m_timer.elapsed();;
    KeyInput& input = getInput(key);

    // Determine action type
    if (input.isDoubleClick()) {
        // If input was double clicked
        ulong timeSinceDblClick = ulong(timeStamp - input.m_timeStamps[KeyInput::kRelease]);
        if (timeSinceDblClick < m_inputHandler->m_repeatClickTime) {
            // If clicked in succession again
#ifdef DEBUG_MODE
            //logInfo("Double clicked released");
#endif
        }
        else {
            // If is a simple release
            input.m_timingType = KeyInput::TimingType::kNone;
#ifdef DEBUG_MODE
            //logInfo("Released");
#endif
        }
    }
    else {
        // If is a simple release
        input.m_timingType = KeyInput::TimingType::kNone;
#ifdef DEBUG_MODE
        //logInfo("Released");
#endif
    }

    // Update input attributes
    input.m_modifiers = event->modifiers();
    input.m_actionType = KeyInput::kRelease;
    input.m_timeStamps[KeyInput::kRelease] = timeStamp;

    // Add input to cache for polling
    m_inputsCache.push_back(input);
}
/////////////////////////////////////////////////////////////////////////////////////////////
KeyInput & KeyHandler::getInput(const Qt::Key& key)
{
    // Replace input in list if exists already
    if (m_keyInputs.find(key) == m_keyInputs.end()) {
        m_keyInputs[key] = KeyInput();
        m_keyInputs[key].m_key = key;
    }
    return m_keyInputs[key];
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool KeyHandler::wasInput(const Qt::Key& key, const KeyInput::ActionType& actionType, KeyInput::TimingType timingType)
{
    auto iter = std::find_if(m_polledInputs.begin(), m_polledInputs.end(),
        [&](const KeyInput& input) {
        return input.m_key == key && 
            input.m_actionType == actionType && 
            input.m_timingType == timingType;
    });

    if (iter == m_polledInputs.end()) {
        return false;
    }
    else {
        return true;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler::InputHandler() :
    m_widget(nullptr),
    m_keyHandler(this),
    m_mouseHandler(this)
{
    // Start timer
    m_timer.start();
}

/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler::InputHandler(View::GLWidget* widget):
    m_widget(widget),
    m_keyHandler(this),
    m_mouseHandler(this),
    m_holdTime(80),
    m_repeatClickTime(500)
{
    // Start timer
    m_timer.start();
}
/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler::~InputHandler()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void InputHandler::update(unsigned long deltaMs)
{
    // Collect mouse button presses
    m_mouseHandler.onUpdate(deltaMs);

    // Collect key presses
    m_keyHandler.onUpdate(deltaMs);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool InputHandler::handleEvent(QInputEvent * inputEvent)
{
    // Add event to event handler queue
    m_widget->m_engine->eventManager()->addEvent(inputEvent);

    // Process input for polling
    bool handled = true;
    KeyInput input;
    switch (inputEvent->type()) {
    case QEvent::KeyPress:
    {
        QKeyEvent* event = static_cast<QKeyEvent*>(inputEvent);
        m_keyHandler.handleKeyPressEvent(event);
        break; 
    }
    case QEvent::KeyRelease: {
        QKeyEvent* event = static_cast<QKeyEvent*>(inputEvent);
        m_keyHandler.handleKeyReleaseEvent(event);
        break;
    }
    case QEvent::Wheel: {
        QWheelEvent* event = static_cast<QWheelEvent*>(inputEvent);
        m_mouseHandler.handleScrollEvent(event);
        break;
    }
    case QEvent::MouseButtonPress: {
        QMouseEvent* event = static_cast<QMouseEvent*>(inputEvent);
        m_mouseHandler.handleMousePressEvent(event);
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent* event = static_cast<QMouseEvent*>(inputEvent);
        m_mouseHandler.handleMouseReleaseEvent(event);
        break;
    }
    case QEvent::MouseButtonDblClick: {
        QMouseEvent* event = static_cast<QMouseEvent*>(inputEvent);
        m_mouseHandler.handleMouseDoubleClickEvent(event);
        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent* event = static_cast<QMouseEvent*>(inputEvent);
        m_mouseHandler.handleMouseMoveEvent(event);
        break;
    }
    default:
#ifdef DEBUG_MODE
        throw("Error, input event not handled");
#endif
        handled = false;
        break;
    }

    return handled;
}






/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
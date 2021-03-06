/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_EVENT_LISTENERS_H
#define GB_EVENT_LISTENERS_H

// Std
#include <memory>
#include <vector>
#include <set>

// External
#include "../scripting/GPythonWrapper.h"

// Internal
#include "GEvent.h"
#include "../GObject.h"
#include "../mixins/GLoadable.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class EventListener;
class PythonClassScript;
class CoreEngine;
class SceneObject;
class ListenerComponent;

/////////////////////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class EventListener
/// @brief Custom event listener for simulation loop
class EventListener: public Object, public Identifiable, public Serializable {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    EventListener(ListenerComponent* engine);
    EventListener(ListenerComponent* engine, const QJsonValue& json);
    ~EventListener();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The script for this event listener
    PythonClassScript* script() { return m_script; }

    const std::set<int>& eventTypes() { return m_eventTypes; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Set event types to listen for
    /// @note Should be using type that is registered to Qt event system
    void setEventTypes(const std::vector<int> eventTypes);

    /// @brief Add an event type to listen for
    /// @note Some built-in types: 2 is mouse press, 3 is release, 4 is double click, 5 is move
    void addEventType(int type);

    /// @brief Check if action should be performed
    virtual bool testEvent(CustomEvent* ev);

    /// @brief Perform response to event
    virtual void perform(CustomEvent* ev);

    /// @brief Initialize this listener in python from the given listener script
    void initializeScript(const GString& filepath);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "EventListener"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::EventListener"; }
    /// @}

private:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief Pointer to the listener component that this event listener belongs to
    ListenerComponent* m_listenerComponent;

    /// @brief Path to the python script for this listener
    GString m_path;

    /// @brief The python script for this listener
    PythonClassScript* m_script;

    /// @brief The instantiation of the class from the script corresponding to this listener
    py::object m_pythonListener;

    /// @brief Event types accepted by this listener
    std::set<int> m_eventTypes;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
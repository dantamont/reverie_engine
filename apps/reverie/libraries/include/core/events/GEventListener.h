#pragma once

// Std
#include <memory>
#include <vector>
#include <set>

// External
#include "core/scripting/GPythonWrapper.h"

// Internal
#include "GEvent.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GIdentifiable.h"

namespace rev {

class EventListener;
class PythonClassScript;
class CoreEngine;
class SceneObject;
class ListenerComponent;


/// @class EventListener
/// @brief Custom event listener for simulation loop
class EventListener: public IdentifiableInterface {
public:

    /// @name Constructors/Destructor
    /// @{

    EventListener(ListenerComponent* engine);
    EventListener(ListenerComponent* engine, const nlohmann::json& json);
    ~EventListener();

    /// @}

    /// @name Properties
    /// @{

    /// @brief The script for this event listener
    PythonClassScript* script() { return m_script; }

    const std::set<int>& eventTypes() { return m_eventTypes; }

    /// @}

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

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const EventListener& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, EventListener& orObject);


    /// @}

private:

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

} // End namespaces

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_EVENT_H
#define GB_EVENT_H

#include <memory>
#include <map>

// QT
#include <QEvent>
#include <QJsonValue>
#include <QMetaType>

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////////////////////
enum EventType {
    kLogEvent,
    kPythonScriptEvent, // Subclass of SimEvent
    kAnimationEvent // Subclass of SimEvent
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Light-weight wrapper for generating unique QEvents
class CustomEvent : public QEvent {
public:
    CustomEvent();
    CustomEvent(int type);
    CustomEvent(QEvent* event);
    ~CustomEvent() {}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    json& data() { return m_data; }
    const json& data() const { return m_data; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CustomEvent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CustomEvent& orObject);


    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Obtain the type to register this evennt as
    QEvent::Type registeredType(int type);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Any data associated with the event
    json m_data;

    /// @brief Static map of all custom event types
    /// @details Index is the attempted event type, and the value is the event type that Qt
    /// assigned. These will usually be equivalent, but may not be if a type was already 
    /// registered
    // TODO: Use index of type in this map as event type for Event Listeners, adds a layer of abstraction
    static tsl::robin_map<int, QEvent::Type> s_registeredEventTypes;

    /// @}
};
Q_DECLARE_METATYPE(CustomEvent);
Q_DECLARE_METATYPE(CustomEvent*);



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Template for easily generating different event classes
/// @details e.g. Event<kLogEvent>
template<EventType E>
class Event : public QEvent {
public:
    Event() : QEvent(registeredType()) {}
    virtual ~Event(){}

    static QEvent::Type EVENT_TYPE;

private:
    static QEvent::Type registeredType();
};

/////////////////////////////////////////////////////////////////////////////////////////////
template<EventType E>
QEvent::Type Event<E>::registeredType()
{
    if (EVENT_TYPE == QEvent::None)
    {
        int generatedType = QEvent::registerEventType(int(E));
        EVENT_TYPE = static_cast<QEvent::Type>(generatedType);
    }
    return EVENT_TYPE;
}

template<EventType E> 
QEvent::Type Event<E>::EVENT_TYPE = QEvent::None;

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
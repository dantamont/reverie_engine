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
#include "../mixins/GLoadable.h"
#include "../containers/GContainerExtensions.h"

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
class CustomEvent : public QEvent, public Serializable {
public:
    CustomEvent();
    CustomEvent(int type);
    CustomEvent(QEvent* event);
    ~CustomEvent() {}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    QJsonObject& data() { return m_data; }
    const QJsonObject& data() const { return m_data; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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
    QJsonObject m_data;

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
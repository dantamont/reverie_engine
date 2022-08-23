#include "core/events/GEvent.h"

namespace rev {

CustomEvent::CustomEvent() : 
    QEvent(registeredType(-1)) 
{
}

CustomEvent::CustomEvent(int typ) : 
    QEvent(registeredType(typ)) 
{
}

CustomEvent::CustomEvent(QEvent * event):
    QEvent(event->type())
{
    // TODO: Implement data handling for QEvent subclasses
}

void to_json(json& orJson, const CustomEvent& korObject)
{
    orJson["type"] = korObject.type();
    orJson["data"] = korObject.m_data;
}

void from_json(const json& korJson, CustomEvent& orObject)
{
    // Type must be set on instantiation, so it is not used here
    if (korJson.contains("data")) {
        orObject.m_data = korJson.at("data");
    }
}

QEvent::Type CustomEvent::registeredType(int type)
{
    QEvent::Type eventType;
    if (!Map::HasKey(s_registeredEventTypes, type)) {
        int generatedType = QEvent::registerEventType(type);
        eventType = static_cast<QEvent::Type>(generatedType);
        s_registeredEventTypes[type] = eventType;
    }
    else {
        eventType = s_registeredEventTypes[type];
    }
    return eventType;
}

tsl::robin_map<int, QEvent::Type> CustomEvent::s_registeredEventTypes = {};

}
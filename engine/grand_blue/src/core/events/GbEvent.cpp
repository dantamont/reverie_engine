#include "GbEvent.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
CustomEvent::CustomEvent() : 
    QEvent(registeredType(-1)) 
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CustomEvent::CustomEvent(int typ) : 
    QEvent(registeredType(typ)) 
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CustomEvent::CustomEvent(QEvent * event):
    QEvent(event->type())
{
    // TODO: Implement data handling for QEvent subclasses
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CustomEvent::asJson() const
{
    QJsonObject json;
    json.insert("type", type());
    json.insert("data", m_data);
    return json;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CustomEvent::loadFromJson(const QJsonValue & json)
{
    // Type must be set on instantiation, so it is not used here
    const QJsonObject& object = json.toObject();
    if (object.contains("data")) {
        m_data = object.value("data").toObject();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QEvent::Type CustomEvent::registeredType(int type)
{
    QEvent::Type eventType;
    if (!Map::HasKey(REGISTERED_TYPES, type)) {
        int generatedType = QEvent::registerEventType(type);
        eventType = static_cast<QEvent::Type>(generatedType);
        REGISTERED_TYPES[type] = eventType;
    }
    else {
        eventType = REGISTERED_TYPES[type];
    }
    return eventType;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<int, QEvent::Type> CustomEvent::REGISTERED_TYPES = {};

}
#include "GEvent.h"

namespace rev {
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
QJsonValue CustomEvent::asJson(const SerializationContext& context) const
{
    QJsonObject json;
    json.insert("type", type());
    json.insert("data", m_data);
    return json;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CustomEvent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    // Type must be set on instantiation, so it is not used here
    Q_UNUSED(context)
    const QJsonObject& object = json.toObject();
    if (object.contains("data")) {
        m_data = object.value("data").toObject();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<int, QEvent::Type> CustomEvent::s_registeredEventTypes = {};

}
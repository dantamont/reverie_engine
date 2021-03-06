#include "GAnimationManager.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include "../scene/GScenario.h"
#include "GAnimStateMachine.h"

#include <QApplication>


namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// AnimationManager
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationManager::AnimationManager(CoreEngine* engine):
    Manager(engine, "AnimationManager")
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationManager::~AnimationManager()
{
    clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationStateMachine * AnimationManager::getStateMachine(const GString & name)
{
    auto iter = std::find_if(m_stateMachines.begin(), m_stateMachines.end(),
        [name](AnimationStateMachine* machine) {
        return machine->getName() == name;
    });
    if (iter == m_stateMachines.end()) {
        throw("Error, state machine not found");
        return nullptr;
    }
    
    return *iter;
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationStateMachine * AnimationManager::getStateMachine(const Uuid & uuid)
{
    auto iter = std::find_if(m_stateMachines.begin(), m_stateMachines.end(),
        [uuid](AnimationStateMachine* machine) {
        return machine->getUuid() == uuid;
    });
    if (iter == m_stateMachines.end()) {
        return nullptr;
    }

    return *iter;
}
/////////////////////////////////////////////////////////////////////////////////////////////
AnimationStateMachine * AnimationManager::addStateMachine()
{
    m_stateMachines.push_back(new AnimationStateMachine());
    return m_stateMachines.back();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationManager::clear()
{
    for (AnimationStateMachine* machine : m_stateMachines) {
        delete machine;
    }
    m_stateMachines.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationManager::postConstruction()
{
    // Initialize signal/slot connections
    deferredSenderConnect("EventManager",
        SIGNAL(deleteThreadedProcess(const Uuid&)),
        SLOT(deleteThreadedProcess(const Uuid&)));
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue AnimationManager::asJson(const SerializationContext& context) const
{
    QJsonObject json;
    QJsonArray machines;
    for (AnimationStateMachine* machine : m_stateMachines) {
        machines.append(machine->asJson());
    }
    json.insert("stateMachines", machines);

    return json;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void AnimationManager::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);
    const QJsonObject& object = json.toObject();
    QJsonArray machines = object["stateMachines"].toArray();
    for (const QJsonValue& val : machines) {
        m_stateMachines.push_back(new AnimationStateMachine());
        m_stateMachines.back()->loadFromJson(val, { m_engine });
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
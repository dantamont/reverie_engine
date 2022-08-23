#include "core/animation/GAnimationManager.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/scene/GScenario.h"
#include "core/animation/GAnimStateMachine.h"

#include <QApplication>

namespace rev {


// AnimationManager

AnimationManager::AnimationManager(CoreEngine* engine):
    Manager(engine, "AnimationManager")
{
}

AnimationManager::~AnimationManager()
{
    clear();
}

AnimationStateMachine * AnimationManager::getStateMachine(const GString & name)
{
    auto iter = std::find_if(m_stateMachines.begin(), m_stateMachines.end(),
        [name](AnimationStateMachine* machine) {
        return machine->getName() == name;
    });
    if (iter == m_stateMachines.end()) {
        Logger::Throw("Error, state machine not found");
        return nullptr;
    }
    
    return *iter;
}

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

AnimationStateMachine * AnimationManager::addStateMachine()
{
    m_stateMachines.push_back(new AnimationStateMachine());
    return m_stateMachines.back();
}

void AnimationManager::clear()
{
    for (AnimationStateMachine* machine : m_stateMachines) {
        delete machine;
    }
    m_stateMachines.clear();
}

void AnimationManager::postConstruction()
{
    // Initialize signal/slot connections
    connect(m_engine->eventManager(),
        SIGNAL(deleteThreadedProcess(const Uuid&)),
        SLOT(deleteThreadedProcess(const Uuid&)));
}

void to_json(json& orJson, const AnimationManager& korObject)
{
    orJson["stateMachines"] = json::array();
    for (AnimationStateMachine* machine : korObject.m_stateMachines) {
        orJson["stateMachines"].push_back(*machine);
    }
}

void from_json(const json& korJson, AnimationManager& orObject)
{
    const json& machines = korJson["stateMachines"];
    for (const json& val : machines) {
        orObject.m_stateMachines.push_back(new AnimationStateMachine());
        val.get_to(*orObject.m_stateMachines.back());
    }
}




} // End namespaces
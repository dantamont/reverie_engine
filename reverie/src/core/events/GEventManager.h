/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_EVENT_MANAGER_H
#define GB_EVENT_MANAGER_H

// Qt
#include <QObject>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>

// Internal
#include "../GManager.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ResourceHandle;
class SimEvent;
class EventListener;
class CustomEvent;
class Model;
class Material;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class Event Manager
class EventManager: public Manager{
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    EventManager(CoreEngine* engine);
	~EventManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Override QT event handling
    virtual bool event(QEvent* ev) override;

    /// @brief Add an event to the event queue
    void addEvent(QEvent* ev);

    /// @brief Processes all events in the event queue
    void processEvents();

    /// @brief Add an event listener for the given type of event
    /// @details Type of event is specified because listener pointer is duplicated in the listener map
    /// for every type of event that it listens to
    void addListener(int eventType, EventListener* listener);

    /// @brief Remove an event listener
    void removeListener(EventListener* listener);

    /// @brief Whether or not there is an event listener with the specified UUID
    bool hasListener(QEvent::Type type, EventListener* listener, size_t& iterPos);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "EventManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::EventManager"; }
    /// @}

signals:

    // Scenario
    void deletedSceneObject(size_t id);
    void addedComponent(Uuid uuid, int componentType, size_t sceneObjectID);
    void deletedSceneObjectComponent(size_t sceneObjectId); // Component::ParentType
    void deletedSceneComponent(Uuid sceneId); // Component::ParentType
    
    // Processes
    void deleteThreadedProcess(const Uuid& process);
    //void pythonBehaviorIsAwake(const Uuid& behaviorWrapperUuid);

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Process an event sent to the event manager
    void processEvent(CustomEvent* event);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QMutex m_queueMutex;

    /// @brief List of events to be handled in the current loop
    std::vector<CustomEvent> m_eventQueue;

    /// @brief List of all event listeners
    /// @details Outer map is indexed by event type, and inner maps are indexed by the listener's UUID
    tsl::robin_map<QEvent::Type, std::vector<EventListener*>> m_eventListeners;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
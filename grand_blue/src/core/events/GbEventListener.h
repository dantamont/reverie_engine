/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_EVENT_LISTENERS_H
#define GB_EVENT_LISTENERS_H

#include <memory>
#include <vector>
#include <set>
#include "../../third_party/pythonqt/PythonQt.h"

// QT

// Internal
#include "GbEvent.h"
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class EventListener;
class PythonClassScript;
class CoreEngine;
class SceneObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class EventListener
/// @brief Custom event listener for simulation loop
class EventListener: public Object, public Serializable {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    EventListener(CoreEngine* engine);
    EventListener(CoreEngine* engine, const QJsonValue& json);
    ~EventListener();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Add an event type to listen for
    void addEventType(int type);

    /// @brief Check if action should be performed
    virtual bool testEvent(CustomEvent* ev);

    /// @brief Perform response to event
    virtual void perform(CustomEvent* ev);

    /// @brief Initialize this listener in python from the given listener script
    void initializeScript(const QString& filepath, const std::shared_ptr<SceneObject>& object);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "EventListener"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::EventListener"; }
    /// @}

private:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Obtain pointer to object
    inline std::shared_ptr<SceneObject> sceneObject() const {
        if (const std::shared_ptr<SceneObject>& object = m_sceneObject.lock()) {
            return object;
        }
        else {
            return nullptr;
        }
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Path to the python script for this listener
    QString m_path;

    /// @brief Weak pointer to the scene object that owns this listener
    std::weak_ptr<SceneObject> m_sceneObject;

    /// @brief The python script for this listener
    std::shared_ptr<PythonClassScript> m_script;

    /// @brief The instantiation of the class from the script corresponding to this listener
    PythonQtObjectPtr m_pythonListener;

    /// @brief Event types accepted by this listener
    std::set<int> m_eventTypes;

    /// @}
};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
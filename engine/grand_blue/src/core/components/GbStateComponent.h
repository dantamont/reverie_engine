#ifndef GB_STATE_COMPONENT
#define GB_STATE_COMPONENT
// See:
// https://doc.qt.io/qt-5/qtwidgets-statemachine-pingpong-example.html
// https://doc.qt.io/qt-5/qtwidgets-statemachine-eventtransitions-example.html
// https://doc.qt.io/qt-5/statemachine-api.html
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>
#include <QStateMachine>

// Project
#include "GbComponent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EventListener;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class StateComponent
class StateComponent: public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    StateComponent();
    StateComponent(const std::shared_ptr<SceneObject>& object);
    ~StateComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return -1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "StateComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::StateComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The state machine
    QStateMachine* m_stateMachine;


    /// @}


};
Q_DECLARE_METATYPE(StateComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

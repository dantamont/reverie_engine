#ifndef GB_SERVICE_H 
#define GB_SERVICE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External
#include <QObject>
#include <QtWidgets>

// Project
#include "../../core/GObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AbstractService
///    @brief An abstract base class for all services and tools

class AbstractService: public rev::Object, public Nameable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    /// @brief Base class for all Services and Tools managed by the ServiceManager.
    AbstractService(const QString& name, bool deferPostConstruction = false);
    ~AbstractService();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Schedule a Qt Signal/Slot connection to a possibly unloaded sender.
    bool deferredSenderConnect(const char* sender_name, const char* sender_signal, const char* receiver_slot);
    bool deferredSenderConnect(const QString& sender_name, const char* sender_signal, const char* receiver_slot);

    /// @brief Returns True if this AbstractService represents a service
    virtual bool isService() const = 0;

    /// @brief Returns True if this AbstractService represents a tool
    virtual bool isTool() const = 0;

    /// @brief Called after Constructor completed
    /// @note Make deferred connection calls here
    virtual void postConstruction() { m_postConstructionDone = true; };

    /// @brief Get the Service with the given name
    static const AbstractService& getService(const QString& name);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB Object overrides
    /// @{
    virtual const char* className() const override { return "AbstractService"; }
    virtual const char* namespaceName() const override { return "rev::View::AbstractService"; }
    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Flag that post-construction is done
    bool m_postConstructionDone;

    /// @brief Flag to defer post-construction (and perform manually)
    bool m_deferPostConstruction;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Service
/// @brief Defines a QObject based Service
class Service: public QObject, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    /// @brief Create a Service with the given name and optional parent
    /// @note name must be unique
    Service(const QString& name, QObject* parent = nullptr, bool deferPostConstruction = false);

    /// @brief Create a Service with the given name and optional parent
    /// @note name must be unique
    Service(const char* name, QObject* parent = nullptr, bool deferPostConstruction = false);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Returns True for a Service and its subclasses
    virtual bool isService() const override  { return true; }

    /// @brief Returns False for a Service and its subclasses
    virtual bool isTool() const override  { return false; }

    /// @brief Returns information about every defined Qt Signal and Slot
    QString signalSlotInfo() const;
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name QObject overrides
    /// @{
    /// @brief Overrides QObject::event
    virtual bool event(QEvent* event) override;
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB Object overrides
    /// @{
    virtual const char* className() const override { return "Service"; }
    virtual const char* namespaceName() const override { return "rev::View::Service"; }
    /// @}
private:
    /// Private Methods
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // gb

#endif // GB_SERVICE_H 
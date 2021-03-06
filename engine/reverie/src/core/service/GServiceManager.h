#ifndef GB_SERVICE_MANAGER_H 
#define GB_SERVICE_MANAGER_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <memory>

// Qt
#include <QtWidgets>

// Project
#include "../../core/GObject.h"
#include "../../core/containers/GContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AbstractService;
class Service;

namespace View {
class Tool;
class Tool;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @struct DeferredConnection
/// @brief Represents a connection to be made after the creation of either the sender or receiver
class DeferredConnection {
public:
    //-----------------------------------------------------------------------------------------------------------------
    // Constructors and Destructors
    DeferredConnection(const GString& sender_name,
        const GString& sender_signal,
        const GString& receiver_name,
        const GString& receiver_method);
    //-----------------------------------------------------------------------------------------------------------------
    // Properties
    inline const GString& senderName() const { return m_senderName; }
    inline const GString& senderSignal() const { return m_senderSignal; }
    inline const GString& receiverName() const { return m_receiverName; }
    inline const GString& receiverMethod() const { return m_receiverMethod; }

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB Object overrides
    /// @{
    const char* className() { return "DeferredConnection"; }
    const char* namespaceName() { return "rev::DeferredConnection"; }
    /// @}

private:
    GString m_senderName;
    GString m_senderSignal;
    GString m_receiverName;
    GString m_receiverMethod;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class ServiceManager
    @brief Singleton that manages Service and Tool instances
*/
class ServiceManager: public QObject, public rev::Object {
    Q_OBJECT
    friend class AbstractService;
    friend class Service;
    friend class View::Tool;
	friend class View::Tool;
public:
    /// @name Class Name and Namespace
    /// @{
    const char* className() { return "ServiceManager"; }
    const char* namespaceName() { return "rev::View::ServiceManager"; }
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{
    /** @brief Get the one and only instance of the ServiceManager
        @return A reference to the singleton ServiceManager
    */
    static ServiceManager& getInstance();
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @brief Get the custom QEvent Type for post construction
        @details This custom event is posted during construction of Service and ServiceUi.  The event is 
            automatically handled by these classes to call the postConstruction allowing developers to do any
            additional initialization after the Service or ServiceUI has been constructed.
        @return The custom QEvent type
    */
    inline QEvent::Type postConstructionEventType() { return m_postConstructionEventType; }
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{
    /** @brief Schedule a Qt Signal/Slot connection to a possibly unloaded sender.
        @details Used when a service, the receiver, desires to connect to a signal of a sender and the sender may 
            not be available.  When the sender becomes available, the senders signal will be automatically connected 
            to the receiving slot/signal.
    */
    bool addDeferredSenderConnection(const QString& sender_name, const QString& sender_signal,
                                       const QString& receiver_name,const QString& receiver_method);

    /** @brief Returns True if a service or tool of the given name is managed by the Service Manager
        @param[in] name The name of the desired service.
        @return True if the service or tool is known to the service manater
    */
    bool hasService(const QString& name) const;

    /** @brief Get a reference to an existing AbstractService with the given name.
        @details Use the hasServiceOrTool method to check if an service or tool exists.
    */
    AbstractService& getService(const QString& name);
    /// @}

signals:
    void prepareToExit();
    void serviceAdded(const AbstractService&);

protected:
     /** @brief Add a service subclass of AbstractService to be managed by the Service Manager
        @details This method is automatically called in Service::event just before the postConstruction
            call is made.  The Service Manager will take ownership of the given AbstractService pointer.
    */
    bool addService(AbstractService* service);

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{
    ServiceManager();
    virtual ~ServiceManager() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Deletions
    /// @{
    // Delete move assignment/copy operators
    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;
    ServiceManager(ServiceManager&&) = delete;
    ServiceManager& operator=(ServiceManager&&) = delete;
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Create an error message for the connection
    void createErrorMessage(const QString& senderName, 
        const QString& signal, 
        const QString& receiverName,
        const QString& slot);

    void connectDeferred(AbstractService* sending_service);
    void removeService(AbstractService* service);
    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{
    QEvent::Type m_postConstructionEventType;
    tsl::robin_map<GString, AbstractService*> m_serviceMap;
    std::vector<DeferredConnection> m_deferredSenderConnections;
    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // rev

#endif // GB_SERVICE_MANAGER_H 

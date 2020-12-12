/** @file GbServiceManager.cpp
    @brief Defines the ServiceManager class
    @author Ryan Dougherty, Tad Gielow, and Dante Tufano
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbServiceManager.h"

// Standard Includes

// External
#include <QEvent>

// Internal
#include "GbService.h"
#include "../../view/base/GbTool.h"
#include "../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Deferred Connection
DeferredConnection::DeferredConnection(const GString& sender_name,
    const GString& sender_signal,
    const GString& receiver_name,
    const GString& receiver_method):
    m_senderName(sender_name),
    m_senderSignal(sender_signal),
    m_receiverName(receiver_name),
    m_receiverMethod(receiver_method){
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ServiceManager
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ServiceManager& ServiceManager::getInstance()
{
    // C++11 guarantees that the following static variable "instance" will be initialized in a thread-safe way.
    // Unfortunately, Visual Studio 2013 does not implement that part of the C++ standard. Given that the main 
    // usage of this class is to set up the ServiceManager in the main thread very early in the application
    // start up.  It that caveat is followed, everything should be fine. -tg
    static ServiceManager instance;
    return instance;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceManager::ServiceManager()
{
    // Register the postConstructionEvent type
    int event_type = QEvent::registerEventType();
    m_postConstructionEventType = (QEvent::Type)event_type;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceManager::~ServiceManager()
{
    m_serviceMap.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ServiceManager::addDeferredSenderConnection(
    const QString& sender_name, const QString& sender_signal,
    const QString& receiver_name, const QString& receiver_method)
{
	bool result = false;
    bool receiverExists = false;
    bool senderExists = false;
	QString errorMsg;

	// The Receiver is usually the one making this call(and therefore usually exists) to connect
	// the Sender's Signal to the Receiver's slot.  
    AbstractService* abstractReceiver = nullptr;
    AbstractService* abstractSender = nullptr;
	if (m_serviceMap.count(receiver_name)) {
		// The receiver is known
		abstractReceiver = m_serviceMap[receiver_name];
		receiverExists = true;
		if (m_serviceMap.count(sender_name)) {
			// The sender is known.
			abstractSender = m_serviceMap[sender_name];
			senderExists = true;
			if (abstractReceiver->isService()) {
				// Receiver is a Service
				Service* receiverService = static_cast<Service*>(abstractReceiver);
				if (abstractSender->isService()) {
					// Sender is a Service
					Service* sender_service = static_cast<Service*>(abstractSender);
					result = receiverService->connect(sender_service,
                        sender_signal.toStdString().c_str(),
                        receiver_method.toStdString().c_str());
				} 
                else if (abstractSender->isTool()) {
					// Sender is a Tool
					View::Tool* sender_tool = static_cast<View::Tool*>(abstractSender);
					result = receiverService->connect(sender_tool,
                        sender_signal.toStdString().c_str(),
                        receiver_method.toStdString().c_str());
				}
			} 
            else if (abstractReceiver->isTool()) {
				// Receiver is a Tool
				View::Tool* receiverTool = static_cast<View::Tool*>(abstractReceiver);
				if (abstractSender->isService()) {
					// Sender is a Service
					Service* sender_service = static_cast<Service*>(abstractSender);
					result = receiverTool->connect(sender_service,
                        sender_signal.toStdString().c_str(), 
                        receiver_method.toStdString().c_str());
				} 
                else if (abstractSender->isTool()) {
					// Sender is a ToolPaneWidget
					View::Tool* sender_tool = static_cast<View::Tool*>(abstractSender);
					result = receiverTool->connect(sender_tool,
                        sender_signal.toStdString().c_str(),
                        receiver_method.toStdString().c_str());
				}
			} 
            else {
                throw("Error, unrecognized service type");
			}

            if (!result) {
                // Receiver and sender both exist, but the connection was not made
                errorMsg.append("Unable to make connection, check for a mismatch between the signal and the slot.");
            }
		} 
        else {
            // The sender was not found, so cache connection info for when sender is constructed
			Vec::EmplaceBack(m_deferredSenderConnections, sender_name, sender_signal, receiver_name, receiver_method);
		}

	} 
    else {
        // If no receiver found
		errorMsg.append("The receiving service/tool is not known by the Service Manager");
		result = false;
	}

#ifdef DEBUG_MODE
    // Log error
	if (!errorMsg.isEmpty()) {
        logError(errorMsg);
        createErrorMessage(sender_name, sender_signal, receiver_name, receiver_method);
	}
#endif
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ServiceManager::addService(AbstractService* service)
{
    bool result = false;
    if (m_serviceMap.count(service->getName())) {
        GString msg(R"(addService: Service ")");
        msg += service->getName();
        msg += R"(" already exists)";
        logError(msg.c_str());
    } else {
        m_serviceMap.emplace(service->getName(), service);
        result = true;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ServiceManager::hasService(const QString& name) const
{
    bool result = false;
    if (m_serviceMap.count(name)) {
        result = true;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AbstractService& ServiceManager::getService(const QString& name)
{
    if (hasService(name)) {
        AbstractService* service = m_serviceMap.at(name);
        return *service;
    } else {
		QString msg(R"(serviceWithName: the service named ")");
		msg += name;
		msg += R"(" is not known by the Service Manager.)";
		logError(msg);
        throw std::out_of_range("ServiceManager:: unknown service or tool:" + name.toStdString());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::createErrorMessage(const QString& senderName,
    const QString& signal,
    const QString& receiverName,
    const QString& slot)
{
    QString msg("ServiceManager:: Unable to connect signal to slot:");
    msg.append("\n    Sender: ");
    msg.append(senderName);
    msg.append("\n    Signal: ");
    msg.append(signal);
    msg.append("\n    Receiver: ");
    msg.append(receiverName);
    msg.append("\n    Slot: ");
    msg.append(slot);
    msg.append("\n Check for an incompatability between the signal and the slot.");
    logError(msg);
    throw(msg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::connectDeferred(AbstractService* sending_service)
{
    // Iterate over the deferred connections list looking for an existing
    // service/tool that desire to connect to the sending service
    auto defIter = std::find_if(m_deferredSenderConnections.begin(),
        m_deferredSenderConnections.end(),
        [&](const DeferredConnection& connection) {
        return connection.senderName() == sending_service->getName();
    });

    // Return if no connections found
    if (defIter == m_deferredSenderConnections.end()) {
        return;
    }

    const DeferredConnection& connection = *defIter;

    // Found a match for the sender
    AbstractService* abstractSender = m_serviceMap[connection.senderName()];
    AbstractService* abstractReceiver = m_serviceMap[connection.receiverName()];

    bool connected = false;
    if (abstractSender->isTool()) {
        // Sender is a Tool
        View::Tool* senderTool = static_cast<View::Tool*>(abstractSender);
        if (abstractReceiver->isTool()) {
            // Receiver is a Tool
            View::Tool* receiverTool = static_cast<View::Tool*>(abstractReceiver);
            connected = receiverTool->connect(senderTool, connection.senderSignal().c_str(),
                connection.receiverMethod().c_str());
        } 
        else {
            // Receiver is a Service
            Service* receiverService = static_cast<Service*>(abstractReceiver);
            connected = receiverService->connect(senderTool, connection.senderSignal().c_str(),
                connection.receiverMethod().c_str());
        }
    } 
    else {
        // Sender is a Service
        Service* senderService = static_cast<Service*>(abstractSender);
        if (abstractReceiver->isTool()) {
            // Receiver is a Tool
            View::Tool* receiver_service_ui = static_cast<View::Tool*>(abstractReceiver);
            connected = receiver_service_ui->connect(senderService, connection.senderSignal().c_str(),
                connection.receiverMethod().c_str());
        } 
        else {
            // Receiver is a Service
            Service* receiver_service = static_cast<Service*>(abstractReceiver);
            connected = receiver_service->connect(senderService, connection.senderSignal().c_str(),
                connection.receiverMethod().c_str());
        }
    }

#ifdef DEBUG_MODE
    if (!connected) {
        // Throw error if connection failed
        createErrorMessage(abstractSender->getName(), connection.senderSignal(), abstractReceiver->getName(), connection.receiverMethod());
    }
#endif
  
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServiceManager::removeService(AbstractService* service)
{
    tsl::robin_map<QString, AbstractService*>::iterator iVas;
    iVas = m_serviceMap.find(service->getName());
    if (iVas != m_serviceMap.end()) {
        m_serviceMap.erase(iVas);
    } 
    else {
        QString msg("ServiceManager:: removeService: Service " 
            + service->getName() + " was not found by Service Manager");
        logError(msg);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // Gb
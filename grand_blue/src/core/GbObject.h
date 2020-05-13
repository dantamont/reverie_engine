/** @file GbObject.h
    @copyright This program is free software: you can redistribute it and/or modify it under the terms of the
        GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the
        License, or(at your option) any later version.
        This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
        the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
        Public License for more details at <http://www.gnu.org/licenses/>.
    @brief Defines a base class used by many other classes defined in the Grand Blue library.
    @details Provides methods to query class name, namespace and logging for errors, warnings, info and debugging
        messages.
    @author Tad Gielow
    @author Dante Tufano
*/

#ifndef GB_OBJECT_BASE_H
#define GB_OBJECT_BASE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QJsonObject>

// Project
#include "GbLogger.h"
#include "encoding\GbUUID.h"
#include "mixins\GbNameable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QTreeWidgetItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class Object
    @brief  A base class for most classes
    @details Provides methods to query class name and namespace, logging methods for errors, warnings, 
        info and debug messages.
*/
class Object: public Nameable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    Object(const Uuid& uuid) :
        m_uuid(uuid) {
    }
    Object() :
        m_uuid(true) {
    }
    Object(const QString& name, NameMode mode = kCaseSensitive):
        Nameable(name, mode){
    }
    virtual ~Object() = default;
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
        @details Every subclass should redefine/override this property to
            return its name
    */
    virtual const char* className() const { return "Object"; }

    /** @property namespaceName
        @brief The full namespace for this class
        @details Every subclass should redefine/override this property to
            return its full namespace.  The built in logging methods will
            use this value for the message category
    */
    virtual const char* namespaceName() const { return "Gb::Object"; }

    /// @property UUID
    /// @brief Uuid of this object
    inline const Uuid& getUuid() const { return m_uuid; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Viewing methods
    /// @brief Methods for creating the visual representation of this object
    /// @{

    virtual QTreeWidgetItem* asTreeWidgetItem() const { return nullptr; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /** @name Logging Methods
        @details Convenience methods to send log messages to the Logger
        @{
    */

    /// @brief Log message from any thread via a log event
    void logThreadMessage(const QString& message, LogLevel logLevel = LogLevel::Info);

    /** @brief Output a message to a log
        @param[in] level The numeric level for the message.  The higher the
            value, the more severe the issue.
        @param[in] category Helps define where the message comes from.
        @param[in] message The textual message for the log.
    */
    void logMessage(LogLevel level, const char* category, const char* message) const;

    /** @brief Output a message to a log
        @param[in] level The numeric level for the message.  The higher the
            value, the more severe the issue.
        @param[in] category Helps define where the message comes from.
        @param[in] message The textual message for the log.
    */
    inline void logMessage(LogLevel level, std::string& category, std::string& message) const
        { logMessage(level, category.c_str(), message.c_str()); }

    /** @brief Log a Critical message and terminate the app
        @param[in] message The textual message for the log.
    */
    inline void logCritical(const std::string& message) const
        {
            logMessage(LogLevel::Critical, namespaceName(), message.c_str());
            abort();
        }
    /** @brief Log a Critical message and terminate the app
        @param[in] message The textual message for the log.
    */
    inline void logCritical(const char* message) const
        { logMessage(LogLevel::Critical, namespaceName(), message); }

#ifdef QT_CORE_LIB
    /** @brief Log a Critical message.
        @param[in] message The textual message for the log.
        @note Only available when Qt is included in build.
    */
    inline void logCritical(const QString& message) const
        { logMessage(LogLevel::Critical, namespaceName(), message.toUtf8().constData()); }
#endif

    /** @brief Log a Debugging message.
        @param[in] message The textual message for the log.
    */
    inline void logDebug(const std::string& message) const
        { logMessage(LogLevel::Debug, namespaceName(), message.c_str());}

    /** @brief Log a Debugging message.
        @param[in] message The textual message for the log.
    */
    inline void logDebug(const char* message) const
	    { logMessage(LogLevel::Debug, namespaceName(), message);}

#ifdef QT_CORE_LIB
    /** @brief Log a Debugging message.
        @param[in] message The textual message for the log.
        @note Only available when Qt is included in build.
    */
    inline void logDebug(const QString& message) const
        { logMessage(LogLevel::Debug, namespaceName(), message.toUtf8().constData()); }
#endif

    /** @brief Log a Error message.
        @param[in] message The textual message for the log.
    */
    inline void logError(const char* message) const
        { logMessage(LogLevel::Error, namespaceName(), message);}

    /** @brief Log a Error message.
        @param[in] message The textual message for the log.
    */
    inline void logError(const std::string& message) const
        { logMessage(LogLevel::Error, namespaceName(), message.c_str()); }

#ifdef QT_CORE_LIB
    /** @brief Log a Error message.
        @param[in] message The textual message for the log.
        @note Only available when Qt is included in build.
    */
    inline void logError(const QString& message) const
        {logMessage(LogLevel::Debug, namespaceName(), message.toUtf8().constData()); }
#endif

    /** @brief Log an Informational message.
        @param[in] message The textual message for the log.
    */
    inline void logInfo(const char* message) const 
        { logMessage(LogLevel::Info, namespaceName(), message); }

    /** @brief Log an Informational message.
        @param[in] message The textual message for the log.
    */
    inline void logInfo(const std::string& message) const
    { logMessage(LogLevel::Info, namespaceName(), message.c_str()); }

#ifdef QT_CORE_LIB
    /** @brief Log an Informational message.
        @param[in] message The textual message for the log.
        @note Only available when Qt is included in build.
    */
    inline void logInfo(const QString& message) const
        { logMessage(LogLevel::Info, namespaceName(), message.toUtf8().constData()); }
#endif

    /** @brief Log a Warning message.
        @param[in] message The textual message for the log.
    */
    inline void logWarning(const char* message) const
        { logMessage(LogLevel::Warning, namespaceName(), message); }

    /** @brief Log a Warning message.
        @param[in] message The textual message for the log.
    */
    inline void logWarning(const std::string& message) const
        { logMessage(LogLevel::Warning, namespaceName(), message.c_str()); }

#ifdef QT_CORE_LIB
    /** @brief Log a Warning message.
        @param[in] message The textual message for the log.
        @note Only available when Qt is included in build.
    */
    inline void logWarning(const QString& message) const
        { logMessage(LogLevel::Warning, namespaceName(), message.toUtf8().constData()); }
#endif
    /** @brief Create a string representation of this Object.
        @details Return a brief string description of the class instance that is useful for debugging purposes.
        The default implementation, where include_type is set to true, outputs the class name of the object and 
        its address: Object(0x00012345).  When the include_type is false, the output is just the address:
        0x00012345.

        This method supports the operator<< methods for ostream and QDebug. Subclasses should override to output
        custom information.

        @param[in] include_type Optional flag indicating whether the class name should be included in the 
            returned string.  Default is true.
    */
    virtual std::string toStdString(bool include_type=true) const;
    virtual QString asQString(bool include_type = true) const;
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
#ifdef QT_CORE_LIB
    /** @brief Output debug information for the class instance via Qt's qDebug() stream.
        @details Using the qDebug() function call, developers can as code follows:

            Object myObject;
            qDebug() << myObject;

        This will call toStdString for the subclass to add to the QDebug stream. There should be no need for
        subclasses to override this method as most everything needed to be done can be accomplished
        by overriding toStdString.
            
        @param[in] qdebug The QDebug stream from the qDebug() function call.
        @param[in] object A reference to a Object, or subclass.
        @note Only available when Qt is included in build.
    */
    friend QDebug operator<<(QDebug qdebug, const Object& object);
#endif
    /** @brief Output debug information for the class instance via std::ostream.
        @details This will call toStdString for the subclass to add to the std::ostream. There should be no need for
        subclasses to override this method as most everything needed to be done can be accomplished by overriding
        toStdString.
    */
    friend std::ostream& operator<<(std::ostream& os, const Object& object);
    /// @}
protected:

    /// @brief Unique identifier for this object
    Gb::Uuid m_uuid;

};
Q_DECLARE_METATYPE(std::shared_ptr<Object>)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

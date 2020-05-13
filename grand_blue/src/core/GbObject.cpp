///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GbObject.h"

// Standard Includes
#include <sstream>

// External
#ifdef QT_CORE_LIB
#include <QDebug>
#include <QApplication>
#endif
// Project

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Object
//---------------------------------------------------------------------------------------------------------------------
// Logging Methods
//---------------------------------------------------------------------------------------------------------------------
void Object::logThreadMessage(const QString & message, LogLevel logLevel)
{
    Q_UNUSED(message)
    Q_UNUSED(logLevel)
    throw("Error, not implemented, need pointer to engine");
//#ifdef QT_CORE_LIB
//    QApplication::postEvent(m_engine,
//        new LogEvent(namespaceName(), message, Process::getThreadID(), logLevel));
//
//#endif
}
//---------------------------------------------------------------------------------------------------------------------
void Object::logMessage(LogLevel level, const char* category, const char* message) const
{
	auto& lg = Gb::Logger::getInstance();
	lg.logMessage(level, category, message);
}
//---------------------------------------------------------------------------------------------------------------------
std::string Object::toStdString(bool include_type) const
{
    std::stringstream ss;
    if (include_type) ss << className() << "(";
    ss << "0x";
    ss.width(8);
    ss.fill('0');
    ss << std::hex << (long)this;
    if (include_type) ss << ")";
    return ss.str();
}
//---------------------------------------------------------------------------------------------------------------------
QString Object::asQString(bool include_type) const
{
    return QString::fromStdString(toStdString(include_type));
}
//---------------------------------------------------------------------------------------------------------------------
// Operators
//---------------------------------------------------------------------------------------------------------------------
QDebug operator<<(QDebug qdebug, const Object& visobject)
{
    return qdebug << visobject.toStdString(true).c_str();
}
//---------------------------------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Object& visobject)
{
    os << visobject.toStdString(true);
    return os;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

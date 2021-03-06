///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GLogger.h"

// Standard Includes
#include <assert.h>
#include <iostream>
#include <fstream>

// External
#include <windows.h>

// Project
#include "../view/logging/GConsoleTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Log Record
LogRecord::LogRecord():
    m_level(LogLevel::Unset)
{}
LogRecord::~LogRecord()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Abstract log handler
AbstractLogHandler::AbstractLogHandler() :
    m_level(LogLevel::Info)
{}

AbstractLogHandler::AbstractLogHandler(LogLevel level) :
    m_level(level)
{
}

AbstractLogHandler::~AbstractLogHandler()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler
//---------------------------------------------------------------------------------------------------------------------
// Constructors and Destructors
//---------------------------------------------------------------------------------------------------------------------
FileLogHandler::FileLogHandler(const GString& filename) :
    AbstractLogHandler(LogLevel::Debug),
    m_filename(filename)
{
    m_ofs.open((const char*)m_filename, std::ofstream::out | std::ofstream::trunc);
}
//---------------------------------------------------------------------------------------------------------------------
FileLogHandler::FileLogHandler(LogLevel level, const GString& filename) :
    AbstractLogHandler(level),
    m_filename(filename)
{
    m_ofs.open((const char*)m_filename, std::ofstream::out | std::ofstream::trunc);
}
//---------------------------------------------------------------------------------------------------------------------
FileLogHandler::~FileLogHandler()
{
    if (isOpen()) {
        m_ofs.close();
    }
}
//---------------------------------------------------------------------------------------------------------------------
// Public Methods
//---------------------------------------------------------------------------------------------------------------------
bool FileLogHandler::isOpen() const
{
    return m_ofs.is_open();
}
//---------------------------------------------------------------------------------------------------------------------
void FileLogHandler::output(LogRecord& log_record)
{
	if (log_record.level() != m_mostRecentRecord.level() || 
        log_record.category() != m_mostRecentRecord.category() ||
		abs(log_record.timestamp().msecsTo(m_mostRecentRecord.timestamp())) > 2) {
        // Output
        // | <level number> | <timestamp> | <category>
        // <message>
        m_ofs << "| " << (unsigned int)log_record.level() 
            << " | " << log_record.timestamp().toString("MMM dd, yyyy @ hh:mm:ss.zzz").toStdString() 
            << " | " << log_record.category()
            << " | \n" << log_record.message() << "\n";
    } else {
        // Output
        // <message>
        m_ofs << log_record.message() << "\n";
    }
    m_ofs.flush();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StandardOutLogHandler
//---------------------------------------------------------------------------------------------------------------------
// Public Methods
//---------------------------------------------------------------------------------------------------------------------
void StandardOutHandler::output(LogRecord& log_record)
{
	if (log_record.level() != m_mostRecentRecord.level() || log_record.category() != m_mostRecentRecord.category() ||
		abs(log_record.timestamp().msecsTo(m_mostRecentRecord.timestamp())) > 2) {
        const GString* level_name = Logger::levelName(log_record.level());
        if (level_name) {
            std::cout << *level_name;
        } else {
            std::cout << (unsigned int)log_record.level();
        }
        std::cout << "  " << log_record.timestamp().toString("MMM dd, yyyy @ hh:mm:ss.zzz").toStdString()
            << "   " << log_record.category() << "\n" << log_record.message() << "\n";
    } else {
        std::cout  << log_record.message() << "\n";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VisVSOutLogHandler

VSConsoleLogHandler::VSConsoleLogHandler() :
    AbstractLogHandler()
{
}
//---------------------------------------------------------------------------------------------------------------------
VSConsoleLogHandler::VSConsoleLogHandler(LogLevel level) :
    AbstractLogHandler(level)
{
}
//---------------------------------------------------------------------------------------------------------------------
VSConsoleLogHandler::~VSConsoleLogHandler()
{
}

//---------------------------------------------------------------------------------------------------------------------
// Public Methods
//---------------------------------------------------------------------------------------------------------------------
void VSConsoleLogHandler::output(LogRecord& log_record)
{
	if (log_record.level() != m_mostRecentRecord.level() || log_record.category() != m_mostRecentRecord.category() ||
		abs(log_record.timestamp().msecsTo(m_mostRecentRecord.timestamp())) > 2) {
        QString outputStr = QStringLiteral("%1  %2  %3\n%4\n");
        QString& outputRef = outputStr;
        const GString* level_name = Logger::levelName(log_record.level());
        if (level_name) {
            outputRef = outputStr.arg(level_name->c_str());
        } else {
            outputRef = outputStr.arg((unsigned int)log_record.level());
        }
        outputRef = outputRef.arg(log_record.timestamp().toString("MMM dd, yyyy @ hh:mm:ss.zzz"))
            .arg((const char*)log_record.category()).arg((const char*)log_record.message());
        OutputDebugString(reinterpret_cast<LPCWSTR>(outputRef.utf16()));
    } else {
        QString outputStr = QStringLiteral("%1\n");
        QString& outputRef = outputStr;
        outputRef = outputRef.arg((const char*)log_record.message());
        OutputDebugString(reinterpret_cast<LPCWSTR>(outputRef.utf16()));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logger
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

tsl::robin_map<LogLevel, GString> Logger::m_level_names;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogInfo(const char * str, const char* cat)
{
    Logger& logger = getInstance();
    logger.logMessage(LogLevel::Info, cat, str);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogInfo(const GString & msg, const char * cat)
{
    LogInfo(msg.c_str(), cat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogWarning(const char * str, const char* cat)
{
    Logger& logger = getInstance();
    logger.logMessage(LogLevel::Warning, cat, str);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogWarning(const GString & msg, const char * cat)
{
    LogWarning(msg.c_str(), cat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogError(const char * str, const char* cat)
{
    Logger& logger = getInstance();
    logger.logMessage(LogLevel::Error, cat, str);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogError(const GString & msg, const char * cat)
{
    LogError(msg.c_str(), cat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogCritical(const char * str, const char* cat)
{
    Logger& logger = getInstance();
    logger.logMessage(LogLevel::Critical, cat, str);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::LogCritical(const GString & msg, const char * cat)
{
    LogCritical(msg.c_str(), cat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::FileLogHandler * Logger::getFileLogger(const GString & logFileName, const GString & loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::getInstance();

    // Create a logging handler to output to a file
    if (!logger.handlerWithName(loggerName)) {
        QString log_file_name = QStringLiteral("./logs/%1.log").arg(logFileName.c_str());
        GString log_file_str(log_file_name);

        auto* fileHandler = new rev::FileLogHandler(log_file_str);
        fileHandler->setLevel(logLevel);

        logger.addHandler(loggerName, fileHandler);

        return fileHandler;
    }
    else {
        return static_cast<rev::FileLogHandler*>(logger.handlerWithName(loggerName));
    }

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::VSConsoleLogHandler * Logger::getVSLogger(const GString & loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::getInstance();

    #ifndef NDEBUG
        // Create a logging handler to output to the Visual Studio Console
        if (!logger.handlerWithName(loggerName)) {
            auto* consoleLogHandler = new rev::VSConsoleLogHandler(logLevel);
            logger.addHandler(loggerName, consoleLogHandler);
            return consoleLogHandler;
        }
        else {
            return static_cast<rev::VSConsoleLogHandler*>(logger.handlerWithName(loggerName));
        }
    #endif
     
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::View::ConsoleTool * Logger::getConsoleTool(const GString & loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::getInstance();

    // Create a logging handler to output to the console tool
    if (!logger.handlerWithName(loggerName)) {
        auto* consoleTool = new rev::View::ConsoleTool(loggerName.c_str());
        consoleTool->setLevel(logLevel);
        logger.addHandler(consoleTool->getName(), consoleTool);
        consoleTool->show();

        return consoleTool;
    }
    else {
        return static_cast<rev::View::ConsoleTool*>(logger.handlerWithName(loggerName));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::StandardOutHandler* Logger::getStdOutHandler(const GString& loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::getInstance();

    // Create a logging handler to output to the console tool
    if (!logger.handlerWithName(loggerName)) {
        auto* handler = new StandardOutHandler();
        handler->setLevel(logLevel);
        logger.addHandler(loggerName, handler);
        return handler;
    }
    else {
        return static_cast<rev::StandardOutHandler*>(logger.handlerWithName(loggerName));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const GString* Logger::levelName(LogLevel level)
{
    const GString* result = nullptr;
    tsl::robin_map<LogLevel, GString>::const_iterator iLm;
    iLm = m_level_names.find(level);
    if (iLm != m_level_names.end()) {
        result = &((*iLm).second);
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::addLevelName(LogLevel level, const GString& level_name)
{
    m_level_names.emplace(level, level_name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger() :
    m_level(LogLevel::Info)
{
    addLevelName(LogLevel::Critical, "Critical");
    addLevelName(LogLevel::Error, "Error");
    addLevelName(LogLevel::Warning, "Warning");
    addLevelName(LogLevel::Info, "Info");
    addLevelName(LogLevel::Debug, "Debug");
//#ifdef QT_CORE_LIB
//    qInstallMessageHandler(loggingHandler);
//#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Logger::~Logger()
{
    // Clear the handlers to make sure their destructors get called.
    m_handlers.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::logMessage(LogLevel level,
    const char* category,
    const char* msg)
{
    QMutexLocker locker(&m_output_mutex);

    unsigned int intLevel = (unsigned int)level;
    if (intLevel >= (unsigned int)m_level) {
        QDateTime dt = QDateTime::currentDateTime();
        m_log_record.setLevel(level);
        m_log_record.setCategory(category);
        m_log_record.setMessage(msg);
        m_log_record.setTimestamp(std::move(dt));
        tsl::robin_map<GString, AbstractLogHandler*>::const_iterator iHandler;
        for (iHandler = m_handlers.cbegin(); iHandler != m_handlers.cend(); ++iHandler) {
            if (level >= (*iHandler).second->level()) {
                (*iHandler).second->output(m_log_record);
            }
        }
    }
    // Uncomment to throw error on critical message
    //if (intLevel >= (unsigned int) LogLevel::Critical) {
        //assert(0);
    //}

    //m_output_mutex.unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Logger::addHandler(const GString& name, AbstractLogHandler *handler)
{
    m_handlers.insert(
        std::pair<GString, AbstractLogHandler*>(name, handler));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AbstractLogHandler* Logger::handlerWithName(const GString& name)
{
    AbstractLogHandler* result = nullptr;
    tsl::robin_map<GString, AbstractLogHandler*>::iterator iH = m_handlers.find(name);
    if (iH != m_handlers.end()) {
        result = (*iH).second;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Logger::removeHandler(const GString& name)
{
    bool result = false;
    tsl::robin_map<GString, AbstractLogHandler*>::iterator iH = m_handlers.find(name);
    if (iH != m_handlers.end()) {
        m_handlers.erase(iH);
        result = true;
    }
    return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function Implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#ifdef QT_CORE_LIB
//void loggingHandler(QtMsgType msg_type, const QMessageLogContext& context, const QString& msg)
//{
//    Q_UNUSED(context);
//    Logger& logger = Logger::getInstance();
//    switch (msg_type) {
//        case QtDebugMsg:
//            logger.logMessage(LogLevel::Debug, "qDebug", msg.toStdString().c_str());
//            break;
//        case QtInfoMsg:
//            logger.logMessage(LogLevel::Info, "qInfo", msg.toStdString().c_str());
//            break;
//        case QtWarningMsg:
//            logger.logMessage(LogLevel::Warning, "qWarning", msg.toStdString().c_str());
//            break;
//        case QtCriticalMsg:
//            logger.logMessage(LogLevel::Critical,  "qCritical", msg.toStdString().c_str());
//            break;
//        case QtFatalMsg:
//            logger.logMessage(LogLevel::Critical, "qFatal", msg.toStdString().c_str());;
//            break;
//    }
//}
//#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespaces
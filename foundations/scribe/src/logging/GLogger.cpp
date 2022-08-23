#include "logging/GLogger.h"

#include <assert.h>
#include <iostream>
#include <fstream>

#include "fortress/types/GStringView.h"

namespace rev {


// Log Record
LogRecord::LogRecord():
    m_level(LogLevel::Unset)
{}
LogRecord::~LogRecord()
{
}


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

FileLogHandler::FileLogHandler(const GString& filename) :
    AbstractLogHandler(LogLevel::Debug),
    m_filename(filename)
{
    m_ofs.open((const char*)m_filename, std::ofstream::out | std::ofstream::trunc);
}

FileLogHandler::FileLogHandler(LogLevel level, const GString& filename) :
    AbstractLogHandler(level),
    m_filename(filename)
{
    m_ofs.open((const char*)m_filename, std::ofstream::out | std::ofstream::trunc);
}


FileLogHandler::~FileLogHandler()
{
    if (isOpen()) {
        m_ofs.close();
    }
}

bool FileLogHandler::isOpen() const
{
    return m_ofs.is_open();
}

void FileLogHandler::output(LogRecord& log_record)
{
	if (log_record.level() != m_mostRecentRecord.level() || 
        log_record.category() != m_mostRecentRecord.category() ||
		abs(log_record.timestamp().getTimeToMs(m_mostRecentRecord.timestamp())) > 2) {
        // Output
        // | <level number> | <timestamp> | <category>
        // <message>
        m_ofs << "| " << (unsigned int)log_record.level() 
            << " | " << log_record.timestamp().toString("MMM dd, yyyy @ hh:mm:ss.zzz")
            << " | " << log_record.category()
            << " | \n" << log_record.message() << "\n";
    } else {
        // Output
        // <message>
        m_ofs << log_record.message() << "\n";
    }
    m_ofs.flush();
}


void StandardOutHandler::output(LogRecord& log_record)
{
	if (log_record.level() != m_mostRecentRecord.level() || log_record.category() != m_mostRecentRecord.category() ||
		abs(log_record.timestamp().getTimeToMs(m_mostRecentRecord.timestamp())) > 2) {
        const GString* level_name = Logger::levelName(log_record.level());
        if (level_name) {
            std::cout << *level_name;
        } else {
            std::cout << (unsigned int)log_record.level();
        }
        std::cout << "  " << log_record.timestamp().toString("MMM dd, yyyy @ hh:mm:ss.zzz")
            << "   " << log_record.category() << "\n" << log_record.message() << "\n";
    } else {
        std::cout  << log_record.message() << "\n";
    }
}

tsl::robin_map<LogLevel, GString> Logger::m_levelNames;

void Logger::LogMessage(LogLevel level, const char* category, const char* message)
{
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Debug, category, message);
}

void Logger::LogDebug(const char* str, const char* cat)
{
#ifdef DEBUG_MODE
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Debug, cat, str);
#endif
}

void Logger::LogDebug(const std::string& msg, const char* cat)
{
    LogDebug(msg.c_str(), cat);
}

void Logger::LogDebug(const GString& msg, const char* cat)
{
    LogDebug(msg.c_str(), cat);
}

void Logger::LogInfo(const char * str, const char* cat)
{
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Info, cat, str);
}

void Logger::LogInfo(const GString & msg, const char * cat)
{
    LogInfo(msg.c_str(), cat);
}

void Logger::LogWarning(const char * str, const char* cat)
{
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Warning, cat, str);
}

void Logger::LogWarning(const GString & msg, const char * cat)
{
    LogWarning(msg.c_str(), cat);
}

void Logger::LogError(const char * str, const char* cat)
{
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Error, cat, str);
}

void Logger::LogError(const GString & msg, const char * cat)
{
    LogError(msg.c_str(), cat);
}

void Logger::LogCritical(const char * str, const char* cat)
{
    Logger& logger = Instance();
    logger.logMessage(LogLevel::Critical, cat, str);
}

void Logger::LogCritical(const GString & msg, const char * cat)
{
    LogCritical(msg.c_str(), cat);
}

GString Logger::LastException()
{
    return s_errorStrings.back();
}

void Logger::AddErrorString(const GString& string)
{
    s_errorStrings.push_back(string);
}

void Logger::Throw(const char* err)
{
    s_errorStrings.push_back(err);
    throw std::exception(err);
}
void Logger::Throw(const std::string& err)
{
    s_errorStrings.push_back(err.c_str());
    throw std::exception(err.c_str());
}
void Logger::Throw(const GString& err)
{
    s_errorStrings.push_back(err.c_str());
    throw std::exception(err.c_str());
}
void Logger::Throw(const std::exception& err)
{
    s_errorStrings.push_back(err.what());
    throw std::exception(err);
}

rev::FileLogHandler * Logger::getFileLogger(const GString & logFileName, const GString & loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::Instance();

    // Create a logging handler to output to a file
    if (!logger.handlerWithName(loggerName)) {
        GString logFilePath = GString::Format("./logs/%s.log", logFileName.c_str());

        auto* fileHandler = new rev::FileLogHandler(logFilePath);
        fileHandler->setLevel(logLevel);

        logger.addHandler(loggerName, fileHandler);

        return fileHandler;
    }
    else {
        return static_cast<rev::FileLogHandler*>(logger.handlerWithName(loggerName));
    }

    return nullptr;
}

rev::StandardOutHandler* Logger::getStdOutHandler(const GString& loggerName, LogLevel logLevel)
{
    // Create logger
    auto& logger = rev::Logger::Instance();

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

const GString* Logger::levelName(LogLevel level)
{
    const GString* result = nullptr;
    tsl::robin_map<LogLevel, GString>::const_iterator iLm;
    iLm = m_levelNames.find(level);
    if (iLm != m_levelNames.end()) {
        result = &((*iLm).second);
    }
    return result;
}

void Logger::addLevelName(LogLevel level, const GString& level_name)
{
    m_levelNames.emplace(level, level_name);
}


Logger::Logger() :
    m_level(LogLevel::Info)
{
    addLevelName(LogLevel::Critical, "Critical");
    addLevelName(LogLevel::Error, "Error");
    addLevelName(LogLevel::Warning, "Warning");
    addLevelName(LogLevel::Info, "Info");
    addLevelName(LogLevel::Debug, "Debug");
}

Logger::~Logger()
{
    // Clear the handlers to make sure their destructors get called.
    for (const auto& handlerPair : m_handlers) {
        AbstractLogHandler* handler = handlerPair.second;
        delete handler;
    }
    m_handlers.clear();
}

void Logger::logMessage(LogLevel level,
    const char* category,
    const char* msg)
{
    /// @fixme Sometimes, this crashes in setCategory on application close, in GString destructor
    static std::mutex s_outputMutex;
    std::unique_lock lock(s_outputMutex);

    unsigned int intLevel = (unsigned int)level;
    if (intLevel >= (unsigned int)m_level) {
        DateTime dt = DateTime::Now();
        m_logRecord.setLevel(level);
        m_logRecord.setCategory(category);
        m_logRecord.setMessage(msg);
        m_logRecord.setTimestamp(std::move(dt));
        tsl::robin_map<GString, AbstractLogHandler*>::const_iterator iHandler;
        for (iHandler = m_handlers.cbegin(); iHandler != m_handlers.cend(); ++iHandler) {
            if (level >= (*iHandler).second->level()) {
                (*iHandler).second->output(m_logRecord);
            }
        }
    }
}

void Logger::addHandler(const GString& name, AbstractLogHandler *handler)
{
    m_handlers.insert(std::pair<GString, AbstractLogHandler*>(name, handler));
    handler->onAddToLogger();
}

AbstractLogHandler* Logger::handlerWithName(const GString& name)
{
    AbstractLogHandler* result = nullptr;
    tsl::robin_map<GString, AbstractLogHandler*>::iterator iH = m_handlers.find(name);
    if (iH != m_handlers.end()) {
        result = (*iH).second;
    }
    return result;
}

bool Logger::removeHandler(const GString& name)
{
    bool result = false;
    tsl::robin_map<GString, AbstractLogHandler*>::iterator iH = m_handlers.find(name);
    if (iH != m_handlers.end()) {
        AbstractLogHandler* handler = iH->second;
        delete handler;

        m_handlers.erase(iH);
        result = true;
    }
    return result;
}

std::vector<GString> Logger::s_errorStrings{};


} // end namespaces
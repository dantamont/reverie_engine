#pragma once

// Standard
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <type_traits>
#include <ostream>
#include <string>

// internal
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/containers/extern/tsl/robin_map.h"
#include "fortress/time/GDateTime.h"
#include "fortress/layer/framework/GSingleton.h"
#include "fortress/types/GString.h"

namespace rev {

/** @brief Defines some named log levels.
    @details Log levels range from 0 to 100, with 100 representing a critical error and lower numbers describe
	    lesser errors, informational messages and debugging messages.
*/
enum class LogLevel : unsigned int {
    Critical = 100,
    Error = 80,
    Warning = 60,
    Info = 40,
    Debug = 20,
    Unset = 0
};

class Logger;
class ConsoleTool;

/** @class LogRecord
    @brief Holds the attributes associated with a log message.  Instances are passed to the "output" method of
		AbstractLogHandler subclasses.
*/
class LogRecord {
	friend Logger;
public:
	/// @name Constructors and Destructors
	/// @{
	/** @brief Create a LogRecord used by AbstractLogHandler to output log messages.
	*/
    LogRecord();
    ~LogRecord();
	/// @}

    /// @name Properties
	/// @{
	/** @brief The category for the message. 
	    @details The category is usually the full namespace name of a class where the message emanated from.
	*/
    inline const GString& category() const { return m_category; }

	/** @brief The numeric log level for the message.
	*/
    inline LogLevel level() const { return m_level; }
    
	/** @brief text od the message.
	*/
    inline const GString& message() const { return m_message; }
   
	/** @brief The timestamp when the log message came to the Logger.
	*/
    inline const DateTime& timestamp() const { return m_timestamp; }
    /// @}
protected:
	inline void setCategory(const char* category) { m_category = category; }
	inline void setLevel(LogLevel level) { m_level = level; }
	inline void setMessage(const char* message) { m_message = message; }
	inline void setTimestamp(const DateTime& timestamp) { m_timestamp = timestamp; }

private:
    GString m_category;
    LogLevel m_level = LogLevel::Unset;
    GString m_message;
    DateTime m_timestamp;
};

/**	@class AbstractLogHandler
	@brief  A base class for output log. 
	@details All sub-classes must implement the "output" method that is repsonsible for formatting and outputting
		based on the data in a LogRecord.
    // TODO: Make this runnable on different threads
    // https://stackoverflow.com/questions/38725264/is-creating-a-separate-thread-for-a-logger-ok
*/
class AbstractLogHandler {
	
public:
	/// @name Constructors and Destructors
	/// @{
    AbstractLogHandler();
    AbstractLogHandler(LogLevel level);
    virtual ~AbstractLogHandler();
	/// @}

	/// @name Properties
	/// @{
	/** @brief Logging level for this handler.
	*/
    inline LogLevel level() const { return m_level; }

    /** @brief Set logging level of handler
        @param[in] level The level for the output.  Only log messages
            with higher level than this level are output.
    */
    inline void setLevel(LogLevel level) { m_level = level; }

    /** @brief The most-recently handled record.
    */
    inline const LogRecord& mostRecentRecord() const { return m_mostRecentRecord; }

    /// @}

    /// @name Public Methods
    /// @{
    /** @brief Output a log record formatted for the handler.
        @param[in] log_record
    */
    virtual void output(LogRecord& log_record) = 0;

    /// @brief Any actions to perform when added to logger
    virtual void onAddToLogger() {}

    /// @}   

protected:

    LogRecord m_mostRecentRecord;

    LogLevel m_level;
};


/**	@class FileLogHandler
    @brief A LogHandler that outputs messages to a file.
*/
class FileLogHandler : public AbstractLogHandler{
public:
	/// @name Constructors and Destructors
	/// @{
    /** @brief Create a LogHandler to write log messages to a file
        @details Outputs messages at Debug or higher in level.
        @param[in] filename The full path of a file.  If the file does not
            exist, it will be created, otherwise it will be truncated.
    */
    FileLogHandler(const GString& filename);

    /** @brief Create a LogHandler to write log messages to a file
        @param[in] level Messages with a log level higher than this will be
            written out to the file.
        @param[in] filename The full path of a file.  If the file does not
            exist, it will be created, otherwise it will be truncated.
    */
    FileLogHandler(LogLevel level, const GString& filename);

	~FileLogHandler();
	/// @}

    /// @name Properties
    /// @{
    /** @brief Full path to log file
        @return file path
    */
    inline GString& filename() { return m_filename; }

    /** @brief Set file path
        @param[in] filename The path for output.
    */
    inline void setFilename(GString& filename) { m_filename = filename; }
    /// @}

	/// @name Public Methods
	/// @{
    bool isOpen() const;

	/** @brief Output a log record to file
	    @param[in] log_record The data for the log to be output.
	*/
	virtual void output(LogRecord& log_record) override;

	/// @}

private:
	/// @name Private members
	/// @{

    GString m_filename;
    std::ofstream m_ofs;

	/// @}
};

/**	@class StandardOutHandler
    @brief Logger that outputs messages to standard out
*/
class StandardOutHandler : public AbstractLogHandler{
public:
	/// @name Public Methods
	//  @{
	/** @brief Output log record to Standard out
	    @param[in] log_record The data for the log to be output.
	*/
	virtual void output(LogRecord& log_record) override;
	// @}
};


/**	@class Logger
    @brief Singleton class for managing multi-level logging of messages.
    @details A general purpose logging system for applications.  The singleton
        Logger manages a set of AbstractLogHandler subclasses that 
        route messages to one or more output locations: file, standard out, etc.  
        Included handlers are:

        - FileLogHandler: Outputs to a file
        - StandardOutHandler: Outputs to the process Standard Out.

        Use the Logger::addHandler methods to register/add a handler.

        Outputting messages to the logging system can be done in multiple
        ways:

        - Subclass from handler:  The kernel handler class has built
            in logging methods and are the easiest way to to implement
            message logging.   These methods: logCritical, logDebug, 
            Logger::LogError, logInfo and Logger::LogWarning will automatically timestamp
            and categorize log messages.
        - Logger::logMessage: The Logger::logMessage methods.
        - qDebug: Generic use of the qDebug macro can also be used for
            logging, although no timestamp or category information
            is included.
        
        Setting up logging is easy:
        @code
        @endcode
*/
class Logger: public SingletonInterface<Logger> {
public:
    /// @name Static Methods
    /// @{

    static void LogMessage(LogLevel level, const char* category, const char* message);

    // Wrappers for logging routines
    static void LogDebug(const char* msg, const char* cat = "message");
    static void LogDebug(const std::string& msg, const char* cat = "message");
    static void LogDebug(const GString& msg, const char* cat = "message");

    static void LogInfo(const char* msg, const char* cat = "message");
    static void LogInfo(const GString& msg, const char* cat = "message");

    static void LogWarning(const char* msg, const char* cat = "message");
    static void LogWarning(const GString& msg, const char* cat = "message");

    static void LogError(const char* msg, const char* cat = "message");
    static void LogError(const GString& msg, const char* cat = "message");

    static void LogCritical(const char* msg, const char* cat = "message");
    static void LogCritical(const GString& msg, const char* cat = "message");


    static GString LastException();

    static void AddErrorString(const GString& string);

    // Throw routines
    static void Throw(const char* err);
    static void Throw(const std::string& err);
    static void Throw(const GString& err);
    static void Throw(const std::exception& err);

    /// @brief Create instance of file logger
    static rev::FileLogHandler* getFileLogger(const GString& logFileName,
        const GString& loggerName,
        LogLevel logLevel = LogLevel::Debug);

    /// @brief Create instance of std::out logger
    static rev::StandardOutHandler* getStdOutHandler(const GString& loggerName,
        LogLevel logLevel = LogLevel::Debug);

    /// @brief Create instance of a custom handler
    template<typename MyCustomLogHandler, typename ...Args>
    static rev::ConsoleTool* getCustomLogHandler(const GString& loggerName, LogLevel logLevel, const Args&... args)
    {
        static_assert(std::is_base_of_v<AbstractLogHandler, MyCustomLogHandler>, "Custom type must be a Log Handler");

        static std::mutex s_mutex;
        
        // Cache all custom loggers right here
        static tsl::robin_map<std::type_index, AbstractLogHandler*> s_customLoggers;

        std::unique_lock lock(s_mutex);

        // Make sure that logger exists
        Logger& logger = rev::Logger::Instance();

        std::type_index customType = std::type_index(typeid(MyCustomLogHandler));
        if (!s_customLoggers.contains(customType)) {
            MyCustomLogHandler* handler = new MyCustomLogHandler(loggerName.c_str(), std::forward<Args>(args)...);
            handler->setLevel(logLevel);
            logger.addHandler(handler->getName(), handler);
            s_customLoggers[customType] = handler;
            return handler;
        }
        else {
            MyCustomLogHandler* handler = dynamic_cast<MyCustomLogHandler*>(logger.handlerWithName(loggerName));
#ifdef DEBUG_MODE
            assert(nullptr != handler && "Error, unrecognized handler type");
#endif
            handler->setLevel(logLevel);
            return handler;
        }
    }

    /// @brief Destructor
    ~Logger();

    static const GString* levelName(LogLevel level);
    static void addLevelName(LogLevel level, const GString& level_name);

    /// @}

    /// @name Properties
    /// @{
    /** @brief The logging level applied to all messages
        @details Lower numbers mean more messages.
    */
    inline LogLevel level() const { return m_level; }

    /** @brief Set the logging level applied to all messages
    @details Lower numbers mean more messages.
    */
    inline void setLevel(LogLevel level) { m_level = level; }

	/// @name Public Methods
	/// @{
	/** @brief Add log handler to the logging system
        @param[in] name The unique name of the handler
	    @param[in] handler The subclass of AbstractLogHandler
	*/
	void addHandler(const GString& name, AbstractLogHandler* handler);

    /** @brief Get the log handler with the given name
        @param[in] name The name of the desired handler
    */
    AbstractLogHandler* handlerWithName(const GString& name);

    /** @brief Output a log message to associated handlers
        @param[in] level The level for the message.  The higher the
            value, the more severe the issue.
        @param[in] category Category helps define where the message comes from.
        @param[in] message Message is the descriptive message of log.
    */
    void logMessage(LogLevel level, const char* category, const char* message);

    /** @brief Output a log message to associated handlers
        @param[in] level The numeric level for the message.  The higher the
            value, the more severe the issue.
        @param[in] category Category helps define where the message comes from.
        @param[in] message Message is the descriptive message of log.
    */
    inline void logMessage(LogLevel level, std::string& category, std::string& message)
        { logMessage(level, category.c_str(), message.c_str()); }

	/** @brief Remove the named handler from the logging system
	    @param[in] name The name of the log handler to remove
        @retval bool True if the named handler was successfully removed,
            False if the named handler was not found.
	*/
	bool removeHandler(const GString& name);

    /** @brief Remove the named handler from the logging system.
        @param[in] name The name of the log handler to remove
        @retval bool True if the named handler was successfully removed,
            False if the named handler was not found.
    */
    bool removeHandler(const char* name)
        { 
            GString sname(name);
            return removeHandler(sname);
        }
	/// @}

protected:

     Logger();

private: 

    static tsl::robin_map<LogLevel, GString> m_levelNames;
    tsl::robin_map<GString, AbstractLogHandler*> m_handlers;

    static std::vector<GString> s_errorStrings;

    LogLevel m_level;
    LogRecord m_logRecord;
};



} /// end namespacing

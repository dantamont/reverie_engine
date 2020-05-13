#ifndef GB_LOGGER_H
#define GB_LOGGER_H
/** @file GbLogger.h 
    Defines a flexible message logging system.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>

// QT
#include <QMetaType>
#include <QDateTime>
#include <QMutexLocker>
#include <QMutex>
#include "../../core/containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging Constants
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Logger;
namespace View {
    class ConsoleTool;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class LogRecord
    @brief Holds the attributes associated with a log message.  Instances are passed to the "output" method of
		AbstractLogHandler subclasses.
*/
class LogRecord {
	friend Logger;
public:
	//-----------------------------------------------------------------------------------------------------------------
	/// @}
	/// @name Constructors and Destructors
	/// @{
	/** @brief Create a LogRecord used by AbstractLogHandler to output log messages.
	*/
    LogRecord();
    ~LogRecord();
	/// @}
	//-----------------------------------------------------------------------------------------------------------------
	/// @name Properties
	/// @{
	/** @brief The category for the message. 
	    @details The category is usually the full namespace name of a class where the message emanated from.
	*/
    inline const QString& category() const { return m_category; }

	/** @brief The numeric log level for the message.
	*/
    inline LogLevel level() const { return m_level; }
    
	/** @brief text od the message.
	*/
    inline const QString& message() const { return m_message; }
   
	/** @brief The timestamp when the log message came to the Logger.
	*/
    inline const QDateTime& timestamp() const { return m_timestamp; }
    /// @}
protected:
	inline void setCategory(const char* category) { m_category = category; }
	inline void setLevel(LogLevel level) { m_level = level; }
	inline void setMessage(const char* message) { m_message = message; }
	inline void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }

private:
    QString m_category;
    LogLevel m_level = LogLevel::Unset;
    QString m_message;
    QDateTime m_timestamp;
};
Q_DECLARE_METATYPE(LogRecord)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	@class AbstractLogHandler
	@brief  A base class for output log. 
	@details All sub-classes must implement the "output" method that is repsonsible for formatting and outputting
		based on the data in a LogRecord.
    // TODO: Make this runnable on different threads
    // https://stackoverflow.com/questions/38725264/is-creating-a-separate-thread-for-a-logger-ok
*/
class AbstractLogHandler {
	
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @}
	/// @name Constructors and Destructors
	/// @{
    AbstractLogHandler();
    AbstractLogHandler(LogLevel level);
    ~AbstractLogHandler();
	/// @}

    //-----------------------------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{
    /** @brief Output a log record formatted for the handler.
        @param[in] log_record
    */
    virtual void output(LogRecord& log_record) = 0;
    /// @}   

protected:
    LogRecord m_mostRecentRecord;

    LogLevel m_level;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	@class FileLogHandler
    @brief A LogHandler that outputs messages to a file.
*/
class FileLogHandler : public AbstractLogHandler{
public:
    //-----------------------------------------------------------------------------------------------------------------
	/// @name Constructors and Destructors
	/// @{
    /** @brief Create a LogHandler to write log messages to a file
        @details Outputs messages at Debug or higher in level.
        @param[in] filename The full path of a file.  If the file does not
            exist, it will be created, otherwise it will be truncated.
    */
    FileLogHandler(std::string filename);

    /** @brief Create a LogHandler to write log messages to a file
        @param[in] level Messages with a log level higher than this will be
            written out to the file.
        @param[in] filename The full path of a file.  If the file does not
            exist, it will be created, otherwise it will be truncated.
    */
    FileLogHandler(LogLevel level, std::string filename);

	~FileLogHandler();
	/// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @brief Full path to log file
        @return file path
    */
    inline std::string& filename() { return m_filename; }

    /** @brief Set file path
        @param[in] filename The path for output.
    */
    inline void setFilename(std::string& filename) { m_filename = filename; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{
    bool isOpen() const;

	/** @brief Output a log record to file
	    @param[in] log_record The data for the log to be output.
	*/
	virtual void output(LogRecord& log_record) override;

	/*@}*/
private:
	/** @name Property
	@{*/
	std::string m_filename;
    std::ofstream m_ofs;
	/*@}*/
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	@class VSConsoleLogHandler
    @brief  Visual Studio Console log handler
*/
class VSConsoleLogHandler : public AbstractLogHandler{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @}
    /// @name Constructors and Destructors
    /// @{
    VSConsoleLogHandler();
    VSConsoleLogHandler(LogLevel level);
    ~VSConsoleLogHandler();
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
	/** @name Public Methods
	@{*/
	/** @brief Print out logs to Visual Studio console
	    @param[in] log_record The data for the log to be output.
	*/
	virtual void output(LogRecord& log_record) override;
	/*@}*/
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	@class Logger
    @brief Singleton class for managing multi-level logging of messages.
    @details A general purpose logging system for applications.  The singleton
        Logger manages a set of AbstractLogHandler subclasses that 
        route messages to one or more output locations: file, standard out,
        Visual Studio, etc.  Included handlers are:

        - FileLogHandler: Outputs to a file
        - StandardOutHandler: Outputs to the process Standard Out.
        - VSConsoleLogHandler: Outputs to the Visual Studio Console

        Use the Logger::addHandler methods to register/add a handler.

        Outputting messages to the logging system can be done in multiple
        ways:

        - Subclass from handler:  The kernel handler class has built
            in logging methods and are the easiest way to to implement
            message logging.   These methods: logCritical, logDebug, 
            logError, logInfo and logWarning will automatically timestamp
            and categorize log messages.
        - Logger::logMessage: The Logger::logMessage methods.
        - qDebug: Generic use of the qDebug macro can also be used for
            logging, although no timestamp or category information
            is included.
        
        Setting up logging is easy:
        @code
        @endcode
*/
class Logger {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{
    /** @brief Get the singleton instance of Logger
    */
    static Logger& getInstance();

    /// @brief Create instance of file logger
    static Gb::FileLogHandler* getFileLogger(const std::string& logFileName, 
        const std::string& loggerName,
        LogLevel logLevel = LogLevel::Debug);

    /// @brief Create instance of VS logger
    static Gb::VSConsoleLogHandler* getVSLogger(const std::string& loggerName,
        LogLevel logLevel = LogLevel::Debug);

    /// @brief Create instance of console tool logger
    static Gb::View::ConsoleTool* getConsoleTool(const std::string& loggerName, 
        LogLevel logLevel = LogLevel::Debug);

    /** @brief 
    */
    static const std::string* levelName(LogLevel level);
    static void addLevelName(LogLevel level, const std::string& level_name);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{
	/** @brief Add log handler to the logging system
        @param[in] name The unique name of the handler
	    @param[in] handler The subclass of AbstractLogHandler
	*/
	void addHandler(const std::string& name, AbstractLogHandler* handler);

    /** @brief Get the log handler with the given name
        @param[in] name The name of the desired handler
    */
    AbstractLogHandler* handlerWithName(const std::string& name);

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
	bool removeHandler(const std::string& name);

    /** @brief Remove the named handler from the logging system.
        @param[in] name The name of the log handler to remove
        @retval bool True if the named handler was successfully removed,
            False if the named handler was not found.
    */
    bool removeHandler(const char* name)
        { 
            std::string sname(name);
            return removeHandler(sname);
        }
	/// @}
private:
    Logger();
    ~Logger();

    static std::unordered_map<LogLevel, std::string> m_level_names;
    std::unordered_map<std::string, AbstractLogHandler*> m_handlers;
    LogLevel m_level;
    LogRecord m_log_record;
    QMutex m_output_mutex;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @cond DocIgnore
//#ifdef QT_CORE_LIB
//// If using Qt Libraries, create a logging handler
///** @brief Defines a Qt logging handler used to route qDebug() calls
//    @details Ties qDebug and other Qt logging calls to the Logger
//    @param[in] msg_type Qt Enum for debug, warning, error types.
//    @param[in] context Unused here
//    @param[in] msg The message to send to the logger
//*/
//void loggingHandler(QtMsgType msg_type,
//                    const QMessageLogContext& context,
//                    const QString& msg);
//#endif
/// @endcond


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif
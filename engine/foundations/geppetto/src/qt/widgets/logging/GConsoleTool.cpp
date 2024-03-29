
// Includes

#include "geppetto/qt/widgets/logging/GConsoleTool.h"
// Standard Includes

// External
#include <windows.h>
#include <QtWidgets>

// Internal
#include "fortress/process/GProcess.h"


namespace rev {


void ReadOnlyTextEdit::keyPressEvent(QKeyEvent * event)
{
    Q_UNUSED(event)
}

void ReadOnlyTextEdit::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event)
}

void ReadOnlyTextEdit::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event)
}

void ReadOnlyTextEdit::mouseReleaseEvent(QMouseEvent * event)
{
    Q_UNUSED(event)
}


// ConsoleTool

// Constructors and Destructors

ConsoleTool::ConsoleTool(const QString& name, QWidget* parent) :
    rev::NameableInterface(name.toStdString()),
    QWidget(parent),
    AbstractLogHandler()
{
    // Set size policy of main window
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumWidth(500);

	// Set default color for the text edit
	m_foreground_color_debug.setRgb(0, 240, 240);
	m_foreground_color_error.setRgb(240, 0, 0);
	m_foreground_color_info.setRgb(100, 100, 240);
	m_foreground_color_warning.setRgb(200, 120, 40);

	// Initialize console window
	setWindowTitle(name);
			
	// Build the contents
    QGridLayout *glo = new QGridLayout(this);
    glo->setContentsMargins(0, 0, 0, 0);
    glo->setColumnStretch(0, 1);
    glo->setRowStretch(0, 1);

	// Add text widget
	m_consoleTextEdit = new ReadOnlyTextEdit();
	m_consoleTextEdit->setReadOnly(true);
    glo->addWidget(m_consoleTextEdit, 0, 0);

    // Connect the Private logging signal to the object slot so this Console Tool can work across threads
	qRegisterMetaType<rev::LogRecord>("rev::LogRecord");
    connect(this, &ConsoleTool::outputRecordPvt, this, &ConsoleTool::handleOutputRecord);
}

// Public Methods

void ConsoleTool::output(rev::LogRecord& logRecord)
{
    /// \todo Can't call slots on non-main thread, so make handleOutputRecord a normal function
    bool isMainThread = QApplication::instance()->thread() == QThread::currentThread();
    if (isMainThread) {
        // In the main thread, so output the record directly
        handleOutputRecord(logRecord);
    } 
    else {
        emit outputRecordPvt(logRecord);
    }

    m_logMutex.lock();
    m_mostRecentLogRecord = logRecord;
    m_logMutex.unlock();
}

void ConsoleTool::onAddToLogger()
{
    show();
}

// Private Methods

void ConsoleTool::handleOutputRecord(const rev::LogRecord& logRecord)
{
    QMutexLocker locker(&m_logMutex);

    const GString& category = logRecord.category();
    rev::LogLevel level = logRecord.level();
    const GString& message = logRecord.message();
    const DateTime& timestamp = logRecord.timestamp();

    // Set the text edit so that is can be written to.
    m_consoleTextEdit->setReadOnly(false);

    // Get the Text Cursor, acts like a QPainter, it determines where and how text is drawn.
    QTextCursor textCursor = m_consoleTextEdit->textCursor();

    // Set up formatting for text blocks(analogous to a paragraph
    QTextBlockFormat blockFormat = textCursor.blockFormat();
    blockFormat.setLeftMargin(0);

    // Set up the text character format to colorize the text
	QTextCharFormat charFormat = textCursor.charFormat();
	if (level >= rev::LogLevel::Error) {
		charFormat.setForeground(QBrush(m_foreground_color_error));
	} else if (level >= rev::LogLevel::Warning) {
		charFormat.setForeground(QBrush(m_foreground_color_warning));
	} else if (level >= rev::LogLevel::Info) {
		charFormat.setForeground(QBrush(m_foreground_color_info));
	} else {
		charFormat.setForeground(QBrush(m_foreground_color_debug));
	}

    // The text will be divided into two blocks, one for the log_record
    // information(level, timestamp, class name) and one for the message

    // Output the log record information only if enough time has passed from
    // the last time this method was called
    LogRecord mostRecentRecord = m_mostRecentLogRecord;
    if (level != mostRecentRecord.level() ||
        category != mostRecentRecord.category() ||
        abs(timestamp.getTimeToMs(mostRecentRecord.timestamp())) > 2) {

        // Create the block for the log_record information
        charFormat.setFontWeight(QFont::Normal);
        textCursor.insertBlock(blockFormat);
        textCursor.setCharFormat(charFormat);

        QString headerStr = QStringLiteral("%1  %2  %3");
        const GString* level_name = rev::Logger::levelName(level);
        if (level_name) {
            headerStr = headerStr.arg(level_name->c_str());
        } else {
            headerStr = headerStr.arg((unsigned int)level);
        }
        headerStr = headerStr.arg(timestamp.toString("MMM dd, yyyy @ hh:mm:ss.zzz").c_str())
            .arg((const char*)category);

        textCursor.insertText(headerStr);
    }

    // Next, output the message with a left side margin(intents the message)
    blockFormat.setLeftMargin(20);
    charFormat.setFontWeight(QFont::Bold);
    textCursor.insertBlock(blockFormat);
    textCursor.setCharFormat(charFormat);

    // Output the log record message
    textCursor.insertText(message.c_str(), charFormat);

    // Reset the editor to readonly
    m_consoleTextEdit->setReadOnly(true);

    // Scroll to the bottom of the console
    scrollToBottom();

}			

void ConsoleTool::scrollToBottom() {
    m_consoleTextEdit->verticalScrollBar()->setValue(m_consoleTextEdit->verticalScrollBar()->maximum());
}


} // rev
#include "GbParameterWidgets.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/mixins/GbLoadable.h"
#include "../../core/readers/GbJsonReader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParameterWidget
ParameterWidget::ParameterWidget(CoreEngine* core, QWidget* parent) :
    QWidget(parent),
    m_engine(core)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::initialize()
{
    this->initializeWidgets();
    this->initializeConnections();
    this->layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FileLoadWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FileLoadWidget::FileLoadWidget(CoreEngine* core, 
    const QString& defaultText,
    const QString& defaultTitleText,
    const QString& selectionFilter,
    QWidget* parent):
    ParameterWidget(core, parent),
    m_defaultText(defaultText),
    m_titleText(defaultTitleText),
    m_selectionFilter(selectionFilter)
{
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FileLoadWidget::initializeWidgets()
{
    m_lineEdit = new QLineEdit(m_defaultText);
    //m_lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_button = new QPushButton();
    m_button->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    //m_button->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FileLoadWidget::initializeConnections()
{
    connect(m_button, &QPushButton::clicked, this, [this]() {
        
        QString text = QFileDialog::getOpenFileName(this, tr(m_titleText.toStdString().c_str()),
            ".", tr(m_selectionFilter.toStdString().c_str()));
        m_lineEdit->setText(text);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FileLoadWidget::layoutWidgets()
{
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_lineEdit);
    m_mainLayout->addWidget(m_button);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    setLayout(m_mainLayout);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// JsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
JsonWidget::JsonWidget(CoreEngine* core, Serializable* serializable, QWidget *parent) :
    ParameterWidget(core, parent),
    m_serializable(serializable)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
JsonWidget::~JsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void JsonWidget::updateText(bool reloadJson)
{
    // Block signals to avoid infinite loop of signal sends
    m_textEdit->blockSignals(true);

    // Get text and color if not valid JSON
    QString text;
    if (!reloadJson) {
        text = m_textEdit->toPlainText();
    }
    else {
        text = JsonReader::getJsonValueAsQString(m_serializable->asJson(), true);
        m_textEdit->setText(text);
    }
    QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);
    QPalette palette;
    QColor badColor = QApplication::palette().color(QPalette::BrightText);
    QColor goodColor = QApplication::palette().color(QPalette::Highlight);
    if (contents.isNull()) {
        palette.setColor(QPalette::Text, badColor);
        m_textEdit->setPalette(palette);
    }
    else {
        palette.setColor(QPalette::Text, goodColor);
        m_textEdit->setPalette(palette);
    }

    m_textEdit->blockSignals(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void JsonWidget::initializeWidgets()
{
    ParameterWidget::initializeWidgets();
    m_textEdit = new QTextEdit();
    m_textEdit->setText(JsonReader::getJsonValueAsQString(m_serializable->asJson(), true));
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void JsonWidget::initializeConnections()
{
    ParameterWidget::initializeConnections();

    // Make component to recolor text on change
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        updateText();
    });

    // Make connection to resize serializable 
    connect(m_confirmButton, &QPushButton::clicked, this, [this](bool checked) {
        Q_UNUSED(checked);

        // Get text and return if not valid JSON
        const QString& text = m_textEdit->toPlainText();
        QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);
        if (contents.isNull()) return;

        // Perform preloaod
        preLoad();

        // Edit serializable via JSON
        m_serializable->loadFromJson(contents.object());

        // Update widget text with updated JSON
        m_textEdit->setText(
            JsonReader::getJsonValueAsQString(m_serializable->asJson(), true)
        );

        // Perform post-load
        postLoad();

    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void JsonWidget::layoutWidgets()
{
    // Create base widget layout
    ParameterWidget::layoutWidgets();
    m_mainLayout = new QVBoxLayout;

    // Create new widgets
    QBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_textEdit);
    layout->addWidget(m_confirmButton);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
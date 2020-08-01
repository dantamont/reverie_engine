#include "GbParameterWidgets.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/mixins/GbLoadable.h"
#include "../../core/readers/GbJsonReader.h"
#include "../GbWidgetManager.h"
#include "../../core/loop/GbSimLoop.h"
#include "../../core/geometry/GbVector.h"

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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QBoxLayout* ParameterWidget::LabeledLayout(const QString& label,
    QWidget* widget,
    QBoxLayout::Direction dir) 
{
    QBoxLayout* layout = new QBoxLayout(dir);
    layout->addWidget(new QLabel(label));
    layout->addWidget(widget);
    return layout;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::AddLabel(const QString & label, QBoxLayout * layout)
{
    layout->addWidget(new QLabel(label));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::AddLabel(const QIcon & label, QBoxLayout * layout, const Vector<float, 2>& size)
{
    QLabel* l = new QLabel();
    l->setPixmap(label.pixmap(size.x(), size.y()));
    layout->addWidget(l);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QWidget*> ParameterWidget::GetTopLevelWidgets(QLayout * layout)
{
    std::vector<QWidget*> widgets;
    for (int i = 0; i < layout->count(); ++i)
    {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr)
        {
            widgets.push_back(widget);
        }
    }
    return widgets;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterWidget::ParameterWidget(CoreEngine* core, QWidget* parent) :
    QWidget(parent),
    m_engine(core)
{
    m_engine->widgetManager()->addParameterWidget(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ParameterWidget::~ParameterWidget()
{
    m_engine->widgetManager()->removeParameterWidget(this);

    // Want to properly delete children
    //clearLayout(m_mainLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::update()
{
#ifdef DEBUG_MODE
    //logInfo("Updating base routine");
#endif
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

///////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::pauseSimulation()
{
    // Pause scenario to edit component
    SimulationLoop* simLoop = m_engine->simulationLoop();
    m_wasPlaying = simLoop->isPlaying();
    if (m_wasPlaying) {
        simLoop->pause();
    }
    m_engine->simulationLoop()->pause();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ParameterWidget::resumeSimulation()
{
    // Unpause scenario
    SimulationLoop* simLoop = m_engine->simulationLoop();
    if (m_wasPlaying) {
        simLoop->play();
    }
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
void JsonWidget::updateText()
{
    // Block signals to avoid infinite loop of signal sends
    m_textEdit->blockSignals(true);

    // Get text and color if not valid JSON
    QString text = m_textEdit->toPlainText();
    //if (!m_reloadJson) {
    //    text = m_textEdit->toPlainText();
    //}
    //else {
    //    text = JsonReader::getJsonValueAsQString(m_serializable->asJson(), true);
    //    m_textEdit->setText(text);
    //}
    QJsonDocument contents = JsonReader::ToJsonDocument(text);
    QJsonObject object = contents.object();
    QPalette palette;
    QColor badColor = QApplication::palette().color(QPalette::BrightText);
    QColor goodColor = QApplication::palette().color(QPalette::Highlight);
    if (contents.isNull() || !isValidObject(object)) {
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
    m_textEdit->setText(JsonReader::ToQString(m_serializable->asJson(), true));
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
        QJsonDocument contents = JsonReader::ToJsonDocument(text);
        if (contents.isNull() || !isValidObject(contents.object())) return;

        // Perform preloaod
        preLoad();

        // Edit serializable via JSON
        m_serializable->loadFromJson(contents.object());

        // Update widget text with updated JSON
        m_textEdit->setText(
            JsonReader::ToQString(m_serializable->asJson(), true)
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
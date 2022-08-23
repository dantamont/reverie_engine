#include "geppetto/qt/widgets/components/GScriptComponentWidget.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/components/GPhysicsWidgets.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

namespace rev {

ScriptWidget::ScriptWidget(WidgetManager* wm, const json& componentJson, Int32_t sId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sId, parent){
    initialize();
}


ScriptWidget::~ScriptWidget()
{
}

void ScriptWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    QString path = m_componentJson["path"].get_ref<const std::string&>().c_str();
    m_fileWidget = new FileLoadWidget(m_widgetManager, path, "Open Python Behavior", "Python Scripts (*.py)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    // Add refresh script button
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}

void ScriptWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to load python script
    connect(m_fileWidget->lineEdit(), &QLineEdit::textChanged, this, [this]() {

        const QString& text = m_fileWidget->lineEdit()->text();
        m_resetScriptMessage.setNewFilePath(text.toStdString().c_str());
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_resetScriptMessage);
    });

    // Make connection to reset script 
    connect(m_confirmButton,
        &QPushButton::clicked, 
        this, [this](bool checked) {
        Q_UNUSED(checked);

        // Refresh script 
        m_resetScriptMessage.setNewFilePath("");
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_resetScriptMessage);
    });
}

void ScriptWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(new QLabel("Python Script Path:"));
    layout->addWidget(m_fileWidget);
    layout->addWidget(m_confirmButton);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);

}



} // rev
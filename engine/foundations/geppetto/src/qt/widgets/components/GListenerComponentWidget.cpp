#include "geppetto/qt/widgets/components/GListenerComponentWidget.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

#include "fortress/json/GJson.h"
#include "ripple/network/messages/GModifyListenerScriptMessage.h"

namespace rev {

ListenerWidget::ListenerWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent)
{
    m_listenerScriptMessage.setSceneObjectId(sceneObjectId);
    initialize();
}

ListenerWidget::~ListenerWidget()
{
}

void ListenerWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Initialize file load widget
    QString path = "";
    if (m_componentJson.contains("listener")) {
        const json& listenerJson = m_componentJson["listener"];

        if (listenerJson.contains("path")) {
            path = listenerJson["path"].get_ref<const std::string&>().c_str();
        }
    }
    m_fileWidget = new FileLoadWidget(m_widgetManager, path, "Open Python Listener", "Listener Scripts (*_listener.py)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    // Initialize event types list
    m_eventTypes = new QLineEdit();
    if (m_componentJson.contains("listener")) {
        QString text = "";
        for (const json& eventType: m_componentJson["listener"]["eventTypes"]) {
            Int32_t type = eventType;
            text += QString::number(type) + ", ";
        }
        m_eventTypes->setText(text);
    }
    m_eventTypes->setToolTip("Comma-delimited list of accepted event types");

    // Add refresh script button
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}

void ListenerWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to load python script
    connect(m_fileWidget->lineEdit(), &QLineEdit::textChanged, this,
        [this]() {
            requestScriptUpdate(m_fileWidget->lineEdit()->text().toStdString().c_str());
        }
    );

    // Make connections to set event types
    connect(m_eventTypes, &QLineEdit::textChanged,this,
        [this]() {
            requestScriptUpdate(m_fileWidget->lineEdit()->text().toStdString().c_str());
        }
    );

    // Make connection to reinitialize script 
    connect(m_confirmButton,
        &QPushButton::clicked, 
        this, [this](bool checked) {
            Q_UNUSED(checked);
            requestScriptUpdate(m_fileWidget->lineEdit()->text().toStdString().c_str());
        }
    );
}

void ListenerWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->addWidget(new QLabel("Listener Script Path:"));
    layout->addWidget(m_fileWidget);
    layout->addWidget(m_confirmButton);

    // Add widgets to main layout
    auto* eventLayout = LabeledLayout("Event Types: ", m_eventTypes);
    m_mainLayout->addLayout(eventLayout);
    m_mainLayout->addLayout(layout);

}

void ListenerWidget::requestScriptUpdate(const char* cStr)
{
    std::vector<int> eventTypes;
    getEventTypes(eventTypes);
    m_listenerScriptMessage.setScriptPath(cStr);
    m_listenerScriptMessage.setEventTypes(eventTypes);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_listenerScriptMessage);
}

void ListenerWidget::getEventTypes(std::vector<int>& outVec)
{
    outVec.clear();
    QStringList intStrs = m_eventTypes->text().split(",", Qt::SkipEmptyParts);
    for (QString& str : intStrs) {
        str.remove(' ');
        outVec.push_back(str.toInt());
    }
}


} // rev
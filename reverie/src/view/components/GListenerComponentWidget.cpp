#include "GListenerComponentWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GComponent.h"
#include "../../core/components/GListenerComponent.h"
#include "../../core/events/GEventListener.h"
#include "../../core/scripting/GPythonScript.h"


#include "../style/GFontIcon.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ListenerWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ListenerWidget::ListenerWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent){
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ListenerComponent* ListenerWidget::listenerComponent() const {
    return static_cast<ListenerComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ListenerWidget::~ListenerWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Initialize file load widget
    QString path = "";
    if (listenerComponent()->listener()) {
        const PythonClassScript* script = listenerComponent()->listener()->script();
        if (script) {
            path = (const char*)script->getPath();
        }
    }
    m_fileWidget = new FileLoadWidget(m_engine, path, "Open Python Listener", "Listener Scripts (*_listener.py)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    // Initialize event types list
    m_eventTypes = new QLineEdit();
    if (listenerComponent()->listener()) {
        QString text = "";
        for (int type : listenerComponent()->listener()->eventTypes()) {
            text += QString::number(type) + ", ";
        }
        m_eventTypes->setText(text);
    }
    m_eventTypes->setToolTip("Comma-delimited list of accepted event types");

    // Add refresh script button
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to load python script
    connect(m_fileWidget->lineEdit(), &QLineEdit::textChanged, this, [this]() {
        std::vector<int> eventTypes;
        getEventTypes(eventTypes);

        const QString& text = m_fileWidget->lineEdit()->text();
        listenerComponent()->initializeListener(text);
        listenerComponent()->listener()->setEventTypes(eventTypes);
    });

    // Make connections to set event types
    connect(m_eventTypes, &QLineEdit::textChanged, this, [this]() {
        std::vector<int> eventTypes;
        getEventTypes(eventTypes);

        // If no listener, create
        if (!listenerComponent()->listener()) {
            // Verify that widget has a valid path
            const QString& path = m_fileWidget->lineEdit()->text();
            if (!path.isEmpty()) {
                QFile scriptFile(path);
                if (scriptFile.exists()) {
                    // Path is valid, create listener
                    listenerComponent()->initializeListener(path);
                }
            }
        }

        // Check that a valid listener was created or existed
        if (listenerComponent()->listener()) {
            listenerComponent()->listener()->setEventTypes(eventTypes);
        }
    });

    // Make connection to reinitialize script 
    connect(m_confirmButton,
        &QPushButton::clicked, 
        this, [this](bool checked) {
        Q_UNUSED(checked);

        // Pause scenario to edit component
        SimulationLoop* simLoop = m_engine->simulationLoop();
        bool wasPlaying = simLoop->isPlaying();
        if (wasPlaying) {
            simLoop->pause();
        }
        m_engine->simulationLoop()->pause();

        auto listenerComponent = (ListenerComponent*)(m_component);
        if (listenerComponent->listener()) {
            // Refresh script 
            std::vector<int> eventTypes;
            getEventTypes(eventTypes);

            listenerComponent->reset();
            listenerComponent->listener()->setEventTypes(eventTypes);
        }

        // Unpause scenario
        if (wasPlaying) {
            simLoop->play();
        }
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void ListenerWidget::getEventTypes(std::vector<int>& outVec)
{
    outVec.clear();
    QStringList intStrs = m_eventTypes->text().split(",", Qt::SkipEmptyParts);
    for (QString& str : intStrs) {
        str.remove(' ');
        outVec.push_back(str.toInt());
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
#include "geppetto/qt/widgets/types/GLoadAudioWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

#include "fortress/json/GJson.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

LoadAudioWidget::LoadAudioWidget(WidgetManager* wm, QWidget *parent) :
    ParameterWidget(wm, parent)
{
    initialize();
}

LoadAudioWidget::~LoadAudioWidget()
{
    delete m_streamAudio;
    delete m_fileWidget;
    delete m_confirmButtons;
}

void LoadAudioWidget::initializeWidgets()
{
    QString path = "";
    m_fileWidget = new FileLoadWidget(m_widgetManager, path, "Select Audio File", "Audio files (*.mp3 *.wav *.flac)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    m_streamAudio = new QCheckBox("Stream Audio");
    m_streamAudio->setToolTip(QStringLiteral("If false, loads entire file in on load. Has much higher memory overhead, but takes less processing power than streaming."));

    // Add dialog confirm buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);
}

void LoadAudioWidget::initializeConnections()
{

    // Make connection to load audio file
    connect(m_confirmButtons,
        &QDialogButtonBox::accepted,
        this,
        [this]() {

            m_loadMessage.setFilePath(m_fileWidget->lineEdit()->text().toStdString().c_str());
            m_audioHandleId = Uuid();
            m_loadMessage.setUuid(m_audioHandleId);
            m_loadMessage.setStreamAudio(m_streamAudio->isChecked());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_loadMessage);

            // Close the widget
            close();
        }
    );

    connect(m_confirmButtons, &QDialogButtonBox::rejected,
        this,
        [this]() 
        {
            // Close this widget
            close();
        }
    );
}

void LoadAudioWidget::layoutWidgets()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    // Add widgets to main layout
    QBoxLayout* fileLoadLayout = LabeledLayout("Audio File:", m_fileWidget);
    fileLoadLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addLayout(fileLoadLayout);
    m_mainLayout->addWidget(m_streamAudio);
    m_mainLayout->addWidget(m_confirmButtons);

    setWindowTitle("Load Audio");
}


} // rev
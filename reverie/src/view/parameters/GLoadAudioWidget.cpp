#include "GLoadAudioWidget.h"

#include "../../core/components/GAudioSourceComponent.h"
#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GComponent.h"


namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LoadAudioWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadAudioWidget::LoadAudioWidget(CoreEngine* core, QWidget *parent) :
    ParameterWidget(core, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadAudioWidget::~LoadAudioWidget()
{
    delete m_streamAudio;
    delete m_fileWidget;
    delete m_confirmButtons;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadAudioWidget::initializeWidgets()
{
    QString path = "";
    m_fileWidget = new FileLoadWidget(m_engine, path, "Select Audio File", "Audio files (*.mp3 *.wav *.flac)");
    m_fileWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    m_streamAudio = new QCheckBox("Stream Audio");
    m_streamAudio->setToolTip(QStringLiteral("If false, loads entire file in on load. Has much higher memory overhead, but takes less processing power than streaming."));

    // Add dialog confirm buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadAudioWidget::initializeConnections()
{
    //connect(m_fileWidget->lineEdit(), &QLineEdit::textChanged, this, [this]() {

    //    const QString& text = m_fileWidget->lineEdit()->text();
    //    std::shared_ptr<ResourceHandle> audio;
    //    audio = m_engine->resourceCache()->getTopLevelHandleWithPath(text);
    //    
    //    if (audio) {
    //        // If the audio file is already loaded, return
    //        return;
    //    }

    //    // Load audio

    //});

    //connect(m_streamAudio, &QCheckBox::stateChanged, this,
    //    [this](int state) {
    //    bool checked = state != 0;

    //}
    //);

    // Make connection to load audio file
    connect(m_confirmButtons,
        &QDialogButtonBox::accepted,
        this, [this]() {

        const QString& text = m_fileWidget->lineEdit()->text();
        std::shared_ptr<ResourceHandle> audio;
        audio = m_engine->resourceCache()->getTopLevelHandleWithPath(text);

        if (audio) {
            // If the audio file is already loaded, return
            QMessageBox::warning(nullptr, "Audio Not Loaded", 
                "Did not load audio file, audio at " + text + " already loaded.");
            return;
        }

        QFile audioFile(text);
        if (!audioFile.exists()) {
            // If the audio file is does not exist, return
            QMessageBox::warning(nullptr, "Audio Not Loaded",
                "Did not load audio, " + text + " does not exist.");
            return;
        }

        // Load audio
        AudioResource::SourceType sourceType = m_streamAudio->isChecked() ? AudioResource::SourceType::kWavStream: AudioResource::SourceType::kWav;
        AudioResource::CreateHandle(m_engine, text, sourceType);

        // Close the widget
        close();
    });

    connect(m_confirmButtons, &QDialogButtonBox::rejected,
        this,
        [this]() {
        // Close this widget
        close();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
#include "geppetto/qt/widgets/components/GAudioListenerComponentWidget.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"

#include "fortress/json/GJson.h"

#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

AudioListenerWidget::AudioListenerWidget(WidgetManager* wm, const json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent)
{
    m_listenerMessage.setSceneObjectId(sceneObjectId);
    initialize();
}

AudioListenerWidget::~AudioListenerWidget()
{
}

void AudioListenerWidget::requestAudioListenerUpdate()
{
    m_listenerMessage.setVelocityX(m_velocityX->text().toDouble());
    m_listenerMessage.setVelocityY(m_velocityY->text().toDouble());
    m_listenerMessage.setVelocityZ(m_velocityZ->text().toDouble());
    m_listenerMessage.setSpeedofSound(m_speedOfSound->text().toDouble());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_listenerMessage);
}

void AudioListenerWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    Vector3 velocity = m_componentJson["velocity"];
    m_velocityX = new QLineEdit(QString::number(velocity.x()));
    m_velocityX->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    m_velocityY = new QLineEdit(QString::number(velocity.y()));
    m_velocityY->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    m_velocityZ = new QLineEdit(QString::number(velocity.z()));
    m_velocityZ->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    Float32_t speedOfSound = m_componentJson["sos"];
    m_speedOfSound = new QLineEdit(QString::number(speedOfSound));
    m_speedOfSound->setValidator(new QDoubleValidator(1e-2, 1e20, 8));
    m_speedOfSound->setToolTip(QStringLiteral("Speed of sound, in world units/sec. Default is 343 (m/s)"));

}

void AudioListenerWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    connect(m_velocityX,
        static_cast<void(QLineEdit::*)(void)>(&QLineEdit::editingFinished),
        this, 
        &AudioListenerWidget::requestAudioListenerUpdate);

    connect(m_velocityY, 
        static_cast<void(QLineEdit::*)(void)>(&QLineEdit::editingFinished), 
        this,
        &AudioListenerWidget::requestAudioListenerUpdate);

    connect(m_velocityZ, 
        static_cast<void(QLineEdit::*)(void)>(&QLineEdit::editingFinished), 
        this,
        &AudioListenerWidget::requestAudioListenerUpdate);

    connect(m_speedOfSound, 
        static_cast<void(QLineEdit::*)(void)>(&QLineEdit::editingFinished), 
        this,
        &AudioListenerWidget::requestAudioListenerUpdate);
}

void AudioListenerWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Create new widgets
    QBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    QGroupBox* velocity = new QGroupBox("Velocity");
    QHBoxLayout* velocityLayout = new QHBoxLayout();
    velocityLayout->setSpacing(5);
    velocityLayout->addWidget(new QLabel("x:"));
    velocityLayout->addWidget(m_velocityX);
    velocityLayout->addWidget(new QLabel("y:"));
    velocityLayout->addWidget(m_velocityY);
    velocityLayout->addWidget(new QLabel("z:"));
    velocityLayout->addWidget(m_velocityZ);
    velocity->setLayout(velocityLayout);

    QBoxLayout* sosLayout = LabeledLayout(QStringLiteral("Speed of Sound:"), m_speedOfSound);
    sosLayout->setSpacing(5);

    layout->addWidget(velocity);
    layout->addLayout(sosLayout);

    // Add widgets to main layout 
    m_mainLayout->addLayout(layout);
}


} // rev
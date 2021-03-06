#include "GAudioListenerComponentWidget.h"

#include "../../core/components/GAudioListenerComponent.h"
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
// AudioListenerWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerWidget::AudioListenerWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent){
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerComponent* AudioListenerWidget::audioListener() const {
    return static_cast<AudioListenerComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AudioListenerWidget::~AudioListenerWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_velocityX = new QLineEdit(QString::number(audioListener()->velocity().x()));
    m_velocityX->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    m_velocityY = new QLineEdit(QString::number(audioListener()->velocity().y()));
    m_velocityY->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    m_velocityZ = new QLineEdit(QString::number(audioListener()->velocity().z()));
    m_velocityZ->setValidator(new QDoubleValidator(-1e200, 1e200, 8));

    m_speedOfSound = new QLineEdit(QString::number(audioListener()->speedOfSound()));
    m_speedOfSound->setValidator(new QDoubleValidator(1e-2, 1e20, 8));
    m_speedOfSound->setToolTip(QStringLiteral("Speed of sound, in world units/sec. Default is 343 (m/s)"));

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AudioListenerWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    connect(m_velocityX, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        const QString& text = m_velocityX->text();
        Vector3 velocity = audioListener()->velocity();
        velocity[0] = text.toDouble();
        audioListener()->setVelocity(velocity);
    });

    connect(m_velocityY, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        const QString& text = m_velocityY->text();
        Vector3 velocity = audioListener()->velocity();
        velocity[1] = text.toDouble();
        audioListener()->setVelocity(velocity);
    });

    connect(m_velocityZ, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        const QString& text = m_velocityZ->text();
        Vector3 velocity = audioListener()->velocity();
        velocity[2] = text.toDouble();
        audioListener()->setVelocity(velocity);
    });


    connect(m_speedOfSound, static_cast<void(QLineEdit::*)(void)>(
        &QLineEdit::editingFinished), this,
        [this]() {
        const QString& text = m_speedOfSound->text();
        audioListener()->setSpeedOfSound(text.toDouble());
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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

    // Add widgets to main layout -------------------
    m_mainLayout->addLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
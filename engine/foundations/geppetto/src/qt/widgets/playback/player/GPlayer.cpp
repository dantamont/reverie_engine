/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "geppetto/qt/widgets/playback/player/GPlayer.h"

#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QComboBox>

#include "enums/GSimulationPlayModeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

#include "fonts/GFontManager.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {

PlayerControls::PlayerControls(WidgetManager* manager, QWidget *parent):
    QWidget(parent),
    rev::NameableInterface("ScenePlayer"),
    m_playButton(0),
    m_stopButton(0),
    m_nextButton(0),
    m_previousButton(0),
    m_muteButton(0),
    m_volumeSlider(0),
    m_rateBox(0),
    m_manager(manager)
{
    m_playbackModeMessage.setPlaybackMode(ESimulationPlayMode::eDebug); // Starts in debug mode
    initializeWidgets();
    initializeConnections();
    layoutWidgets();
}

void PlayerControls::initializeWidgets()
{
    m_playButton = new QToolButton(this);
    m_playButton->setToolTip("Play/Pause scene playback");

    m_stopButton = new QToolButton(this);
    m_stopButton->setIcon(SAIcon(QStringLiteral("stop")));
    m_stopButton->setEnabled(false);

    m_nextButton = new QToolButton(this);
    m_nextButton->setIcon(SAIcon(QStringLiteral("forward")));

    m_previousButton = new QToolButton(this);
    m_previousButton->setIcon(SAIcon(QStringLiteral("backward")));

    m_modeButton = new QToolButton(this);

    // Request playback data from the main application to populate widgets
    sendRequestPlaybackDataMessage();

    //ui->pushButton->setFont(font);
    //ui->pushButton->setText("\uf083");

    //muteButton = new QToolButton(this);
    //muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    //volumeSlider = new QSlider(Qt::Horizontal, this);
    //volumeSlider->setRange(0, 100);

    //rateBox = new QComboBox(this);
    //rateBox->addItem("0.5x", QVariant(0.5));
    //rateBox->addItem("1.0x", QVariant(1.0));
    //rateBox->addItem("2.0x", QVariant(2.0));
    //rateBox->setCurrentIndex(1);
}

void PlayerControls::initializeConnections()
{
    static const GString sender = "geppetto";
    m_requestPlaybackDataMessage.header().setSender(sender);
    m_togglePlaybackMessage.header().setSender(sender);
    m_playbackModeMessage.header().setSender(sender);

    connect(m_playButton, SIGNAL(clicked()), this, SLOT(playClicked()));
    connect(m_stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));
    connect(m_nextButton, SIGNAL(clicked()), this, SIGNAL(next()));
    connect(m_previousButton, SIGNAL(clicked()), this, SIGNAL(previous()));
    connect(m_modeButton, &QAbstractButton::clicked, this, [&]() {
        /// @todo Replace with encoded GString
        QString modeStr;
        switch ((ESimulationPlayMode)m_playbackModeMessage.getPlaybackMode()) {
        case ESimulationPlayMode::eDebug:
            m_playbackModeMessage.setPlaybackMode((GSimulationPlayMode)ESimulationPlayMode::eStandard);
            modeStr = QStringLiteral("binoculars");
            break;
        case ESimulationPlayMode::eStandard:
            m_playbackModeMessage.setPlaybackMode((GSimulationPlayMode)ESimulationPlayMode::eDebug);
            modeStr = QStringLiteral("gamepad");
            break;
        }
        m_modeButton->setIcon(SAIcon(modeStr));
        sendTogglePlaybackModeMessage();
    });
    //connect(muteButton, SIGNAL(clicked()), this, SLOT(muteClicked()));
    //connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SIGNAL(changeVolume(int)));
    //connect(rateBox, SIGNAL(activated(int)), SLOT(updateRate()));
}

void PlayerControls::layoutWidgets()
{
    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_stopButton);
    layout->addWidget(m_previousButton);
    layout->addWidget(m_playButton);
    layout->addWidget(m_nextButton);
    layout->addWidget(m_modeButton);
    //layout->addWidget(muteButton);
    //layout->addWidget(volumeSlider);
    //layout->addWidget(rateBox);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
}

void PlayerControls::setPlayIcon(bool isPlaying)
{
    //QStyle::StandardPixmap icon; 
    QString playIcon = QStringLiteral("play");
    if (isPlaying)
        playIcon = QStringLiteral("pause");

    m_playButton->setIcon(SAIcon(playIcon));
}


int PlayerControls::volume() const
{
    return m_volumeSlider ? m_volumeSlider->value() : 0;
}

void PlayerControls::setVolume(int volume)
{
    if (m_volumeSlider)
        m_volumeSlider->setValue(volume);
}

bool PlayerControls::isMuted() const
{
    return m_playerMuted;
}

void PlayerControls::setMuted(bool muted)
{
    if (muted != m_playerMuted) {
        m_playerMuted = muted;

        m_muteButton->setIcon(style()->standardIcon(muted
            ? QStyle::SP_MediaVolumeMuted
            : QStyle::SP_MediaVolume));
    }
}

void PlayerControls::playClicked()
{
    // Send play message
    sendTogglePlaybackMessage();
}

void PlayerControls::muteClicked()
{
    emit changeMuting(!m_playerMuted);
}

qreal PlayerControls::playbackRate() const
{
    return m_rateBox->itemData(m_rateBox->currentIndex()).toDouble();
}

void PlayerControls::processMessage(GPlaybackDataMessage* message)
{
    m_playbackDataMessage = *message;
    const GSimulationPlayState& playbackState = m_playbackDataMessage.getPlaybackState();
    setPlayIcon(ESimulationPlayState::ePlayedState == playbackState);

    QString chr;
    switch ((ESimulationPlayMode)m_playbackDataMessage.getPlaybackMode()) {
    case ESimulationPlayMode::eDebug:
        chr = FontIcon::FaUnicodeCharacter("gamepad").string();
        break;
    case ESimulationPlayMode::eStandard:
        chr = FontIcon::FaUnicodeCharacter("binoculars").string();
        break;
    }
    m_modeButton->setText(chr);
}

void PlayerControls::setPlaybackRate(float rate)
{
    for (int i = 0; i < m_rateBox->count(); ++i) {
        if (qFuzzyCompare(rate, float(m_rateBox->itemData(i).toDouble()))) {
            m_rateBox->setCurrentIndex(i);
            return;
        }
    }

    m_rateBox->addItem(QStringLiteral("%1x").arg(rate), QVariant(rate));
    m_rateBox->setCurrentIndex(m_rateBox->count() - 1);
}

//void PlayerControls::updateRate()
//{
//    emit changeRate(playbackRate());
//}


void rev::PlayerControls::sendRequestPlaybackDataMessage()
{
    m_manager->messageGateway()->copyAndQueueMessageForSend(m_requestPlaybackDataMessage);
}

void rev::PlayerControls::sendTogglePlaybackMessage()
{
    m_manager->messageGateway()->copyAndQueueMessageForSend(m_togglePlaybackMessage);
}

void rev::PlayerControls::sendTogglePlaybackModeMessage()
{
    m_manager->messageGateway()->copyAndQueueMessageForSend(m_playbackModeMessage);
}


// End namespaces        
}
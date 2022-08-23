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
#pragma once

// Qt
#include <QWidget>

// Internal
#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/types/GNameable.h"

#include "ripple/network/messages/GPlaybackDataMessage.h"
#include "ripple/network/messages/GRequestPlaybackDataMessage.h"
#include "ripple/network/messages/GTogglePlaybackMessage.h"
#include "ripple/network/messages/GTogglePlaybackModeMessage.h"

class QAbstractButton;
class QAbstractSlider;
class QComboBox;

namespace rev {

class CoreEngine;
class WidgetManager;

class PlayerControls: public QWidget, public rev::NameableInterface {
    Q_OBJECT
public:

    PlayerControls(WidgetManager* manager, QWidget *parent = 0);

    int volume() const;
    bool isMuted() const;
    qreal playbackRate() const;

    void processMessage(GPlaybackDataMessage* message);

    //Signal<> m_playSignal; ///< Signal for playing
    //Signal<> m_pauseSignal; ///< Signal for pausing


public slots:
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(float rate);

signals:
    void stop();
    void next();
    void previous();
    void changeVolume(int volume);
    void changeMuting(bool muting);
    void changeRate(qreal rate);


protected:

    void sendRequestPlaybackDataMessage();
    void sendTogglePlaybackMessage();
    void sendTogglePlaybackModeMessage();

private slots:
    void playClicked();
    void muteClicked();
    //void updateRate();

private:
    void initializeWidgets();
    void initializeConnections();
    void layoutWidgets();

    void setPlayIcon(bool isPlaying);

    bool m_playerMuted;
    QAbstractButton *m_playButton;
    QAbstractButton *m_stopButton;
    QAbstractButton *m_nextButton;
    QAbstractButton *m_previousButton;
    QAbstractButton *m_modeButton;
    QAbstractButton *m_muteButton;
    QAbstractSlider *m_volumeSlider;
    QComboBox *m_rateBox;

    WidgetManager* m_manager{ nullptr };

    GRequestPlaybackDataMessage m_requestPlaybackDataMessage{};
    GPlaybackDataMessage m_playbackDataMessage{};
    GTogglePlaybackMessage m_togglePlaybackMessage{};
    GTogglePlaybackModeMessage m_playbackModeMessage{};

    /// @}
};


// End namespaces        
}

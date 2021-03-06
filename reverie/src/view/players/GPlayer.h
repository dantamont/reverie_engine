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

#ifndef GB_PLAYERCONTROLS_H
#define GB_PLAYERCONTROLS_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QMediaPlayer>

// Internal
#include "../base/GTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////

class QAbstractButton;
class QAbstractSlider;
class QComboBox;

namespace rev {

class CoreEngine;

namespace View {

class PlayerControls : public Tool
{
    Q_OBJECT

public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    PlayerControls(CoreEngine* core, QWidget *parent = 0);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    int volume() const;
    bool isMuted() const;
    qreal playbackRate() const;

    /// @}

public slots:
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(float rate);

signals:
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void changeVolume(int volume);
    void changeMuting(bool muting);
    void changeRate(qreal rate);

private slots:
    void playClicked();
    void muteClicked();
    void updateRate();

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void initializeWidgets();
    void initializeConnections();
    void layoutWidgets();

    void setPlayIcon(bool isPlaying);

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    bool m_playerMuted;
    QAbstractButton *m_playButton;
    QAbstractButton *m_stopButton;
    QAbstractButton *m_nextButton;
    QAbstractButton *m_previousButton;
    QAbstractButton *m_modeButton;
    QAbstractButton *m_muteButton;
    QAbstractSlider *m_volumeSlider;
    QComboBox *m_rateBox;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // PLAYERCONTROLS_H
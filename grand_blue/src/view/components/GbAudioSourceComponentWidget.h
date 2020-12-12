#ifndef GB_AUDIO_SOURCE_COMPONENT_WIDGET_H
#define GB_AUDIO_SOURCE_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class AudioSourceComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioComponentWidget
/// @brief Widget representing a script component
class AudioComponentWidget : public ComponentWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AudioComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~AudioComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a audio source component
    AudioSourceComponent* audioComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Returns current index
    size_t populateAudioSources();

    /// @brief Populate values based on audio source
    void updateSourceWidgets();

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Audio source to use for the component
    QComboBox* m_audioSources;

    // "Global" settings, relating to audio resource and shared between audio components
    QCheckBox* m_isLooping;
    QCheckBox* m_singleInstance;
    QCheckBox* m_tickOnInaudible;
    QCheckBox* m_killOnInaudible;
    QCheckBox* m_listenerRelative;
    QCheckBox* m_useDistanceDelay;

    QLineEdit* m_minAttenDistance;
    QLineEdit* m_maxAttenDistance;
    QComboBox* m_attenuationModel;
    QLineEdit* m_attenuationRolloff;
    QLineEdit* m_sourceVolume;

    // "Local" settings, applying only for this component
    QCheckBox* m_isProtected;
    QCheckBox* m_isBackground;
    QCheckBox* m_is3d;

    QLineEdit* m_volume;
    QLineEdit* m_pan;
    QLineEdit* m_relativePlaySpeed;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
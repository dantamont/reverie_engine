#ifndef GB_LOAD_AUDIO_WIDGET_H
#define GB_LOAD_AUDIO_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class AudioSourceComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LoadAudioWidget
/// @brief Widget representing a script component
class LoadAudioWidget : public ParameterWidget{
    Q_OBJECT
public:
    friend class LoadAudioCommand;

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    LoadAudioWidget(CoreEngine* core, QWidget *parent = 0);
    ~LoadAudioWidget();

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

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief UUID of the audio resource handle
    Uuid m_audioHandleID = Uuid(false); // null UUID

    /// @brief Controls whether or not to stream audio file
    QCheckBox* m_streamAudio;

    /// @brief Path to the python script used by this component
    FileLoadWidget* m_fileWidget;
    QDialogButtonBox* m_confirmButtons;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "ripple/network/messages/GLoadAudioResourceMessage.h"

namespace rev {

class FileLoadWidget;

/// @class LoadAudioWidget
/// @brief Widget representing a script component
class LoadAudioWidget : public ParameterWidget{
    Q_OBJECT
public:
    friend class LoadAudioCommand;

    /// @name Constructors/Destructor
    /// @{

    LoadAudioWidget(WidgetManager* wm, QWidget *parent = 0);
    ~LoadAudioWidget();

    /// @}

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    GLoadAudioResourceMessage m_loadMessage; ///< Message for loading audio resource
    Uuid m_audioHandleId = Uuid(false); ///< UUID of the audio resource handle
    QCheckBox* m_streamAudio{ nullptr }; ///< Controls whether or not to stream audio file
    FileLoadWidget* m_fileWidget{ nullptr }; ///< Path to the python script used by this component
    QDialogButtonBox* m_confirmButtons{ nullptr }; ///< Confirm to load audio

    /// @}
};

// End namespaces        
}

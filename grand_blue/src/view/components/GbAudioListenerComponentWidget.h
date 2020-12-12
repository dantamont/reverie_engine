#ifndef GB_AUDIO_LISTENER_COMPONENT_WIDGET_H
#define GB_AUDIO_LISTENER_COMPONENT_WIDGET_H


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

class AudioListenerComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AudioListenerWidget
/// @brief Widget representing a script component
class AudioListenerWidget : public ComponentWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AudioListenerWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~AudioListenerWidget();

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
    AudioListenerComponent* audioListener() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Audio source to use for the component
    QLineEdit* m_velocityX;
    QLineEdit* m_velocityY;
    QLineEdit* m_velocityZ;

    QLineEdit* m_speedOfSound;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
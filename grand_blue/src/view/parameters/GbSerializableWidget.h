#ifndef GB_SERIALIZABLE_WIDGET_H
#define GB_SERIALIZABLE_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../parameters/GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class Serializable;

namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SerializableWidget
/// @brief Subclass of a JSON Widget that is specific to components
class SerializableWidget : public JsonWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    SerializableWidget(CoreEngine* core, Serializable* component, QWidget *parent = 0);
    ~SerializableWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() override;

    /// @brief What do do after reloading serializable
    virtual void postLoad() override;

    virtual void layoutWidgets() override;


    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    bool m_wasPlaying;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
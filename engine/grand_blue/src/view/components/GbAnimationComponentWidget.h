#ifndef GB_ANIMATION_COMPONENT_WIDGET_H
#define GB_ANIMATION_COMPONENT_WIDGET_H


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

class BoneAnimationComponent;

namespace View {
class EditableComboBox;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationComponentWidget
/// @brief Generic component widget that uses JSON data to modify components
class AnimationComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~AnimationComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    BoneAnimationComponent* animationComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    void repopulateStateMachineList();

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    EditableComboBox* m_stateMachineSelect;
    QPushButton* m_launchGraphButton;

    /// @}
};




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
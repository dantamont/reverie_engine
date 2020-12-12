#ifndef GB_STATE_WIDGET_H 
#define GB_STATE_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "../GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class BaseAnimationState;
class AnimationController;

namespace View {

class AnimationTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimStateWidget
class AnimStateWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AnimStateWidget(BaseAnimationState* state, AnimationController* controller, QWidget* parent = nullptr);
    virtual ~AnimStateWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override;

    /// @}

protected slots:

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    BaseAnimationState* m_state;
    AnimationController* m_controller;

    QLabel* m_label;
    AnimationTreeWidget* m_clipWidget;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif
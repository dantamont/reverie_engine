#ifndef GB_MOTION_WIDGET_H 
#define GB_MOTION_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "../GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class Motion;
class AnimationController;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimMotionWidget
class AnimMotionWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AnimMotionWidget(Motion* motion, AnimationController* controller, QWidget* parent = nullptr);
    virtual ~AnimMotionWidget();

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

    AnimationController* m_controller;
    Motion* m_motion;

    QLabel* m_label;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif
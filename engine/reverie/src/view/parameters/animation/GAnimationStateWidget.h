#ifndef GB_STATE_WIDGET_H 
#define GB_STATE_WIDGET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "../GParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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

    template<typename T>
    T* state() const {
        return dynamic_cast<T*>(m_state);
    }

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    BaseAnimationState* m_state;
    AnimationController* m_controller;

    QLabel* m_label;
    AnimationTreeWidget* m_clipWidget;

    QLabel* m_transitionInfo;
    QComboBox* m_transitionTypes;
    QLineEdit* m_fadeInTimeSec;
    QLineEdit* m_fadeInBlendWeight;
    QLineEdit* m_fadeOutTimeSec;
    QLineEdit* m_fadeOutBlendWeight;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif
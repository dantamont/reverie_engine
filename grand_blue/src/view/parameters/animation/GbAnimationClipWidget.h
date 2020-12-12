#ifndef GB_ANIM_CLIP_WIDGET 
#define GB_ANIM_CLIP_WIDGET
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
class AnimationClip;
class Animation;

namespace View {

class AnimationTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimClipWidget
class AnimClipWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AnimClipWidget(AnimationClip* clip, CoreEngine* core, QWidget* parent = nullptr);
    virtual ~AnimClipWidget();

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

    std::shared_ptr<Animation> getAnimation() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    AnimationClip* m_clip;

    QLineEdit* m_nameEdit;

    /// @brief Speed scaling of the clip
    QLineEdit* m_speedFactor;

    /// @brief In seconds
    QLineEdit* m_playbackDuration;

    QLineEdit* m_tickDelay;
    QLineEdit* m_timeDelay;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif
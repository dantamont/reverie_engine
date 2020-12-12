// See:
// https://www.studica.com/blog/game-design-tutorial-blend-trees-unity
// https://gamedev.stackexchange.com/questions/112143/when-to-use-a-blend-tree-vs-state-machine-for-animation
// https://www.gamasutra.com/view/feature/3456/animation_blending_achieving_.php

// Inverse Kinematics:
// http://soliduscode.com/using-jacobian-for-inverse-kinematics/
// http://soliduscode.com/iksolver-class/

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ANIMATION_CONTROLLER_H
#define GB_ANIMATION_CONTROLLER_H

// Internal
#include "GbBlendQueue.h"
#include "GbAnimStateMachine.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class AnimationTransition;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class AnimationController
/// @brief Class representing a set of controllable animations
class AnimationController : public Object, public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationController(CoreEngine* engine, const QJsonValue& json);
    AnimationController(CoreEngine* engine, const std::shared_ptr<ResourceHandle>& model);
    ~AnimationController();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    CoreEngine* engine() const { return m_engine; }

    const AnimationStateMachine* stateMachine() const { return m_stateMachine; }
    AnimationStateMachine* stateMachine() { return m_stateMachine; }
    void setStateMachine(AnimationStateMachine* stateMachine) {
        m_stateMachine = stateMachine;
    }

    const BlendQueue& blendQueue() const { return m_blendQueue; }
    BlendQueue& blendQueue() { return m_blendQueue; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Perform motion actions, and generate any new ones from auto-playing motions
    void updateMotions();

    /// @brief Return a pointer to the motion with the specified UUID
    Motion* getMotion(const Uuid& uuid);

    const std::vector<Motion>& motions() const { return m_motions; }

    /// @brief Add a motion to the animation controller at the specified state
    Motion* addMotion(BaseAnimationState* state);
    void removeMotion(Motion* motion);

    /// @brief Get model for the animation controller
    std::shared_ptr<Model> getModel() const;

    /// @brief Add state to the controller
    void addState(AnimationState* state);
    void addTransition(AnimationTransition* state);

    /// @property isPlaying
    bool isPlaying() const { return m_isPlaying; }
    void setPlaying(bool play) { m_isPlaying = play; }

    /// @brief Set uniforms in the given draw command
    bool bindUniforms(DrawCommand& drawCommand);
    bool bindUniforms(ShaderProgram& shaderProgram);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationController"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::AnimationController"; }
    /// @}

protected:

    friend class AnimationProcess;
    friend class BlendQueue;

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @

    /// @brief Vector of motions that need to be updated
    std::vector<MotionAction> m_motionActions;
    std::vector<MotionAction> m_motionActionQueue;

    /// @brief Whether or not the controller is playing
    bool m_isPlaying = true;

    /// @brief The model being used by the controller
    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;

    /// @brief Motions through animation state machine
    std::vector<Motion> m_motions;

    /// @brief The state machine for this controller
    AnimationStateMachine* m_stateMachine;

    /// @brief The blend queue for the controller
    BlendQueue m_blendQueue;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
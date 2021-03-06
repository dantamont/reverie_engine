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
#include "GBlendQueue.h"
#include "GAnimStateMachine.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class AnimationTransition;
class AnimationProcess;
class SceneObject;

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

    AnimationController(SceneObject* so, const QJsonValue& json);
    AnimationController(SceneObject* so, const std::shared_ptr<ResourceHandle>& model);
    ~AnimationController();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::shared_ptr<AnimationProcess>& process() { return m_process; }

    SceneObject* sceneObject() const { return m_sceneObject; }

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
    Motion* getMotion(const GString& name);


    const std::vector<Motion>& motions() const { return m_motions; }

    /// @brief Add a motion to the animation controller at the specified state
    Motion* addMotion(BaseAnimationState* state);
    void removeMotion(Motion* motion);

    /// @brief Get model for the animation controller
    Model* getModel() const;

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
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "AnimationController"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::AnimationController"; }
    /// @}

protected:
    friend class BoneAnimationComponent;
    friend class AnimationProcess;
    friend class BlendQueue;
    friend class Motion;

    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void initializeProcess(const SceneObject& so);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @

    std::mutex m_motionActionMutex;

    /// @brief Vector of motions that need to be updated
    std::vector<MotionAction> m_motionActions;
    std::vector<MotionAction> m_motionActionQueue;

    /// @brief Whether or not the controller is playing
    bool m_isPlaying = true;

    /// @brief The model being used by the controller
    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @brief Pointer to the scene object that this controller's component lives on
    SceneObject* m_sceneObject;

    /// @brief Motions through animation state machine
    std::vector<Motion> m_motions;

    /// @brief The state machine for this controller
    AnimationStateMachine* m_stateMachine;

    /// @brief The blend queue for the controller
    BlendQueue m_blendQueue;

    /// @brief The process for running the animation controller
    std::shared_ptr<AnimationProcess> m_process = nullptr;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
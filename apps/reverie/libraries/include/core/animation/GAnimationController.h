#pragma once

/// @see
// https://www.studica.com/blog/game-design-tutorial-blend-trees-unity
// https://gamedev.stackexchange.com/questions/112143/when-to-use-a-blend-tree-vs-state-machine-for-animation
// https://www.gamasutra.com/view/feature/3456/animation_blending_achieving_.php

// Inverse Kinematics:
/// @see
// http://soliduscode.com/using-jacobian-for-inverse-kinematics/
// http://soliduscode.com/iksolver-class/


// Internal
#include "GBlendQueue.h"
#include "GAnimStateMachine.h"
#include "core/rendering/shaders/GUniform.h"

namespace rev {

class AnimationTransition;
class AnimationProcess;
class SceneObject;

/// @class AnimationController
/// @brief Class representing a set of controllable animations
class AnimationController  {
public:
    /// @name Constructors/Destructor
    /// @{

    explicit AnimationController(SceneObject* so, const nlohmann::json& json);
    AnimationController(SceneObject* so, const std::shared_ptr<ResourceHandle>& model);
    ~AnimationController();

    /// @}

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
    bool applyUniforms(DrawCommand& drawCommand);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const AnimationController& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, AnimationController& orObject);


    /// @}


protected:
    friend class BoneAnimationComponent;
    friend class AnimationProcess;
    friend class BlendQueue;
    friend class Motion;

    /// @name Private Methods
    /// @{

    void initializeProcess(const SceneObject& so);

    /// @}

    /// @name Private Members
    /// @

    std::mutex m_motionActionMutex;

    struct AnimationUniforms {
        UniformData m_true; ///< Holds the boolean true
        UniformData m_globalInverseTransform; ///< The global inverse transform
        UniformData m_inverseBindPose; ///< The inverse bind pose
        UniformData m_boneTransforms; ///< The animation transforms
    };
    AnimationUniforms m_uniforms; ///< The uniforms for this controller's animation

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



} // End namespaces

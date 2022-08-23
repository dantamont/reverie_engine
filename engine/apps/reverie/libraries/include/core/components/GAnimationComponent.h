#ifndef GB_ANIMATION_COMPONENT
#define GB_ANIMATION_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GComponent.h"
#include "core/geometry/GCollisions.h"
#include "core/rendering/GGLFunctions.h"
#include "core/mixins/GRenderable.h"
#include "core/animation/GAnimationController.h"

// External
#include "fortress/layer/framework/GSignalSlot.h"
#include "ripple/network/messages/GAddedAnimationMotionMessage.h"
#include "ripple/network/messages/GRemovedAnimationMotionMessage.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class ShaderProgram;
class Uniform;
class AnimationController;
class ResourceHandle;
class DrawCommand;

class WorldMatrixVector;
template<typename WorldMatrixType>
class TransformTemplate;
typedef TransformTemplate<Matrix4x4> Transform;
typedef TransformTemplate<WorldMatrixVector> IndexedTransform;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class BoneAnimationComponent
class BoneAnimationComponent: public Component{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    BoneAnimationComponent();
    BoneAnimationComponent(const BoneAnimationComponent & other);
    BoneAnimationComponent(const std::shared_ptr<SceneObject>& object);
    ~BoneAnimationComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{


    /// @brief Enable the behavior of this component
    virtual void enable() override;

    /// @brief Disable the behavior of this component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @brief Update bounds given a transform
    void updateBounds(const IndexedTransform& transform);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Model
    const std::shared_ptr<ResourceHandle>& modelHandle() const;

    AnimationController& animationController() {
        return m_animationController;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const BoneAnimationComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, BoneAnimationComponent& orObject);


    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
       
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @todo Implement this to emit signal on AnimationController::addMotion and removeMotion calls
    //Signal<GAddedAnimationMotionMessage*> m_addedMotionSignal; ///< Called whenever a motion is added to an animation controller
    //Signal<GRemovedAnimationMotionMessage*> m_removedMotionSignal; ///< Called whenever a motion is removed from an animation controller


    /// @brief The animations used by this model component
    AnimationController m_animationController;


    /// @}


};
Q_DECLARE_METATYPE(BoneAnimationComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

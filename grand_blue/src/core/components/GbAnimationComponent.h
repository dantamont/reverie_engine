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
#include "GbComponent.h"
#include "../geometry/GbCollisions.h"
#include "../rendering/GbGLFunctions.h"
#include "../mixins/GbRenderable.h"
#include "../animation/GbAnimationController.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class ShaderProgram;
struct Uniform;
class AnimationController;
class AnimationProcess;
class ResourceHandle;
class DrawCommand;
class Transform;

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

    /// @brief Animation bounds in world space
    /// @details Bounds are calculated via updateBounds method by transforming skeleton's bounding box into world space
    const AABB& bounds() const { return m_transformedBoundingBox; }

    /// @brief Set uniforms for the animation component, which are specific to only one instance of a model
    void bindUniforms(DrawCommand& drawCommand);
    void bindUniforms(ShaderProgram& shaderProgram);

    /// @brief Enable the behavior of this component
    virtual void enable() override;

    /// @brief Disable the behavior of this component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @brief Update bounds given a transform
    void updateBounds(const Transform& transform);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::shared_ptr<AnimationProcess>& process() { return m_process; }

    /// @property Model
    const std::shared_ptr<ResourceHandle>& modelHandle() const;

    AnimationController& animationController() {
        return m_animationController;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "BoneAnimationComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::BoneAnimationComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize animation controller from model
    //void initializeAnimationController();

    void initializeProcess();

       
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief the bounding box for the animation component
    AABB m_transformedBoundingBox;

    /// @brief The animations used by this model component
    AnimationController m_animationController;


    /// @brief The process for running the animation controller
    std::shared_ptr<AnimationProcess> m_process = nullptr;


    /// @}


};
Q_DECLARE_METATYPE(BoneAnimationComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

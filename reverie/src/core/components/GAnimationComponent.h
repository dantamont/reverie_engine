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
#include "../geometry/GCollisions.h"
#include "../rendering/GGLFunctions.h"
#include "../mixins/GRenderable.h"
#include "../animation/GAnimationController.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class ShaderProgram;
struct Uniform;
class AnimationController;
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
    const char* namespaceName() const override { return "rev::BoneAnimationComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{
       
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The animations used by this model component
    AnimationController m_animationController;


    /// @}


};
Q_DECLARE_METATYPE(BoneAnimationComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

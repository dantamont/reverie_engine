#ifndef GB_MODEL_COMPONENT
#define GB_MODEL_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GbComponent.h"
#include "../rendering/GbGLFunctions.h"
#include "../mixins/GbRenderable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class ResourceHandle;
class ShaderProgram;
struct Uniform;
class DrawCommand;
class Camera;
struct SortingLayer;
class MainRenderer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class ModelComponent
class ModelComponent: public Component, public Shadable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ModelComponent();
    ModelComponent(const std::shared_ptr<SceneObject>& object);
    ~ModelComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Update bounds given a transform
    void updateBounds(const Transform& transform);

    /// @brief Create the draw commands for the model component
    void createDrawCommands(
        std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
        Camera& camera,
        ShaderProgram& shaderProgram);

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<BoundingBoxes>& bounds() const { return m_transformedBounds; }

    /// @property Model
    std::shared_ptr<Model> model() const;
    const std::shared_ptr<ResourceHandle>& modelHandle() const {
        return m_modelHandle;
    }
    void setModelHandle(const std::shared_ptr<ResourceHandle>& handle) {
        m_modelHandle = handle;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ModelComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ModelComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the model component, which are specific to only this instance of a model
    //void bindUniforms(DrawCommand& drawCommand);
       
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Transformed bounding geometry for all model chunks
    std::vector<BoundingBoxes> m_transformedBounds;

    /// @brief The model used by this model instance
    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @}


};
Q_DECLARE_METATYPE(ModelComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

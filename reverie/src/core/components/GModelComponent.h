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
#include "GComponent.h"
#include "../rendering/GGLFunctions.h"
#include "../mixins/GRenderable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Model;
class ResourceHandle;
class ShaderProgram;
struct Uniform;
class DrawCommand;
class AbstractCamera;
class CollidingGeometry;
struct SortingLayer;
class MainRenderer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class ModelComponent
class ModelComponent: public Component{
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

    /// @brief Create the draw commands for the model component
    /// @param[in] cullShape shape used for culling visible geometry (typically a camera frustum)
    void createDrawCommands(
        std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
        AbstractCamera& camera,
        ShaderProgram& shaderProgram,
        ShaderProgram* prepassProgram = nullptr);

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @brief Update scene object's bounds given a transform
    void updateBounds(const Transform & transform);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The render settings used for this model
    RenderSettings& renderSettings() { return m_renderSettings; }

    /// @property Model
    Model* model() const;
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
    const char* namespaceName() const override { return "rev::ModelComponent"; }

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

    /// @brief The model used by this model instance
    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @brief The render settings associated with this model
    RenderSettings m_renderSettings;

    /// @}


};
Q_DECLARE_METATYPE(ModelComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

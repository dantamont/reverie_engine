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
class ShaderProgram;
struct Uniform;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class ModelComponent
class ModelComponent: public Component, public Renderable {
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

    /// @brief Draw this model component
    void draw(const std::shared_ptr<Gb::ShaderProgram>& shaderProgram, 
        RenderSettings* settings = nullptr) override;

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    virtual void reload() override {}

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Model
    std::shared_ptr<Model> model() { return m_model; }
    void setModel(std::shared_ptr<Model> model) { m_model = model; }

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
    void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
    virtual void releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
   
    void drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings = nullptr) override;
    
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The model used by this model instance
    std::shared_ptr<Model> m_model;

    /// @}


};
Q_DECLARE_METATYPE(ModelComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

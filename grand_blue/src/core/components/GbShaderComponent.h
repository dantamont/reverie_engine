#ifndef GB_MATERIAL_COMPONENT
#define GB_MATERIAL_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GbComponent.h"
#include "../rendering/renderer/GbRenderSettings.h"
#include "../mixins/GbRenderable.h"
#include "../rendering/shaders/GbShaderPreset.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderPreset;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class ShaderComponent
/// @brief  A renderer component to be attached to a scene object
class ShaderComponent: public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ShaderComponent();
    ShaderComponent(const std::shared_ptr<SceneObject>& object);
    ~ShaderComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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

    std::shared_ptr<ShaderPreset> shaderPreset() const {
        if (std::shared_ptr<ShaderPreset> mtl = m_shaderPreset.lock())
            return mtl;
        else
            return nullptr;
    }

    void setShaderPreset(const std::shared_ptr<ShaderPreset>& preset) {
        m_shaderPreset = preset;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ShaderComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ShaderComponent"; }

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
    /// @name Protected Members
    /// @{

    std::weak_ptr<ShaderPreset> m_shaderPreset;

    /// @}


};
Q_DECLARE_METATYPE(ShaderComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

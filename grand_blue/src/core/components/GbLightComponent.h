//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_COMPONENT_H
#define GB_LIGHT_COMPONENT_H

// QT

// Internal
#include "GbComponent.h"
#include "../rendering/lighting/GbLight.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3g> Vec3List;
typedef std::vector<Vector4g> Vec4List;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Transform;
class SceneObject;
class Scene;
class Light;
class ShaderProgram;
class RenderProjection;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @class LightComponent
/// @brief A light object in GL
class LightComponent : public Component {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    LightComponent(); // For Qt metatype registration
    LightComponent(const std::shared_ptr<SceneObject>& object, Light::LightType type = Light::kPoint);
    LightComponent(const std::shared_ptr<SceneObject>& object, const Color& color, Light::LightType type = Light::kPoint);
    ~LightComponent();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const { return 1; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    Light& light();
    const Light& light() const;

    /// @brief unbind light buffer
    void unbindLightBuffer() const;

    /// @brief Enable this component
    virtual void enable() override;

    /// @brief Disable this component
    virtual void disable() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "LightComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::LightComponent"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class Gb::Transform;
    friend class Gb::Scene;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    QJsonValue lightAsJson() const;
    void loadLightFromJson(const QJsonValue& json);

    /// @brief Pointer to light
    int m_lightIndex = -1;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// Brief Cached intensity of the light for enabling and disabling
    real_g m_cachedIntensity;

    /// @}
};
Q_DECLARE_METATYPE(LightComponent)

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
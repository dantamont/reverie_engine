//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_COMPONENT_H
#define GB_LIGHT_COMPONENT_H

// QT

// Internal
#include "GComponent.h"
#include "../rendering/lighting/GLightSettings.h"
#include "../rendering/lighting/GShadowMap.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Transform;
class SceneObject;
class Scene;
//class Light;
class ShaderProgram;
class ShadowMap;
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
    ~LightComponent();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Cached light so that GL doesn't have to be pinged for attributes
    const Light& cachedLight() const { return m_cachedLight; }
    Light& cachedLight() { return m_cachedLight; }

    /// @brief Cached shadow so that GL doesn't have to be pinged for attributes
    const ShadowInfo& cachedShadow() const { return m_cachedShadow; }
    ShadowInfo& cachedShadow() { return m_cachedShadow; }


    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const { return 1; }

    /// @brief Get light type
    Light::LightType getLightType() const { return m_cachedLight.getType(); }

    /// @property Color
    const Vector4& getDiffuseColor() const { return m_cachedLight.getDiffuseColor(); }

    const Vector4& getAmbientColor() const { return m_cachedLight.getAmbientColor(); }

    const Vector4& getSpecularColor() const { return m_cachedLight.getSpecularColor(); }

    /// @property Intensity
    real_g getIntensity() const { return m_cachedLight.getIntensity(); }

    /// @property Attenuations
    const Vector4& getAttenuations() const { return m_cachedLight.attenuations(); }

    /// @property Range
    real_g getRange() const { return m_cachedLight.getRange(); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the position of the light in GL
    void setLightPosition(const Vector3& position);

    LightingSettings& lightingSettings() const;

    /// @brief Whether or not the light casts shadows
    bool castsShadows() const {
        return m_castShadows;
    }

    /// @brief unbind light buffer
    void updateLight();

    /// @brief Unbind shadow buffer
    void updateShadow();

    /// @brief Enable shadow casting for the light
    void enableShadowCasting(bool setStatus);

    /// @brief Disable shadow casting for the light
    void disableShadowCasting(bool setStatus);

    /// @brief Update shadow map's light-space matrix
    void updateShadowMap();

    /// @brief Enable this component
    virtual void enable() override;

    /// @brief Disable this component
    virtual void disable() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "LightComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::LightComponent"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class rev::Transform;
    friend class rev::Scene;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void clearLight();

    void initializeLight(Light::LightType type);

    QJsonValue lightAsJson() const;
    void loadLightFromJson(const QJsonValue& json);

    /// @brief Pointer to light
    int m_lightIndex = -1;

    /// @brief Cached version of light with current attributes
    Light m_cachedLight;

    /// @brief Cached version of the shadow with current attributes
    ShadowInfo m_cachedShadow;

    /// @brief The shadow map if the light is a shadow caster
    ShadowMap* m_shadowMap = nullptr;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// Brief Cached intensity of the light for enabling and disabling
    real_g m_cachedIntensity = -1;

    /// @brief whether or not shadows are being cast
    bool m_castShadows = false;

    /// @}
};
Q_DECLARE_METATYPE(LightComponent)

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
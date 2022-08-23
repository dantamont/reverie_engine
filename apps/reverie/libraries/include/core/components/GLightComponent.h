#pragma once

// Internal
#include "GComponent.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/lighting/GShadowMap.h"

namespace rev {

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

class CoreEngine;
class SceneObject;
class Scene;
class ShaderProgram;
class ShadowMap;
class RenderProjection;

template<typename WorldMatrixType>
class TransformTemplate;
typedef TransformTemplate<Matrix4x4> Transform;

/// @class LightComponent
/// @brief A light object in GL
class LightComponent : public Component {
public:
    /// @name Constructors/Destructor
    /// @{
    LightComponent(); // For Qt metatype registration
    LightComponent(const std::shared_ptr<SceneObject>& object, Light::LightType type = Light::kPoint);
    ~LightComponent();

    /// @}

    /// @name Public Methods
    /// @{

    Int32_t getLightIndex() const { return m_lightIndex;  }

    /// @brief Cached light so that GL doesn't have to be pinged for attributes
    const Light& cachedLight() const { return m_cachedLight; }

    /// @brief Cached shadow so that GL doesn't have to be pinged for attributes
    const ShadowInfo& cachedShadow() const { return m_cachedShadow; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const { return 1; }

    /// @brief Get light type
    Light::LightType getLightType() const { return m_cachedLight.getType(); }

    /// @property Color
    const Vector4& getDiffuseColor() const { return m_cachedLight.diffuseColor(); }
    void setDiffuseColor(const Vector4& color);

    const Vector4& getAmbientColor() const { return m_cachedLight.ambientColor(); }
    void setAmbientColor(const Vector4& color);

    const Vector4& getSpecularColor() const { return m_cachedLight.specularColor(); }
    void setSpecularColor(const Vector4& color);

    /// @property Intensity
    Real_t getIntensity() const { return m_cachedLight.getIntensity(); }

    /// @property Attenuations
    const Vector4& getAttenuations() const { return m_cachedLight.attenuations(); }

    /// @property Range
    Real_t getRange() const { return m_cachedLight.getRange(); }

    void setIntensity(Float32_t i);
    void setAttributes(const Vector4& attributes);
    void setRange(Float32_t range);

    /// @brief Set the type of light
    void setLightType(Light::LightType type);

    /// @brief Set the position of the light in GL
    void setLightPosition(const Vector3& position);

    LightingSettings& lightingSettings() const;

    /// @brief Whether or not the light casts shadows
    bool castsShadows() const {
        return m_castShadows;
    }

    /// @brief Update the far clip plane of the shadow
    /// @param farClip the value of the far clip plane distance
    void setShadowFarClipPlane(Real_t farClip);

    /// @brief Set the light-space matrix (view-projection matrix) of the shadow map camera
    /// @param lightMatrix the light-space matrix
    void setShadowLightMatrix(const Matrix4x4& lightMatrix);

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

    void loadLightFromJson(const nlohmann::json& json);

    /// @}

    
    /// @name Friend
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const LightComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, LightComponent& orObject);


    /// @}

protected:
    /// @name Friends
    /// @{

    friend class rev::TransformTemplate<Matrix4x4>;
    friend class rev::Scene;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Update light buffer
    void updateLight();

    /// @brief Update shadow buffer
    void updateShadow();

    void clearLight();

    void initializeLight(Light::LightType type);

    void lightAsJson(json& orJson) const;

    int m_lightIndex = -1; ///< Pointer to light
    Light m_cachedLight; ///< Cached version of light with current attributes
    ShadowInfo m_cachedShadow; ///< Cached version of the shadow with current attributes
    std::unique_ptr<ShadowMap> m_shadowMap; ///< The shadow map if the light is a shadow caster

    /// @}

    /// @name Protected Members
    /// @{

    Real_t m_cachedIntensity = -1; ///< Cached intensity of the light for enabling and disabling
    bool m_castShadows = false; ///< whether or not shadows are being cast

    /// @}
};
    
// End namespaces
}

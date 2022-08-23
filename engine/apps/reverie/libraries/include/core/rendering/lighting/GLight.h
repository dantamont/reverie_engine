#pragma once

// Internal
#include "fortress/containers/GColor.h"
#include "core/rendering/buffers/GShaderStorageBuffer.h"

// Buffer for clustered light rendering
#define LIGHT_BUFFER_NAME QStringLiteral("LightBuffer")

namespace rev {

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

class CoreEngine;
class ShaderProgram;
class RenderProjection;
class RenderContext;
class LightingSettings;


/// @class Light
/// @brief A light object in GL
class Light {
public:
    /// @name Static
    /// @{

    enum LightType {
        kPoint = 0,
        kDirectional,
        kSpot
    };

    enum LightFlags {
        kEnabled = 1 << 0 // Whether or not the light is enabled
    };

    /// @}

    /// @name Constructors/Destructor
    /// @{
    Light();
    ~Light();

    /// @}

    /// @name Properties
    /// @{

    /// @note Not needed for directional lights
    const Vector4& getPosition() const { return m_position; }
    void setPosition(const Vector3& position);

    /// @brief Used for spot and directional lights
    const Vector4& direction() const { return m_direction; }
    void setDirection(const Vector3& dir);

    /// @property Color
    const Vector4& diffuseColor() const { return m_diffuseColor; }
    const Vector4& ambientColor() const { return m_ambientColor; }
    const Vector4& specularColor() const { return m_specularColor; }

    /// @brief Return index of the light for use in shader
    int getIndex() const { return m_typeIndexFlags[1]; }
    void setIndex(int idx);

    void setColor(const Color& color);
    void setDiffuseColor(const Color& color);
    void setAmbientColor(const Color& color);
    void setSpecularColor(const Color& color);

    /// @property Attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    const Vector4& attributes() const { return m_attributes; }
    const Vector4& cutoffs() const { return m_attributes;  } // for spot light
    const Vector4& attenuations() const { return m_attributes; } // for point light
    void setAttributes(const Vector4& attr);

    LightType getType() const { return LightType(m_typeIndexFlags[0]); }
    void setType(LightType type);

    /// @property Intensity
    /// @brief Amplifies the brightness of the light
    Real_t getIntensity() const { return m_moreAttributes[0]; }
    void setIntensity(Real_t brightness);

    /// @property Range
    /// @brief Get range of the light
    Real_t getRange() const { return m_moreAttributes[1]; }
    void setRange(Real_t range);
    void setRangeFromCutoff(Real_t brightnessCutoff);

    int& flags() { return m_typeIndexFlags[2]; }

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Get FOV in radians if the light is a spot light
    float getFOVRad() const {
        if (getType() != Light::kSpot) {
            Logger::Throw("Error, cannot obtain FOV for non-spot light");
        }
        // Cutoff is stored as cos(angle), so find outer half-angle, and double
        return acos(m_attributes[1]) * 2.0;
    }

    void enable();

    void disable();

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class LightingSettings;
    friend class LightComponent;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Initialize static list entries for the light
    void initializeLight(RenderContext& context);

    /// @}

    /// @name Protected Members
    /// @{

    Vector4 m_position; ///< Position of the light
    Vector4 m_direction = {0.0, -1.0, 0.0, 0.0}; ///< Direction that light is casting onto

    /// @todo Use color class
    Vector4 m_ambientColor = { 0.0f, 0.0f, 0.0f, 1.0f }; ///< Ambient color channel
    Vector4 m_diffuseColor; ///< Diffuse color channel
    Vector4 m_specularColor = { 0.0f, 0.0f, 0.0f, 1.0f }; ///< Specular color channel

    /// @brief Light attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    Vector4 m_attributes;
    Vector4 m_moreAttributes = { 1, 100, 0, 0 }; ///< Light intensity, range
    Vector<int, 4> m_typeIndexFlags = { (int)kPoint, 0, (size_t)LightFlags::kEnabled, 0 }; ///< Light type, light index, Light flags

    /// @}

};

// End namespaces
}

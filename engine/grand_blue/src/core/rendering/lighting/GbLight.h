//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_H
#define GB_LIGHT_H

// QT

// Internal
#include "../../containers/GbColor.h"
#include "../buffers/GbShaderStorageBuffer.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer for clustered light rendering
#define LIGHT_BUFFER_NAME QStringLiteral("LightBuffer")

namespace Gb {

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

class ShaderProgram;
class RenderProjection;
class RenderContext;
class LightingSettings;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Light
/// @brief A light object in GL
class Light {
public:
    //---------------------------------------------------------------------------------------
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

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Light();
    ~Light();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @note Not needed for directional lights
    const Vector4& getPosition() const { return m_position; }
    void setPosition(const Vector3& position) { 
        m_position = Vector4(position, 1.0);
    }

    /// @brief Used for spot and directional lights
    Vector4& direction() { return m_direction; }
    const Vector4& getDirection() const { return m_direction; }
    void setDirection(const Vector3& dir) { m_direction = dir; }

    /// @property Color
    const Vector4& getDiffuseColor() const { return m_diffuseColor; }
    Vector4& diffuseColor() { return m_diffuseColor; }

    const Vector4& getAmbientColor() const { return m_ambientColor; }
    Vector4& ambientColor() { return m_ambientColor; }

    const Vector4& getSpecularColor() const { return m_specularColor; }
    Vector4& specularColor() { return m_specularColor; }

    /// @brief Return index of the light for use in shader
    int getIndex() const { return m_typeIndexFlags[1]; }
    void setIndex(int idx) {
        m_typeIndexFlags[1] = idx;
    }

    void setColor(const Color& color) {
        setDiffuseColor(color);
        setAmbientColor(color);
        setSpecularColor(color);
    }
    void setDiffuseColor(const Color& color) { m_diffuseColor = color.toVector4g(); }
    void setAmbientColor(const Color& color) { m_ambientColor = color.toVector4g(); }
    void setSpecularColor(const Color& color) {  m_specularColor = color.toVector4g(); }

    /// @property Attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    const Vector4& getAttributes() const { return m_attributes; }
    Vector4& attributes() { return m_attributes; }
    Vector4& cutoffs() { return m_attributes;  } // for spot light
    Vector4& attenuations() {  return m_attributes; } // for point light
    const Vector4& attenuations() const { return m_attributes; } // for point light
    void setAttributes(const Vector4& attr);

    ///// @brief Get second set of attributes
    //const Vector4g& getTypeIntensityAndIndex() const {
    //    return m_typeIntensityAndIndex;
    //}

    LightType getType() const { return LightType(m_typeIndexFlags[0]); }
    void setType(LightType type) { m_typeIndexFlags[0] = (int)type; }

    /// @property Intensity
    /// @brief Amplifies the brightness of the light
    real_g getIntensity() const { return m_moreAttributes[0]; }
    void setIntensity(real_g brightness) {
        m_moreAttributes[0] = brightness;
    }

    /// @property Range
    /// @brief Get range of the light
    real_g getRange() const { return m_moreAttributes[1]; }
    void setRange(real_g range) { m_moreAttributes[1] = range; }
    void setRangeFromCutoff(real_g brightnessCutoff);

    int& flags() { return m_typeIndexFlags[2]; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Get FOV in radians if the light is a spot light
    float getFOVRad() const {
        if (getType() != Light::kSpot) {
            throw("Error, cannot obtain FOV for non-spot light");
        }
        // Cutoff is stored as cos(angle), so find outer half-angle, and double
        return acos(m_attributes[1]) * 2.0;
    }

    void enable();

    void disable();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class LightingSettings;
    friend class LightComponent;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    //Light(LightType type);
    //Light(const Color& color, LightType type = kPoint);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Initialize static list entries for the light
    void initializeLight(RenderContext& context);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Position of the light
    Vector4 m_position;

    /// @brief Direction that light is casting onto
    Vector4 m_direction = {0.0, -1.0, 0.0, 0.0};

    /// @brief Color channels of the light
    Vector4 m_ambientColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector4 m_diffuseColor;
    Vector4 m_specularColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    /// @brief Light attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    Vector4 m_attributes;

    /// @brief Light intensity, range
    Vector4 m_moreAttributes = { 1, 100, 0, 0 };

    /// @brief  Light type, light index, Light flags
    Vector<int, 4> m_typeIndexFlags = { (int)kPoint, 0, (size_t)LightFlags::kEnabled, 0 };

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
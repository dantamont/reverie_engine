//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_H
#define GB_LIGHT_H

// QT

// Internal
#include "../../containers/GbColor.h"
#include "../shaders/GbShaderStorageBuffer.h"

#define LIGHT_SETTINGS_BUFFER_NAME QStringLiteral("LightSettingsBuffer")
#define LIGHT_BUFFER_NAME "LightBuffer"

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

class ShaderProgram;
class RenderProjection;
//class RenderContext;
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

    static int CreateLight(RenderContext& context);
    static int CreateLight(RenderContext& context, LightType type);
    static int CreateLight(RenderContext& context, const Color& color, LightType type = kPoint);

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
    Vector4g& getPosition() { return m_position; }
    void setPosition(const Vector3g& position) { m_position = position; }

    /// @brief Used for spot and directional lights
    Vector4g& direction() { return m_direction; }
    const Vector4g& getDirection() const { return m_direction; }
    void setDirection(const Vector3g& dir) { m_direction = dir; }

    /// @property Color
    const Vector4g& getDiffuseColor() const { return m_diffuseColor; }
    Vector4g& diffuseColor() { return m_diffuseColor; }

    const Vector4g& getAmbientColor() const { return m_ambientColor; }
    Vector4g& ambientColor() { return m_ambientColor; }

    const Vector4g& getSpecularColor() const { return m_specularColor; }
    Vector4g& specularColor() { return m_specularColor; }

    /// @brief Return index of the light for use in shader
    unsigned int getIndex() const { return m_typeIntensityAndIndex[2]; }

    void setDiffuseColor(const Color& color) { m_diffuseColor = color.toVector4g(); }
    void setAmbientColor(const Color& color) { m_ambientColor = color.toVector4g(); }
    void setSpecularColor(const Color& color) {  m_specularColor = color.toVector4g(); }

    /// @property Attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    const Vector4g& getAttributes() const { return m_attributes; }
    Vector4g& attributes() { return m_attributes; }
    Vector4g& cutoffs() { return m_attributes;  } // for spot light
    Vector4g& attenuations() {  return m_attributes; } // for point light
    void setAttributes(const Vector4g& attr) { m_attributes = attr; }

    /// @brief Get second set of attributes
    const Vector4g& getTypeIntensityAndIndex() const {
        return m_typeIntensityAndIndex;
    }

    LightType getType() const { return LightType((int)m_typeIntensityAndIndex[0]); }
    void setType(LightType type) { m_typeIntensityAndIndex[0] = (int)type; }

    /// @property Intensity
    /// @brief Amplifies the brightness of the light
    double getIntensity() const { return m_typeIntensityAndIndex[1]; }
    void setIntensity(real_g brightness) {  m_typeIntensityAndIndex[1] = brightness; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class LightingSettings;

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

    /// @brief Create and add light to static map
    static Light& AddLight(RenderContext& context);

    /// @brief Clear the light from static lists
    void clearLight(LightingSettings& ls);

    /// @brief Initialize static list entries for the light
    void initializeLight(RenderContext& context);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Position of the light
    Vector4g m_position;

    /// @brief Direction that light is casting onto
    Vector4g m_direction = {0.0, -1.0, 0.0, 0.0};

    /// @brief Color channels of the light
    Vector4g m_ambientColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    Vector4g m_diffuseColor;
    Vector4g m_specularColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    /// @brief Light attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, cutoffs
    Vector4g m_attributes;

    /// @brief Light type, light intensity, and light index
    Vector4g m_typeIntensityAndIndex;

    /// @}

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightSettings
class LightingSettings {
public:

    enum LightingModel {
        kPhong, // Most basic lighting model
        kBlinnPhong // More realistic than Phong
    };

    LightingSettings(RenderContext& context, size_t maxNumLights = 1024);
    ~LightingSettings();

    /// @brief Clears a light
    void clearLight(int lightIndex);

    /// @brief Check that there aren't too many lights
    void checkLights() const;

    /// @brief Clear all light data
    void clearLights();

    /// @brief Light data
    Light* lightData();

    /// @biref Light buffer
    ShaderStorageBuffer<Light>& lightBuffer() { return m_lights; }

    /// @brief Unbind light data
    void unbindLightData();

    /// @brief Count of lights
    unsigned int m_lightCount = 0;

    /// @brief Indices of deleted lights in attribute lists
    std::vector<int> m_deletedIndices;

    /// @brief The lighting model to use
    LightingModel m_lightingModel = LightingModel::kBlinnPhong;;

protected:

    /// @brief Buffer of light data for reference in shaders
    ShaderStorageBuffer<Light> m_lights;

    /// @brief The max number of lights allowed
    unsigned int m_maxLights;

    RenderContext& m_context;
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
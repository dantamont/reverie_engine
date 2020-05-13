//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_H
#define GB_LIGHT_H

// QT

// Internal
#include "GbComponent.h"
#include "../containers/GbColor.h"

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

class ShaderProgram;
namespace GL {
class RenderProjection;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @class Light
/// @brief A light object in GL
class Light: public Component {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum LightType {
        kPoint,
        kDirectional,
        kSpot
    };

    enum LightingModel {
        kPhong, // Most basic lighting model
        kBlinnPhong // More realistic than Phong
    };

    /// @brief Clear all static light data
    static void clearLights();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Light(); // For Qt metatype registration
    Light(const std::shared_ptr<SceneObject>& object, LightType type = kPoint);
    Light(const std::shared_ptr<SceneObject>& object, const Color& color, LightType type = kPoint);
    ~Light();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    void setPosition(const Vector3g& position) {
        updatePosition(position);
    }

    /// @brief Used for spot and directional lights
    void setDirection(const Vector3g& dir) {
        m_direction = dir;
        updateDirection();
    }

    /// @property Color
    const Color& getDiffuseColor() const { return m_diffuseColor; }
    const Color& getAmbientColor() const { return m_ambientColor; }
    const Color& getSpecularColor() const { return m_specularColor; }

    /// @brief Return index of the light for use in shader
    unsigned int getIndex() const { return m_index; }

    void setDiffuseColor(const Color& color) {
        m_diffuseColor = color;
        updateDiffuseColor();
    }
    void setAmbientColor(const Color& color) {
        m_ambientColor = color;
        updateAmbientColor();
    }
    void setSpecularColor(const Color& color) {
        m_specularColor = color;
        updateSpecularColor();
    }

    /// @property Attributes
    const Vector4g& getAttributes() const { return m_attributes; }
    void setAttributes(const Vector4g& attr) {
        m_attributes = attr;
        updateAttributes();
    }

    /// @brief Get second set of attributes
    Vector4g getTypeAndIntensity() const {
        return { float((int)m_lightType), m_intensity, 0, 0 };
    }

    LightType getType() const { return m_lightType; }
    void setType(LightType type) {
        m_lightType = type;
        updateType();
    }

    /// @property Intensity
    /// @brief Amplifies the brightness of the light
    double getIntensity() const { return m_intensity; }
    void setIntensity(real_g brightness, bool cache = true) { 
        m_intensity = brightness;
        if (cache) m_cachedIntensity = m_intensity;
        updateIntensity();
    }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const { return 1; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

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
    const char* className() const override { return "Light"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::Light"; }

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

    /// @brief Clear the light from static lists
    void clearLight();

    /// @brief Initialize static list entries for the light
    void initializeLight();

    /// @brief Update UBO for each attribute
    void updatePosition(const Vector3g& position) const;
    void updateDirection() const;
    void updateDiffuseColor() const;
    void updateAmbientColor() const;
    void updateSpecularColor() const;
    void updateAttributes() const;
    void updateIntensity() const;
    void updateType() const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    Vector4g m_direction;

    LightType m_lightType;

    /// @brief index of the light (to track for GL shaders)
    unsigned int m_index;

    /// @brief Brightness of the light
    real_g m_intensity;
    real_g m_cachedIntensity;

    /// @brief Light attributes
    /// @details 
    ///    For directional lights: empty
    ///    For point lights: constant, linear, and quadratic attenuations: attenuation = a1 + a2 * distance + a3 * distance^2
    ///        Default (1, 0, 0) is no attenuation (infinite light distance)
    ///    For spot lights, direction and cutoff
    Vector4g m_attributes;

    /// @brief Color channels of the light
    Color m_ambientColor;
    Color m_diffuseColor;
    Color m_specularColor;

    /// @brief Static count of lights
    static unsigned int LIGHT_COUNT;

    /// @brief Static vector of all lights
    static std::vector<Light*> LIGHTS;

    /// @brief Indices of deleted lights in attribute lists
    static std::vector<int> DELETED_INDICES;

    /// @brief The max number of lights allowed
    static unsigned int MAX_LIGHTS;
    
    /// @brief The lighting model to use
    static LightingModel LIGHTING_MODEL;

    /// @}
};
Q_DECLARE_METATYPE(Light)

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
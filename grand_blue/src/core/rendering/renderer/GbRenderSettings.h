#ifndef GB_RENDER_SETTINGS_H
#define GB_RENDER_SETTINGS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes
#include <vector>
#include <set>

// Qt

// Project
#include "../../mixins/GbLoadable.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "../../containers/GbContainerExtensions.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarationss
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderSettings;
class RenderContext;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class RenderSetting
/// @brief Represents a single setting that can be bound and released in GL
class RenderSetting: public Serializable{
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Type of GL setting
    enum SettingType {
        kCullFace = 0,
        kBlend,
        kDepth,
        kMAX_SETTING_TYPE
    };

    /// @brief Creat a render setting from JSON
    static std::shared_ptr<RenderSetting> Create(const QJsonValue& json);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderSetting();
    ~RenderSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual SettingType settingType() const = 0;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void bind(RenderContext& context);
    void release(RenderContext& context);

    /// @brief Whether this render setting has been set in GL
    bool isSet(RenderContext& context) const;

    /// @brief Set settings in OpenGL
    virtual void set(RenderContext& context) const = 0;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class RenderSettings;
    friend class RenderContext;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Set this as the current setting in RenderSettings cache
    void makeCurrent(RenderContext& context) const;

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) = 0;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{
    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class CulledFace {
    kFront = GL_FRONT,
    kBack = GL_BACK,
    kFrontAndBack = GL_FRONT_AND_BACK
};

/// @class CullFaceSetting
class CullFaceSetting : public RenderSetting {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum CullSettingFlag {
        kModifyEnable = 1 << 0, // Whether or not to modify if culling is enabled
        kModifyFace = 1 << 1 // Whether or not to modify the face that is culled
    };
    typedef QFlags<CullSettingFlag> CullFlags;

    static CullFaceSetting Current(RenderContext& context);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    CullFaceSetting();
    explicit CullFaceSetting(CulledFace culledFace);
    explicit CullFaceSetting(bool cull);
    CullFaceSetting(bool cull, CulledFace culledFace);
    ~CullFaceSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual SettingType settingType() const override {
        return SettingType::kCullFace;
    }


    bool isSettingEnabled() const { return m_cullFlags & kModifyEnable; }
    bool isSettingCullFace() const { return m_cullFlags & kModifyFace; }

    /// @property isCullFace
    bool cullFace() const { return m_cullFace; }
    void setIsCullFace(bool cull) { m_cullFace = cull; }

    /// @property culledFace
    CulledFace culledFace() const { return m_culledFace; }
    void setCulledFace(CulledFace culledFace) { m_culledFace = culledFace; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    bool operator==(const CullFaceSetting& other) const;
    bool operator!=(const CullFaceSetting& other) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    size_t m_cullFlags;

    /// @brief Whether or not to cull face
    bool m_cullFace;

    /// @brief Face to cull
    CulledFace m_culledFace;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlendSetting
// TODO: Implement convenience wrappers for transparency presets
class BlendSetting : public RenderSetting {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    BlendSetting();
    BlendSetting(bool blendEnabled, 
        int sourceAlphaFactor = GL_SRC_ALPHA, 
        int destAlphaFactor = GL_ONE_MINUS_SRC_ALPHA,
        int sourceRGBFactor = GL_ONE, 
        int destRGBFactor = GL_ZERO,
        int blendEquationAlpha = GL_FUNC_ADD,
        int blendEquationRGB = GL_FUNC_ADD);
    ~BlendSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    void setBlendColor(const Vector4& color) { m_blendColor = color; }

    virtual SettingType settingType() const override {
        return SettingType::kBlend;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Whether or not blending is enabled
    bool m_blendEnabled;

    /// @brief Source RGB factor (factor for what is currently stored in the shader)
    int m_sourceRGB;

    /// @brief Source Alpha (factor for what is currently stored in the shader)
    int m_sourceAlpha;

    /// @brief Destination RGB (factor for what is currently stored in the color buffer)
    int m_destinationRGB;

    /// @brief Destination Alpha(factor for what is currently stored in the color buffer)
    int m_destinationAlpha;

    /// @brief The blend equations
    int m_blendEquationAlpha;
    int m_blendEquationRGB;

    /// @brief Blend color (as floats from [0, 1]), GL default is (0, 0, 0, 0)
    Vector4 m_blendColor;

    /// @}
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class DepthPassMode {
    kNever = GL_NEVER,
    kLess = GL_LESS,
    kEqual = GL_EQUAL,
    kLessEqual = GL_LEQUAL,
    kGreater = GL_GREATER,
    kNotEqual = GL_NOTEQUAL,
    kGreaterEqual = GL_GEQUAL,
    kAlways = GL_ALWAYS
};

/// @class DepthSetting
class DepthSetting : public RenderSetting {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum DepthSettingFlag {
        kModifyWrite = 1 << 0, // Whether or not to modify write value
        kModifyTesting = 1 << 1, // Whether or not to toggle testing
        kModifyPassMode = 1 << 2 // Whether or not to modify pass mode
    };
    typedef QFlags<DepthSettingFlag> DepthFlags;

    static DepthSetting Current(RenderContext& context);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    DepthSetting();
    DepthSetting(bool enableTesting, DepthPassMode depthPassMode);
    DepthSetting(bool enableTesting, DepthPassMode depthPassMode, bool writeToDepthBuffer);
    DepthSetting(DepthPassMode depthPassMode, bool writeToDepthBuffer);
    ~DepthSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual SettingType settingType() const override {
        return SettingType::kDepth;
    }

    size_t depthFlags() { return m_depthFlags; }

    bool isSettingDepthWrite() const { return m_depthFlags & size_t(kModifyWrite); }
    bool isSettingTesting() const { return m_depthFlags & size_t(kModifyTesting); }
    bool isSettingPassMode() const { return m_depthFlags & size_t(kModifyPassMode); }

    bool isTestEnabled() const { return m_testEnabled; }
    void setTestEnabled(bool enabled) { m_testEnabled = enabled; }

    bool isWritingDepth() const { return m_writeToDepthBuffer; }
    void setWriteDepth(bool write) { m_writeToDepthBuffer = write; }

    DepthPassMode depthPassMode() const { return m_passMode; }
    void setDepthPassMode(DepthPassMode mode) { m_passMode = mode; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    bool operator==(const DepthSetting& other) const;
    bool operator!=(const DepthSetting& other) const;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    size_t m_depthFlags;

    /// @brief Whether or not depth testing is enabled
    bool m_testEnabled;

    /// @brief Whether or not to write to depth buffer
    bool m_writeToDepthBuffer;

    /// @brief The Depth Testing mode
    //  GL_ALWAYS 	The depth test always passes.
    //  GL_NEVER 	The depth test never passes.
    //  GL_LESS 	Passes if the fragment's depth value is less than the stored depth value.
    //  GL_EQUAL 	Passes if the fragment's depth value is equal to the stored depth value.
    //  GL_LEQUAL 	Passes if the fragment's depth value is less than or equal to the stored depth value.
    //  GL_GREATER 	Passes if the fragment's depth value is greater than the stored depth value.
    //  GL_NOTEQUAL 	Passes if the fragment's depth value is not equal to the stored depth value.
    //  GL_GEQUAL 	Passes if the fragment's depth value is greater than or equal to the stored depth value.
    DepthPassMode m_passMode;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderSettings
/// @brief Handles setting global GL settings when rendering
// TODO: Add bind modes
class RenderSettings : public Serializable{
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Cache settings from OpenGL
    static void CacheSettings(RenderContext& context);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderSettings(const QJsonValue& json);
    RenderSettings();
    ~RenderSettings();
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property ShapeMode
    size_t shapeMode() const { return m_shapeMode; }
    void setShapeMode(size_t shapeMode) { m_shapeMode = shapeMode; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Return setting of the specified type
    inline const RenderSetting* setting(RenderSetting::SettingType type) const {
        return m_settings[type];
    }

    /// @brief Override settings with specified render settings
    void overrideSettings(const RenderSettings& other);

    /// @brief Add default blend to the settings
    void addDefaultBlend();

    /// @brief Add a setting to toggle
    void addSetting(const RenderSetting& setting);

    template<typename T, typename ...Args>
    RenderSetting& addSetting(Args&&... args) {
        static_assert(std::is_base_of_v<RenderSetting, T>, "Error, T must be a RenderSetting subclass");
        auto setting = new T(std::forward<Args>(args)...);

        // Clear old setting, and set as new one
        if (m_settings[(int)setting->settingType()]) {
            delete m_settings[(int)setting->settingType()];
        }
        m_settings[(int)setting->settingType()] = setting;
        return *m_settings[(int)setting->settingType()];
    }

    /// @brief "Bind" the render settings, applying changes in openGL
    void bind(RenderContext& context);

    /// @brief "Release" the render settings, undoing changes to OpenGL
    void release(RenderContext& context);

    /// @brief Clear all settings
    void clearSettings();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief Vector of settings to toggle in GL
    std::array<RenderSetting*, RenderSetting::kMAX_SETTING_TYPE> m_settings;

    /// @brief Draw mode
    size_t m_shapeMode;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @class RenderSetting
/// @brief Represents a single setting that can be bound and released in GL
class RenderSetting: public Serializable, public std::enable_shared_from_this<RenderSetting> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Type of GL setting
    enum SettingType {
        kCullFace,
        kBlend,
        kDepth
    };

    /// @brief Creat a render setting from JSON
    static std::shared_ptr<RenderSetting> create(const QJsonValue& json);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    RenderSetting(SettingType type, int order);
    ~RenderSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    SettingType type() const { return m_settingType; }


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void bind() = 0;
    virtual void release();

    /// @brief Whether this render setting has been set in GL
    bool isSet() const;

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
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Return shared pointer to this
    std::shared_ptr<RenderSetting> sharedPtr() {
        return shared_from_this();
    }

    /// @brief Set this as the current setting in RenderSettings cache
    void makeCurrent();

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings() = 0;

    /// @brief Get previousu GL settings
    virtual void cachePreviousSettings();

    /// @brief Get GL functions from current contect
    QOpenGLFunctions* gl() {
        return QOpenGLContext::currentContext()->functions();
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The order to render, lower means higher priority (sooner)
    int m_order = 0;

    /// @brief The type of setting
    SettingType m_settingType;

    /// @brief The previous setting
    std::shared_ptr<RenderSetting> m_prevSetting = nullptr;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CullFaceSetting
class CullFaceSetting : public RenderSetting {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    CullFaceSetting();
    CullFaceSetting(bool cull, int culledFace = GL_BACK);
    ~CullFaceSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

        /// @property isCullFace
    void setIsCullFace(bool cull) { m_cullFace = cull; }

    /// @property culledFace
    void setCulledFace(int culledFace) { m_culledFace = culledFace; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void bind() override;

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
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Whether or not to cull face
    bool m_cullFace;

    /// @brief Face to cull
    int m_culledFace;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class BlendSetting
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

    void setBlendColor(const Vector4g& color) { m_blendColor = color; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void bind() override;

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
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings() override;

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
    Vector4g m_blendColor;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class DepthSetting
class DepthSetting : public RenderSetting {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    DepthSetting();
    DepthSetting(bool enable, int mode = GL_LESS);
    ~DepthSetting();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void bind() override;

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
    friend class RenderSettings;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Whether or not depth testing is enabled
    bool m_testEnabled;

    /// @brief The Depth Testing mode
    //  GL_ALWAYS 	The depth test always passes.
    //  GL_NEVER 	The depth test never passes.
    //  GL_LESS 	Passes if the fragment's depth value is less than the stored depth value.
    //  GL_EQUAL 	Passes if the fragment's depth value is equal to the stored depth value.
    //  GL_LEQUAL 	Passes if the fragment's depth value is less than or equal to the stored depth value.
    //  GL_GREATER 	Passes if the fragment's depth value is greater than the stored depth value.
    //  GL_NOTEQUAL 	Passes if the fragment's depth value is not equal to the stored depth value.
    //  GL_GEQUAL 	Passes if the fragment's depth value is greater than or equal to the stored depth value.
    int m_mode;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RenderSettings
/// @brief Handles setting global GL settings when rendering
// TODO: Add bind modes
class RenderSettings : public Serializable, public GL::OpenGLFunctions  {
public:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Modes to control binding duringdrawing routines
    /// @details Determines whether or not an element (e.g. shader, texture) is bound and released on draw
    enum BindMode {
        kBind = 0x1,
        kRelease = 0x2
    };

    struct BindSettings {
        QFlags<BindMode> m_shaderBindMode;
        QFlags<BindMode> m_materialBindMode;
    };

    /// @brief Struct for handling sorting of settings
    struct LessOrder {
        bool operator()(const std::shared_ptr<RenderSetting>& lhs, const std::shared_ptr<RenderSetting>& rhs) {
            return lhs->m_order < rhs->m_order;
        }
    };

    /// @brief Get the setting of the given type
    static std::shared_ptr<RenderSetting> current(RenderSetting::SettingType type);

    /// @brief Map of GL settings (to avoid get calls to GL, which are slow)
    static std::unordered_map<RenderSetting::SettingType, std::shared_ptr<RenderSetting>> CURRENT_SETTINGS;

    /// @brief Cache settings from OpenGL
    static void cacheSettings();

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

    /// @brief Set material bind/release flags
    inline bool hasMaterialFlag(BindMode bindMode) {
        return m_bindSettings.m_materialBindMode.testFlag(bindMode);
    }
    inline void setMaterialBind(bool bind) { m_bindSettings.m_materialBindMode.setFlag(kBind, bind); }
    inline void setMaterialRelease(bool rel) { m_bindSettings.m_materialBindMode.setFlag(kRelease, rel); }
    
    /// @brief Set shader program bind/release flags
    inline bool hasShaderFlag(BindMode bindMode) {
        return m_bindSettings.m_shaderBindMode.testFlag(bindMode);
    }
    inline void setShaderBind(bool bind) { m_bindSettings.m_shaderBindMode.setFlag(kBind, bind); }
    inline void setShaderRelease(bool rel) { m_bindSettings.m_shaderBindMode.setFlag(kRelease, rel); }


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Add default blend to the settings
    void addDefaultBlend();

    /// @brief Add a setting to toggle
    void addSetting(std::shared_ptr<RenderSetting> setting);

    /// @brief "Bind" the render settings, applying changes in openGL
    void bind();

    /// @brief "Release" the render settings, undoing changes to OpenGL
    void release();

    /// @brief Clear all settings
    void clearSettings();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private methods
    /// @{
    
    /// @brief Whether or not this object contains the given setting type
    inline bool hasSetting(std::shared_ptr<RenderSetting> setting) {
        auto settingIter = std::find_if(m_settings.begin(), m_settings.end(), 
            [&](std::shared_ptr<RenderSetting> s) {
            return s->type() == setting->type();
        });
        if (settingIter == m_settings.end()) {
            return false;
        }
        else {
            return true;
        }
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief Ordered multi-set of settings to toggle in GL
    std::multiset<std::shared_ptr<RenderSetting>, LessOrder> m_settings;

    /// @brief Draw mode
    size_t m_shapeMode;

    /// @brief Settings for binding materials, shaders, etc.
    BindSettings m_bindSettings;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

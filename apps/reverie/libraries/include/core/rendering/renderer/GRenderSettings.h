#pragma once

// Standard Includes
#include <vector>
#include <set>

// Qt

// Project
#include "fortress/types/GLoadable.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/shaders/GUniform.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {

class RenderSettings;
class RenderContext;

/// @brief Type of GL setting
enum RenderSettingType {
    kCullFace = 0,
    kBlend,
    kDepth,
    kStencil,
    kMAX_SETTING_TYPE
};

/// @class RenderSetting
/// @brief Represents a single setting that can be bound and released in GL
class RenderSetting {
public:
    /// @name Static
    /// @{

    /// @brief Creat a render setting from JSON
    static std::shared_ptr<RenderSetting> Create(const nlohmann::json& json);

    /// @}

    /// @name Constructors and Destructors
    /// @{
    RenderSetting();
    ~RenderSetting();

    /// @}

    /// @name Properties
    /// @{

    virtual RenderSettingType settingType() const = 0;


    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Properly cast to subclass for JSON Serialization
    json asJson() const;

    void bind(RenderContext& context);
    void release(RenderContext& context);

    /// @brief Whether this render setting has been set in GL
    bool isSet(RenderContext& context) const;

    /// @brief Set settings in OpenGL
    virtual void set(RenderContext& context) const = 0;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const RenderSetting& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, RenderSetting& orObject);


    /// @}

protected:
    friend class RenderSettings;
    friend class RenderContext;

    /// @name Protected methods
    /// @{

    /// @brief Set this as the current setting in RenderSettings cache
    void makeCurrent(RenderContext& context) const;

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) = 0;

    /// @}

    /// @name Protected members
    /// @{
    /// @}
};

enum class CulledFace {
    kFront = GL_FRONT,
    kBack = GL_BACK,
    kFrontAndBack = GL_FRONT_AND_BACK
};

/// @class CullFaceSetting
class CullFaceSetting : public RenderSetting {
public:
    /// @name Static
    /// @{

    enum CullSettingFlag {
        kModifyEnable = 1 << 0, // Whether or not to modify if culling is enabled
        kModifyFace = 1 << 1 // Whether or not to modify the face that is culled
    };
    typedef QFlags<CullSettingFlag> CullFlags;

    static CullFaceSetting Current(RenderContext& context);

    /// @}

    /// @name Constructors and Destructors
    /// @{
    CullFaceSetting();
    explicit CullFaceSetting(CulledFace culledFace);
    explicit CullFaceSetting(bool cull);
    CullFaceSetting(bool cull, CulledFace culledFace);
    ~CullFaceSetting();

    /// @}

    /// @name Properties
    /// @{

    virtual RenderSettingType settingType() const override {
        return RenderSettingType::kCullFace;
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

    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    /// @name Operators
    /// @{

    bool operator==(const CullFaceSetting& other) const;
    bool operator!=(const CullFaceSetting& other) const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CullFaceSetting& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CullFaceSetting& orObject);


    /// @}

protected:
    friend class RenderSettings;

    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

    /// @name Protected members
    /// @{

    size_t m_cullFlags;

    /// @brief Whether or not to cull face
    bool m_cullFace;

    /// @brief Face to cull
    CulledFace m_culledFace;

    /// @}
};

// TODO: Unused, implement
enum class TransparencyRenderMode {
    kOpaque = 0,
    kTransparentNormal,
    kTransparentAdditive, // Adds colors together, so result will be brighter than either
    kTransparentSubtractive // Darkens colors behind the material by subtracting the material's colors from the background colors
};

/// @class BlendSetting
// TODO: Implement convenience wrappers for transparency presets
class BlendSetting : public RenderSetting {
public:
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

    /// @name Properties
    /// @{

    void setBlendColor(const Vector4& color) { m_blendColor = color; }

    virtual RenderSettingType settingType() const override {
        return RenderSettingType::kBlend;
    }

    /// @}

    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const BlendSetting& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, BlendSetting& orObject);


    /// @}

protected:
    friend class RenderSettings;

    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

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

    /// @name Constructors and Destructors
    /// @{
    DepthSetting();
    DepthSetting(bool enableTesting, DepthPassMode depthPassMode);
    DepthSetting(bool enableTesting, DepthPassMode depthPassMode, bool writeToDepthBuffer);
    DepthSetting(DepthPassMode depthPassMode, bool writeToDepthBuffer);
    ~DepthSetting();

    /// @}

    /// @name Properties
    /// @{

    virtual RenderSettingType settingType() const override {
        return RenderSettingType::kDepth;
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

    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    /// @name Operators
    /// @{

    bool operator==(const DepthSetting& other) const;
    bool operator!=(const DepthSetting& other) const;

    /// @}


    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const DepthSetting& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, DepthSetting& orObject);


    /// @}

protected:
    friend class RenderSettings;

    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

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

enum class StencilPassMode {
    kNever = GL_NEVER,
    kLess = GL_LESS,
    kEqual = GL_EQUAL,
    kLessEqual = GL_LEQUAL,
    kGreater = GL_GREATER,
    kNotEqual = GL_NOTEQUAL,
    kGreaterEqual = GL_GEQUAL,
    kAlways = GL_ALWAYS
};

enum class StencilBufferOp {
    kKeep = GL_KEEP,
    kZero = GL_ZERO,
    kReplace = GL_REPLACE,
    kIncr = GL_INCR,
    kIncrWrap = GL_INCR_WRAP,
    kDecr = GL_DECR,
    kDecrWrap = GL_DECR_WRAP,
    kInvert = GL_INVERT
};

/// @class StencilSetting
class StencilSetting : public RenderSetting {
public:
    /// @name Static
    /// @{

    enum StencilMask {
        kDisabled = 0x00, // Each bit ends up as 0 in the stencil buffer (disabling writes)
        kEnabled = 0xFF // Each bit is written to the stencil buffer as is
    };

    enum StencilSettingFlag {
        kModifyWrite = 1 << 0, // Whether or not to modify write value
        kModifyTesting = 1 << 1, // Whether or not to toggle testing
        kModifyPassMode = 1 << 2, // Whether or not to modify pass mode
        kModifyOp = 1 << 3 // Whether or not to call glStencilOp, which sets how stencil buffer updates
    };
    typedef QFlags<StencilSettingFlag> StencilFlags;

    static StencilSetting Current(RenderContext& context);

    /// @}

    /// @name Constructors and Destructors
    /// @{
    StencilSetting();
    ~StencilSetting();

    /// @}

    /// @name Properties
    /// @{

    virtual RenderSettingType settingType() const override {
        return RenderSettingType::kStencil;
    }


    bool isSettingStencilWrite() const { return m_stencilFlags & size_t(kModifyWrite); }
    bool isSettingTesting() const { return m_stencilFlags & size_t(kModifyTesting); }
    bool isSettingPassMode() const { return m_stencilFlags & size_t(kModifyPassMode); }
    bool isSettingOp() const { return m_stencilFlags & size_t(kModifyOp); }

    bool isTestEnabled() const { return m_testEnabled; }
    void setTestEnabled(bool enabled) { m_testEnabled = enabled; }

    bool isWritingStencil() const { return m_mask != StencilMask::kDisabled; }
    void setWriteStencil(bool write) { m_mask = StencilMask::kEnabled; }

    StencilPassMode stencilPassMode() const { return m_passMode; }
    void setStencilPassMode(StencilPassMode mode) { m_passMode = mode; }


    /// @}

    /// @name Public Methods
    /// @{

    virtual void set(RenderContext& context) const override;

    /// @}

    /// @name Operators
    /// @{

    bool operator==(const StencilSetting& other) const;
    bool operator!=(const StencilSetting& other) const;

    /// @}


    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const StencilSetting& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, StencilSetting& orObject);


    /// @}

protected:
    friend class RenderSettings;

    /// @name Protected methods
    /// @{

    /// @brief Cache GL settings for use within application
    virtual void cacheSettings(RenderContext& context) override;

    /// @}

    /// @name Protected members
    /// @{

    size_t m_stencilFlags;

    /// @brief Whether or not stencil testing is enabled
    bool m_testEnabled;

    GLuint m_mask = (GLuint)(StencilMask::kEnabled);

    // The stencil test passing mode
    //  GL_ALWAYS 	The stencil test always passes.
    //  GL_NEVER 	The stencil test never passes.
    //  GL_LESS 	Passes if the fragment's stencil value is less than the reference value.
    //  GL_EQUAL 	Passes if the fragment's stencil value is equal to the reference value.
    //  GL_LEQUAL 	Passes if the fragment's stencil value is less than or equal to the reference stencil value.
    //  GL_GREATER 	Passes if the fragment's stencil value is greater than the reference stencil value.
    //  GL_NOTEQUAL 	Passes if the fragment's stencil value is not equal to the reference stencil value.
    //  GL_GEQUAL 	Passes if the fragment's stencil value is greater than or equal to the reference stencil value.
    StencilPassMode m_passMode = StencilPassMode::kEqual;

    // Reference value
    GLint m_ref = 1;

    // Control how to actually update the stencil buffer
    // GL_KEEP 	    The currently stored stencil value is kept.
    // GL_ZERO 	    The stencil value is set to 0.
    // GL_REPLACE 	The stencil value is replaced with the reference value set with glStencilFunc.
    // GL_INCR 	    The stencil value is increased by 1 if it is lower than the maximum value.
    // GL_INCR_WRAP Same as GL_INCR, but wraps it back to 0 as soon as the maximum value is exceeded.
    // GL_DECR 	    The stencil value is decreased by 1 if it is higher than the minimum value.
    // GL_DECR_WRAP Same as GL_DECR, but wraps it to the maximum value if it ends up lower than 0.
    // GL_INVERT 	Bitwise inverts the current stencil buffer value.

    /// @brief What to do if stencil test failes
    StencilBufferOp m_failOp;

    /// @brief What to do if stencil test passes, but the depth test fails
    StencilBufferOp m_depthFailOp;

    /// @brief What to do if stencil and depth test both pass
    StencilBufferOp m_passOp;

    /// @}
};

enum class PrimitiveMode {
    kPoints = GL_POINTS,
    kLines = GL_LINES,
    kLineStrip = GL_LINE_STRIP,
    kLineLoop = GL_LINE_LOOP,
    kTriangles = GL_TRIANGLES,
    kTriangleStrip = GL_TRIANGLE_STRIP,
    kTriangleFan = GL_TRIANGLE_FAN
};

/// @class RenderSettings
/// @brief Handles setting global GL settings when rendering
class RenderSettings  {
public:

    /// @name Static
    /// @{

    /// @brief Cache settings from OpenGL
    static void CacheSettings(RenderContext& context);

    /// @}

    /// @name Constructors and Destructors
    /// @{
    RenderSettings(const nlohmann::json& json);
    RenderSettings();
    ~RenderSettings();
    /// @}

    /// @name Properties
    /// @{

    // TODO: Used for depth sorting, fully implement
    const TransparencyRenderMode& transparencyType() const { return m_transparencyMode; }
    void setTransparencyType(const TransparencyRenderMode& type) {
        m_transparencyMode = type;
    }

    /// @brief The number of instances to draw
    const int instanceCount() const { return m_numInstances; }
    void setInstanceCount(int count) { m_numInstances = count; }

    /// @property ShapeMode
    PrimitiveMode shapeMode() const { return m_shapeMode; }
    void setShapeMode(PrimitiveMode shapeMode) { m_shapeMode = shapeMode; }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Return setting of the specified type
    inline const RenderSetting* setting(RenderSettingType type) const {
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
  
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const RenderSettings& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, RenderSettings& orObject);


    /// @}

private:
    /// @name Private members
    /// @{

    /// @brief Vector of settings to toggle in GL
    std::array<RenderSetting*, RenderSettingType::kMAX_SETTING_TYPE> m_settings;

    /// @brief Draw mode
    PrimitiveMode m_shapeMode;

    /// @brief Number of instances to draw
    uint32_t m_numInstances;

    // TODO: Used for depth sorting, fully implement
    /// @brief The transparency mode, used to drive blend presets
    TransparencyRenderMode m_transparencyMode = TransparencyRenderMode::kOpaque;

    /// @}
};



} // end namespacing

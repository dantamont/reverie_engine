#pragma once

// Internal
#include "core/rendering/GGLFunctions.h"
#include "core/mixins/GRenderable.h"
#include "core/rendering/shaders/GShaderProgram.h"

namespace rev {

class SceneCamera;
class PostProcessingChain;
class ShaderPreset;
class FrameBuffer;
class RenderContext;

/// @class PostProcessingEffectStage
/// @brief Represents a single stage of a post-processing effect
class PostProcessingEffectStage: public Shadable{
public:

    /// @name Static
    /// @{

    enum class EffectType {
        kReinhardToneMapping, // HDR
        kExposureHDR, // HDR
        kContrast,
        kHorizontalBlur,
        kVerticalBlur,
        kBrightnessFilter,
        kBloom,
        kDepthToScreen,
        BUILTIN_TYPES_COUNT
    };

    /// @}

    /// @name Constructors/Destructor
    /// @{

    PostProcessingEffectStage(size_t type);
    virtual ~PostProcessingEffectStage();

    /// @}

    /// @name Properties
    /// @{

    const Shader& vertexShader() const { return m_vertexShader; }
    const Shader& fragmentShader() const { return m_fragmentShader; }

    /// @brief The effect type
    size_t getType() { return m_type; }

    /// @}

    /// @name Public Methods
    /// @{

    void loadVertexShader(const QString& filePath);
    void loadFragmentShader(const QString& filePath);

    /// @}

protected:
    /// @name Protected Members
    /// @{

    /// @brief Vertex Shader
    Shader m_vertexShader;

    /// @brief Fragment shader
    Shader m_fragmentShader;

    /// @brief Effect type
    size_t m_type;

    /// @}
};


/// @class PostProcessingEffect
class PostProcessingEffect: public NameableInterface, public IdentifiableInterface, protected gl::OpenGLFunctions {
public:
    /// @name Static
    /// @{
    
    enum EffectFlag {
        kSetCheckPoint = 1 << 0, // Blit to checkpoint buffer after this effect is applied
        kUseCheckPoint = 1 << 1, // Whether or not to bind the checkpoint texture for effect
        kUseCameraTexture = 1 << 2, // Whether or not to use the unprocessed scene texture
    };

    /// @}

    /// @name Constructors/Destructor
    /// @{
    PostProcessingEffect();
    PostProcessingEffect(const QString& name);
    virtual ~PostProcessingEffect();

    /// @}

    /// @name Properties
    /// @{

    bool hasCheckPoint() const { return m_effectFlags.testFlag(kSetCheckPoint); }
    void setCheckPoint(bool sc) { m_effectFlags.setFlag(kSetCheckPoint, sc); }

    bool usesCheckPoint() const { return m_effectFlags.testFlag(kUseCheckPoint); }
    void useCheckPoint(bool use) {
        m_effectFlags.setFlag(kUseCheckPoint, use);
    }

    bool usesCameraTexture() const { return m_effectFlags.testFlag(kUseCameraTexture); }
    void useCameraTexture(bool use) {
        m_effectFlags.setFlag(kUseCameraTexture, use);
    }

    const std::shared_ptr<ShaderPreset>& shaderPreset() const { return m_shaderPreset; }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Add a post-processing stage to the effect
    void addStage(const std::shared_ptr<PostProcessingEffectStage>& stage);

    /// @brief Initialize the post-processing effect
    void reinitialize(CoreEngine* core);

    /// @brief Render effect to quad
    virtual void applyEffect(PostProcessingChain& chain);

    /// @}

protected:
    /// @name Protected Members
    /// @{

    QFlags<EffectFlag> m_effectFlags; ///< Flags associated with this effect
    std::vector<std::shared_ptr<PostProcessingEffectStage>> m_stages; ///< Different stages within this effect, to combine into a shader program
    std::shared_ptr<ShaderPreset> m_shaderPreset; ///< The shader preset/program representing this effect

    /// @}
};
    
// End namespaces
}

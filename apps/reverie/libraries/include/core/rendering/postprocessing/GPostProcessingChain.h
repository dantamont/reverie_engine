#pragma once

// Internal
#include "core/rendering/GGLFunctions.h"
#include "fortress/types/GLoadable.h"

namespace rev {

class SceneCamera;
class PostProcessingEffect;
class ShaderProgram;
class ShaderPreset;
class RenderContext;
class FrameBuffer;

/// @class PostProcessingChain
class PostProcessingChain {
public:

    /// @name Constructors/Destructor
    /// @{

    PostProcessingChain(SceneCamera* camera, RenderContext* context);
    ~PostProcessingChain();

    /// @}
    
    /// @name Properties
    /// @{

    SceneCamera* camera() { return m_camera; }

    /// @brief Get checkpoint buffer
    FrameBuffer& checkpointBuffer() {
        return *m_checkpointBuffer;
    }

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Clear buffers
    void clearBuffers();

    /// @brief Switch read and write framebuffer
    void pingPongBuffers();

    /// @brief Get read framebuffer
    FrameBuffer& readBuffer();

    /// @brief Get write framebuffer
    FrameBuffer& writeBuffer();

    /// @brief Add an effect to the chain
    void addEffect(const std::shared_ptr<PostProcessingEffect>& effect);

    /// @brief Apply post-processing to the camera's framebuffer
    void postProcess();

    /// @brief Clear the chain of any effects
    void clear();

    /// @brief Whether or not the chain has any effects
    bool isEmpty() const;

    /// @brief Resize the post-processing chain's framebuffers
    void resize(uint32_t w, uint32_t h);

    /// @brief Save effect to checkpoint buffer
    void saveCheckPoint();

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const PostProcessingChain& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, PostProcessingChain& orObject);


    /// @}

private:
    /// @name Private Methods
    /// @{

    void initialize(RenderContext* context);

    /// @}

    /// @name Private Members
    /// @{

    /// @brief The camera for which the post-processing chain is to be applied
    SceneCamera* m_camera;

    RenderContext* m_renderContext;

    /// @brief Framebuffer used as a checkpoint for effects to reference an earlier 
    /// pre-processing scene texture
    std::shared_ptr<FrameBuffer> m_checkpointBuffer;

    /// @brief The vector of FBOs to be used for post-processing that requires ping-pong
    std::vector<std::shared_ptr<FrameBuffer>> m_pingPongBuffers;

    /// @brief vector of all post processing effects present in the chain
    std::vector<std::shared_ptr<PostProcessingEffect>> m_effects;

    // Moved to effect
    /// @brief The shader presets representing stages in this post-processing chain
    /// @details Each effect may actually get it's own shader depending on the need to ping-pong, but
    /// many effects may be combinable into an uber shader
    //std::vector<std::shared_ptr<ShaderPreset>> m_shaderPresets;

    /// @brief Index of read framebuffer in fbo vector
    unsigned int m_readBufferIndex;

    /// @}
};

// End namespaces
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_POST_PROCESSING_CHAIN_H
#define GB_POST_PROCESSING_CHAIN_H

// standard

// Internal
#include "../GGLFunctions.h"
#include "../../GObject.h"
#include "../../mixins/GLoadable.h"
//#include "../view/GFrameBuffer.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class SceneCamera;
class PostProcessingEffect;
class ShaderProgram;
class ShaderPreset;
class RenderContext;
class FrameBuffer;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
//////////////////////////////////////////////////////////////////////////////////////////////////

/// @class PostProcessingChain
class PostProcessingChain : public Object, public Serializable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    PostProcessingChain(SceneCamera* camera, RenderContext* context);
    ~PostProcessingChain();

    /// @}
    
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    SceneCamera* camera() { return m_camera; }

    /// @brief Get checkpoint buffer
    FrameBuffer& checkpointBuffer() {
        return *m_checkpointBuffer;
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
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
    void resize(size_t w, size_t h);

    /// @brief Save effect to checkpoint buffer
    void saveCheckPoint();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    virtual const char* className() const override { return "PostProcessingChain"; }

    /// @property namespaceName
    virtual const char* namespaceName() const override { return "rev::PostProcessingChain"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    void initialize(RenderContext* context);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
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

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif
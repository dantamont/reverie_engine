#include "GPostProcessingEffect.h"
#include "GPostProcessingChain.h"
#include "../shaders/GShaderProgram.h"
#include "../shaders/GShaderPreset.h"
#include "../../components/GCameraComponent.h"
#include "../view/GFrameBuffer.h"
#include "../renderer/GRenderContext.h"

#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../containers/GFlags.h"
#include <core/rendering/renderer/GMainRenderer.h>

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PostProcessingEffectStage
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingEffectStage::PostProcessingEffectStage(size_t type) :
    m_type(type)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingEffectStage::~PostProcessingEffectStage()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PostProcessingEffectStage::loadVertexShader(const QString & filePath)
{
    // TODO: See if this is problematic with the way shaders are deleted
    m_vertexShader = Shader(filePath, Shader::ShaderType::kVertex, false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PostProcessingEffectStage::loadFragmentShader(const QString & filePath)
{
    // TODO: See if this is problematic with the way shaders are deleted
    m_fragmentShader = Shader(filePath, Shader::ShaderType::kFragment, false);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PostProcessingEffect
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingEffect::PostProcessingEffect()
{
    m_name = m_uuid.toString();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingEffect::PostProcessingEffect(const QString & name)
{
    m_name = name;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PostProcessingEffect::~PostProcessingEffect()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PostProcessingEffect::addStage(const std::shared_ptr<PostProcessingEffectStage>& stage)
{
    m_stages.push_back(stage);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PostProcessingEffect::reinitialize(CoreEngine* core)
{
    // TODO: Just make engine a singleton
    bool current = core->mainRenderer()->renderContext().isCurrent();
    if (!current) {
        throw("Error, render context is not current");
    }

    // Remove old shader program from resource cache
    if (m_shaderPreset) {
        if (m_shaderPreset->shaderProgram()) {
            Flags<ResourceDeleteFlag> deleteFlags = Flags<ResourceDeleteFlag>(ResourceDeleteFlag::kDeleteHandle);
            ResourceHandle* shaderHandle = m_shaderPreset->shaderProgram()->handle();
            shaderHandle->setRemovable(true);
            core->resourceCache()->remove(shaderHandle, deleteFlags);
        }
    }

    // Clear shader program from preset
    if (!m_shaderPreset) {
        m_shaderPreset = std::make_shared<ShaderPreset>(core, m_name);
    }
    else {
        m_shaderPreset->setShaderProgram(nullptr);
        m_shaderPreset->clearUniforms();
    }

    // Need to steal vertex source from quad shader program
    //std::shared_ptr<ShaderProgram> quadShaderProgram = core->resourceCache()->getHandleWithName("quad",
    //    ResourceType::kShaderProgram)->resourceAs<ShaderProgram>();


    /// Process shaders
    std::vector<const Shader*> vertexShaders;
    std::vector<const Shader*> fragmentShaders;
    for (const auto& stage : m_stages) {
        if (stage->vertexShader().isValid()) {
            vertexShaders.push_back(&stage->vertexShader());
        }
        if (stage->fragmentShader().isValid()) {
            fragmentShaders.push_back(&stage->fragmentShader());
        }
    }

    // Construct new shader program for quad effect
    QString fragSource = Shader::CombineEffectFragmentShaders(fragmentShaders).c_str();
    
    // TODO: Combine vertex shaders
    QString vertexSource = Shader::CombineEffectVertexShaders(vertexShaders).c_str();

    // Create new shader program
    auto handle = ResourceHandle::create(core, ResourceType::kShaderProgram);
    auto sp = std::make_unique<ShaderProgram>(vertexSource, fragSource, 1.0);
    handle->setName(m_name);
    handle->setResource(std::move(sp), false);
    handle->setIsLoading(true);

    // FIXME: See LoadProcess as well to fix this there, must initialize after handle is set
    handle->resourceAs<ShaderProgram>()->initializeShaderProgram(); // Does not initialize by default
    
    handle->setConstructed(true);
    handle->setRemovable(false);
    handle->setUnsaved(true); // Don't want to save the shader program to file
    handle->setCore(true); // For now, effect shaders are standard, and should be preserved
    m_shaderPreset->setShaderProgram(handle->resourceAs<ShaderProgram>());

    // Set default values for shader program
    // TODO
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PostProcessingEffect::applyEffect(PostProcessingChain& chain)
{
    if (!m_shaderPreset) {
#ifdef DEBUG_MODE
        logWarning("Shader preset not set for effect");
#endif
        return;
    }

    // Return if shader program not yet loaded
    ShaderProgram* sp = m_shaderPreset->shaderProgram();
    if (!sp) {
        return;
    }

    // TODO: Don't do this every time
    m_shaderPreset->queueUniforms();

    // Bind shader program
    sp->bind();
    sp->updateUniforms();

    // Bind camera scene texture for use (texture unit 3)
    if (usesCameraTexture()) {
        chain.camera()->frameBuffers().writeBuffer().bindColorTexture(3);
    }

    // Bind checkpoint scene texture for use (texture unit 4)
    if (usesCheckPoint()) {
        chain.checkpointBuffer().bindColorTexture(4);
    }

    // Bind and clear write framebuffer
    chain.writeBuffer().bind();
    chain.writeBuffer().clear();

    // Render effect into write buffer (binding color texture from read buffer and rendering with effect shader)
    chain.readBuffer().drawQuad(*chain.camera(), *sp);

    // Release write buffer, since drawing is done
    chain.writeBuffer().release();

    // Release shader program
    sp->release();

    // Switch read and write buffers for ping-pong approach
    chain.pingPongBuffers();

    // If is a checkpoint, blit to checkpoint buffer
    if (hasCheckPoint()) {
        chain.saveCheckPoint();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

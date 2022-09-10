#include "core/rendering/renderer/GRenderCommand.h"

#include "core/GCoreEngine.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/containers/GColor.h"
#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"
#include "core/rendering/materials/GMaterial.h"
#include <core/canvas/GGlyph.h>

#include "core/mixins/GRenderable.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GShaderStorageBuffer.h"
#include "core/components/GCameraComponent.h"
#include "fortress/containers/GSortingLayer.h"
#include "core/rendering/postprocessing/GPostProcessingChain.h"
#include "core/rendering/lighting/GShadowMap.h"

namespace rev {


RenderCommand::RenderCommand()
{
}




void DrawCommand::ResetDepths()
{
    s_farthestDepth = std::numeric_limits<float>::max();
    s_nearestDepth = -std::numeric_limits<float>::max();
}

DrawCommand::DrawCommand()
{
    // Reserve space for 5 uniforms (84 bytes each)
    m_mainUniforms.reserve(5);
    m_prepassUniforms.reserve(5);
}

DrawCommand::DrawCommand(Renderable& renderable, UniformContainer& uniformContainer, AbstractCamera& camera, int sceneObjectId):
    m_renderable(&renderable),
    m_camera(&camera),
    m_uniformContainer(&uniformContainer),
    m_sceneObjectId(sceneObjectId)
{
    // Reserve space for 5 uniforms (84 bytes each)
    m_mainUniforms.reserve(5);
    m_prepassUniforms.reserve(5);

    // Set uniforms from renderable
    renderable.setUniforms(*this);
}

DrawCommand::DrawCommand(Renderable & renderable, ShaderProgram& program, UniformContainer& uniformContainer, AbstractCamera& camera, int sceneObjectId, ShaderProgram* prepassShaderProgram) :
    m_renderable(&renderable),
    m_shaderProgram(&program),
    m_uniformContainer(&uniformContainer),
    m_camera(&camera),
    m_prepassShaderProgram(prepassShaderProgram),
    m_sceneObjectId(sceneObjectId)
{
    // Reserve space for 5 uniforms (84 bytes each)
    m_mainUniforms.reserve(5);
    m_prepassUniforms.reserve(5);

    // Set uniforms from renderable
    renderable.setUniforms(*this);
}

void DrawCommand::setShaderPrograms(ShaderProgram* prepassShader, ShaderProgram* mainShader)
{
    m_prepassShaderProgram = prepassShader;
    m_shaderProgram = mainShader;
}


void DrawCommand::perform(OpenGlRenderer& renderer, uint32_t commandIndex)
{
    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    // Bind shader 
    updateShaderUniforms(renderer, *m_shaderProgram, false, commandIndex);

    // Perform actual render
    // Need to pass in render context to bind shadow map textures
    m_renderable->draw(*m_shaderProgram, 
        &renderer.m_renderContext, 
        &m_renderSettings,
        0);

    // Set render key to this key
    renderer.m_renderState.m_lastRenderedKey = m_sortKey;
}

void DrawCommand::addUniform(const UniformData& uniformData, Int32_t mainUniformId, Int32_t prepassUniformId)
{
    m_mainUniforms.emplace_back(mainUniformId, uniformData);
    if (m_prepassShaderProgram) {
#ifdef DEBUG_MODE
        assert(prepassUniformId > -1 && "Error, must specify prepassUniformId");
#endif
        m_prepassUniforms.emplace_back(prepassUniformId, uniformData);
    }
}

void DrawCommand::setUniform(const UniformData& uniformData, Int32_t mainUniformId, Int32_t prepassUniformId)
{
    int uniformIndex;
    bool uniformExists = hasUniform(mainUniformId, &uniformIndex);
    if (!uniformExists) {
        addUniform(uniformData, mainUniformId, prepassUniformId);
    }
    else {
        m_mainUniforms[uniformIndex] = Uniform(mainUniformId, uniformData);
        if (m_prepassShaderProgram) {
            m_prepassUniforms[uniformIndex] = Uniform(prepassUniformId, uniformData);
#ifdef DEBUG_MODE
            assert(prepassUniformId > -1 && "Error, must specify prepassUniformId");
#endif
        }
    }
}

void DrawCommand::addUniforms(const std::vector<Uniform>& uniforms, const std::vector<Int32_t>& prepassUniformIds)
{
    m_mainUniforms.insert(m_mainUniforms.end(), uniforms.begin(), uniforms.end());
    if (m_prepassShaderProgram) {
        // Set the uniforms for the prepass shader, and correct the uniform ID
        Uint32_t startIndex = m_prepassUniforms.size();
        m_prepassUniforms.insert(m_prepassUniforms.end(), uniforms.begin(), uniforms.end());
        for (Uint32_t i = 0; i < prepassUniformIds.size(); i++) {
            Uniform& prepassUniform = m_prepassUniforms[startIndex + i];
            prepassUniform.setId(prepassUniformIds[i]);
        }
    }
}

void DrawCommand::onAddToQueue()
{
    /// @todo Create a SortKeyPacker to handle this

    size_t maxBit = (sizeof(unsigned long long) * 8) - 1;
    size_t currentBit = maxBit;

    // Ensure that world bounds are set, or has deferred geometry
    bool isDeferredGeometry = m_passFlags.testFlag(RenderablePassFlag::kDeferredGeometry);
    if (!m_renderableWorldBounds.boxData().isInitialized() && !isDeferredGeometry) {
#ifdef DEBUG_MODE
        Logger::Throw("Error, world bounds must be set for non-deferred draw command");
#else
        Logger::LogError("Error, world bounds must be set for non-deferred draw command");
#endif
    }

    // Reset sort key
    m_sortKey = 0;

    // TODO: Make configurable settings to generate sort key
    // Viewport
    // 5 bits for 32 possible full-screen layers
    currentBit -= 5; // 58
    m_sortKey.setBits(m_camera->index(), currentBit, 5);

    // Viewport Layer, layer within viewport, e.g. skybox, world layer, effects
    // 7 bits, 128 possible layers
    currentBit -= 7; // 51
    size_t renderLayerOrder = 0;
    if (m_renderLayer.isValid()) {
        renderLayerOrder = m_renderLayer.getPositiveOrder();
    }
    m_sortKey.setBits(renderLayerOrder, currentBit, 7);

    // Translucency, Opaque, normal, additive, or subtractive
    // 3 bits, 8 possible types of translucency (in case of future modes)
    currentBit -= 3; // 48
    m_sortKey.setBits((int)m_renderable->renderSettings().transparencyType(), currentBit, 3);

    // If transparent, depth sort will be last
    size_t shaderBit;
    size_t materialBit;
    if (m_renderable->renderSettings().transparencyType() == TransparencyRenderMode::kOpaque) {
        //currentBit -= 26; // 22
        shaderBit = 15;
        materialBit = 0;
    }
    else {
        // Depth will be least-significant bit
        //currentBit = 0;
        shaderBit = currentBit - 8;
        materialBit = shaderBit - 15;
    }

    // Depth sorting ------------------------------------------------------
    // 26 bits
    // Deferred until pre-sort, since needs normalized depth values from all commands
    m_depth = getDepth();
    s_farthestDepth = std::min(m_depth, s_farthestDepth);
    s_nearestDepth = std::max(m_depth, s_nearestDepth);

    // Shader and material ------------------------------------------------

    // Shader
    // 8 bits, for 256 shaders
    m_sortKey.setBits(m_shaderProgram->getProgramID(), shaderBit, 8);

    // Renderable Sort (e.g., material)
    // 15 bits, for ~37,000 materials
    if (!isDeferredGeometry) {
        // Material sort is unnecessary with shadow commands, which are the only deferred geometry
        m_sortKey.setBits(m_renderable->getSortID(), materialBit, 15);
    }

}

void DrawCommand::preSort()
{
    // Depth sorting ------------------------------------------------------
    // 26 bits
    // Set bit
    // If opaque, sort from front-to-back, otherwise sort from back-to-front
    float depth = (m_depth - DrawCommand::s_nearestDepth) / (DrawCommand::s_farthestDepth - DrawCommand::s_nearestDepth); // normalized depth
    if (m_renderable->renderSettings().transparencyType() != TransparencyRenderMode::kOpaque) {
        // Back-to-front sorting for transparent objects
        depth = 1.0 - depth;
    }

    // Will right-shift depth before inserting into sort key, to snip down to
    // the proper size (currently 26 bits)
    size_t rightShift = 8 * sizeof(float) - 26;
    
    // For opaque objects, depth will be more significant than shader or material
    size_t currentBit = 22;
    if (m_renderable->renderSettings().transparencyType() != TransparencyRenderMode::kOpaque) {
        // Depth will be least-significant bit for transparent objects
        currentBit = 0;
    }
    m_sortKey.setBits(SortKey::FloatToBinary(depth), currentBit, 26, rightShift);
}

bool DrawCommand::hasUniform(const GStringView& name, int* outIndex)
{
    auto iter = std::find_if(m_mainUniforms.begin(), m_mainUniforms.end(),
        [&](const Uniform& u) {
            return u.getName(*m_shaderProgram) == name.c_str();
        });

    if (iter == m_mainUniforms.end()) {
        return false;
    }
    else {
        // Replace uniform if already set
        if (outIndex) {
            *outIndex = iter - m_mainUniforms.begin();
        }
        return true;
    }
}

bool DrawCommand::hasUniform(Uint32_t uniformId, int * outIndex)
{
    auto iter = std::find_if(m_mainUniforms.begin(), m_mainUniforms.end(),
        [&](const Uniform& u) {
        return u.getId() == uniformId;
    });

    if (iter == m_mainUniforms.end()) {
        return false;
    }
    else {
        // Replace uniform if already set
        if (outIndex) {
            *outIndex = iter - m_mainUniforms.begin();
        }
        return true;
    }
}

bool DrawCommand::hasUniform(const Uniform & uniform, int * outIndex)
{
    return hasUniform(uniform.getId(), outIndex);
}

float DrawCommand::getDepth()
{
    // Save on depth calculation if geometry is deferred
    if (m_passFlags.testFlag(RenderablePassFlag::kDeferredGeometry)) {
        return 0;
    }

    // Calculate depth
#ifdef DEBUG_MODE
    int idx;
    if (!hasUniform(Shader::s_worldMatrixUniformName, &idx)) {
        Logger::Throw("Error, command requires a world matrix");
    }
#endif

    Vector3 position = m_mainUniforms[idx].getValue<Matrix4x4>(*m_uniformContainer).getTranslationVector();
    float depth = m_camera->getDepth(position);

    return depth;
}

void DrawCommand::setRenderableWorldBounds(const AABB & bounds)
{
    m_renderableWorldBounds = bounds;
}

void DrawCommand::depthPrePass(OpenGlRenderer & renderer)
{
    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    if (m_renderable->renderSettings().transparencyType() != TransparencyRenderMode::kOpaque) {
        // Don't depth prepass for transparent objects, messes up blend
        return;
    }

    // Bind shader 
    // TODO: Think about a better pipeline for animation uniforms for prepass
    //prepassShader->setUniforms(*m_shaderProgram); // "steal" uniforms from shader program
    ShaderProgram* prepassShader;
    if (m_prepassShaderProgram) {
        // Use specified prepass shader for depth pass
        prepassShader = m_prepassShaderProgram;
    }
    else {
        // Default to same shader program as render
        // Need to render normals for SSAO
        prepassShader = m_shaderProgram;
    }
    updateShaderUniforms(renderer, *prepassShader, true);

    // Perform actual render
    m_renderable->draw(*prepassShader,
        &renderer.m_renderContext,
        &m_renderSettings,
        //(size_t)Renderable::RenderPassFlag::kIgnoreSettings | 
        (size_t)RenderableIgnoreFlag::kIgnoreUniformMismatch);

    // Set render key to this key
    renderer.m_renderState.m_lastRenderedKey = m_sortKey;
}

void DrawCommand::shadowPass(OpenGlRenderer & renderer)
{
    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    // Bind shader
    updateShaderUniforms(renderer, *m_prepassShaderProgram, true);

    // Perform depth render into shadow map
    /// @note Can't ignore textures in case of transparency :/
    /// @todo Flag objects that are transparent so that by default, textures can be ignored
    m_renderable->draw(*m_prepassShaderProgram,
        &renderer.m_renderContext,
        &m_renderSettings,
        //(size_t)RenderableIgnoreFlag::kIgnoreTextures |
        (size_t)RenderableIgnoreFlag::kIgnoreUniformMismatch);
}

void DrawCommand::updateCameraSettings(OpenGlRenderer & renderer)
{
    // TODO: Somehow integrate viewport and GL settings checks as independent commands
    RenderFrameState& rs = renderer.m_renderState;
    if (rs.m_camera) {
        // If a camera is assigned
        if (rs.m_camera->getUuid() != m_camera->getUuid()) {
            switchCameras(renderer);
        }
    }
    else {
        // If no camera is assigned (first camera in frame)
        switchCameras(renderer);
    }
}

void DrawCommand::switchCameras(OpenGlRenderer & renderer)
{
    RenderFrameState& rs = renderer.m_renderState;

    // Release framebuffer of other camera
    bool hadCamera = rs.m_camera != nullptr;
    //bool cameraWasSphereType = false;
    if (hadCamera) {
        rs.m_camera->releaseFrame(FrameBuffer::BindType::kWrite);
        //cameraWasSphereType = rs.m_camera->cameraType() == CameraType::kSphereCamera;
        rs.m_camera = nullptr;
    }

    // Set camera in current render state
    rs.m_camera = m_camera;

    // Bind framebuffer and set viewport
    // Do not clear during shadow pass, everything is cleared at beginning of pass
    // since clearing the point light framebuffer will clear all textures
    bool isShadowPass = rs.m_stage == RenderFrameState::kShadowMapping;
    m_camera->bindFrame(FrameBuffer::BindType::kWrite, !isShadowPass);

    // Bind camera uniforms
    bool isRenderStage = rs.m_stage == RenderFrameState::kRender;
    ShaderProgram* shader;
    if (!isRenderStage && m_prepassShaderProgram) {
        // Use specified prepass shader if depth or lighting pass
        shader = m_prepassShaderProgram;
    }
    else {
        // Default to same shader program as render
        shader = m_shaderProgram;
    }
    m_camera->bindUniforms(&renderer, shader);

    // Bind textures
    m_camera->bindTextures();

    // Bind SSBs for the camera light clustering or shadow mapping
    if (renderer.m_renderState.m_stage != RenderFrameState::kDepthPrepass) {
        m_camera->bindBuffers();
    }
}

//void DrawCommand::switchShaders(OpenGlRenderer & renderer)
//{
//    if (m_shaderProgram->isBound()) {
//        return;
//    }
//
//    RenderFrameState& rs = renderer.m_renderState;
//
//    // Set shader as current
//    bool hadShader = rs.m_shaderProgram != nullptr;
//    rs.m_shaderProgram = m_shaderProgram;
//    
//    // Bind shader program
//    rs.m_shaderProgram->bind();
//
//    // Bind built-in buffers
//}

void DrawCommand::updateShaderUniforms(OpenGlRenderer & renderer, ShaderProgram & shaderProgram, bool ignoreMismatch, int commandIndex)
{
    Q_UNUSED(renderer);

    const std::vector<Uniform>* uniforms;
    if (shaderProgram.getProgramID() == m_shaderProgram->getProgramID()) {
        uniforms = &m_mainUniforms;
    }
    else {
        uniforms = &m_prepassUniforms;
    }

    shaderProgram.bind();
    if (!m_mainUniforms.size()) {
        return;
    }

    for (const Uniform& uniform : *uniforms) {
        if (!shaderProgram.hasUniform(uniform.getId())) {
            //Logger::Throw("Error, shader program does not support uniform");
            // No longer raise error, this is better for prepass
            continue;
        }

        shaderProgram.setUniformValue(uniform);
    }

    // Set command ID as a uniform
    Int32_t colorUniformID = shaderProgram.uniformMappings().m_colorIdUniform;
    if (colorUniformID != -1) {
        if (commandIndex > -1) {
            /// @todo Set this only when renderable is created
            UniformData& commandIndexUniform = renderer.m_commandIndexUniforms[commandIndex];
            Vector4f color = MousePicker::GetChannelId<4, float>(commandIndex);
            commandIndexUniform.setValue(
                color,
                *m_uniformContainer);
            shaderProgram.setUniformValue(colorUniformID, commandIndexUniform);
        }
    }

    shaderProgram.updateUniforms(*m_uniformContainer, ignoreMismatch);
}

float DrawCommand::s_farthestDepth = std::numeric_limits<float>::max();

float DrawCommand::s_nearestDepth = -std::numeric_limits<float>::max();




} // End namespaces

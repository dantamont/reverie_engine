#include "GRenderCommand.h"

#include "../../GCoreEngine.h"
#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../../rendering/renderer/GMainRenderer.h"
#include "../../containers/GFlags.h"
#include "../../containers/GColor.h"
#include "../../GCoreEngine.h"
#include "../../scene/GScenario.h"
#include "../../rendering/materials/GMaterial.h"

#include "../../mixins/GRenderable.h"
#include "../../rendering/shaders/GShaderProgram.h"
#include "../../rendering/buffers/GShaderStorageBuffer.h"
#include "../../components/GCameraComponent.h"
#include "../../containers/GSortingLayer.h"
#include "../../rendering/postprocessing/GPostProcessingChain.h"
#include "../../rendering/lighting/GShadowMap.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// RenderCommand
/////////////////////////////////////////////////////////////////////////////////////////////
RenderCommand::RenderCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//QJsonValue RenderCommand::asJson(const SerializationContext& context) const
//{
//    return QJsonValue();
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void RenderCommand::loadFromJson(const QJsonValue& json, const SerializationContext& context)
//{
//    Q_UNUSED(json)
//}



/////////////////////////////////////////////////////////////////////////////////////////////
// DrawCommand
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::ResetDepths()
{
    s_farthestDepth = std::numeric_limits<float>::max();
    s_nearestDepth = -std::numeric_limits<float>::max();
}
/////////////////////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand()
{
    // Reserve space for 5 uniforms (84 bytes each)
    m_uniforms.reserve(5);
}
/////////////////////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand(Renderable & renderable, ShaderProgram& program, AbstractCamera& camera, int sceneObjectId, ShaderProgram* prepassShaderProgram) :
    m_renderable(&renderable),
    m_shaderProgram(&program),
    m_camera(&camera),
    m_prepassShaderProgram(prepassShaderProgram),
    m_sceneObjectId(sceneObjectId)
{
    // Reserve space for 5 uniforms (84 bytes each)
    m_uniforms.reserve(5);

    // Set uniforms from renderable
    renderable.setUniforms(*this);
}
/////////////////////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand(Renderable & renderable,
    ShaderProgram & program, 
    ShadowMap & shadowMap, 
    int sceneObjectId):
    DrawCommand(renderable, program, *shadowMap.camera(), sceneObjectId)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::perform(MainRenderer& renderer, size_t commandIndex)
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
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::addUniform(const Uniform & uniform)
{
    m_uniforms.push_back(uniform);
}
///////////////////////////////////////////////////////////////////////////////////////////////
const Uniform* DrawCommand::getUniform(const GStringView & uniformName) const
{
    auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [uniformName](const Uniform& uniform) {
        return uniform.getName() == uniformName;
    });

    if (it == m_uniforms.end()) {
        return nullptr;
    }
    else {
        return &(*it);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::addUniforms(const std::vector<Uniform>& uniforms)
{
    m_uniforms.insert(m_uniforms.end(), uniforms.begin(), uniforms.end());
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::onAddToQueue()
{
    size_t maxBit = (sizeof(unsigned long long) * 8) - 1;
    size_t currentBit = maxBit;

    // Render layer is no longer a prerequisite
    //if (!renderLayer()) {
    //    throw("Error, no render layer assigned");
    //}

    // Ensure that world bounds are set, or has deferred geometry
    if (!m_worldBounds.boxData().isInitialized() && !m_passFlags.testFlag(RenderablePassFlag::kDeferredGeometry)) {
#ifdef DEBUG_MODE
        throw("Error, world bounds must be set for non-deferred draw command");
#else
        logError("Error, world bounds must be set for non-deferred draw command");
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
    if (m_renderLayer) {
        renderLayerOrder = m_renderLayer->getPositiveOrder();
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
    m_sortKey.setBits(m_renderable->getSortID(), materialBit, 15);

}
///////////////////////////////////////////////////////////////////////////////////////////////
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

    // Testing to make sure that depth is preserved in sort key
    //unsigned long long mask = 0;
    //unsigned long long one = 1;
    //for (int i = 0; i < 26; i++) {
    //    mask |= one << i;
    //}
    //QString maskStr = QString(SortKey(mask));
    //QString depthStr0 = QString(SortKey(m_sortKey.key()));
    //QString depthStr1 = QString(SortKey(m_sortKey.key() >> currentBit));
    //unsigned long long shiftedDepth = ((m_sortKey.key() >> currentBit) & mask);
    //QString depthStr = QString(SortKey(shiftedDepth));
    //float retrievedDepth = SortKey::BinaryToFloat(shiftedDepth << rightShift);
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool DrawCommand::hasUniform(const GString & uniformName, int * outIndex)
{
    auto iter = std::find_if(m_uniforms.begin(), m_uniforms.end(),
        [&](const Uniform& u) {
        return u.getName() == uniformName;
    });

    if (iter == m_uniforms.end()) {
        return false;
    }
    else {
        // Replace uniform if already set
        *outIndex = iter - m_uniforms.begin();
        return true;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
bool DrawCommand::hasUniform(const Uniform & uniform, int * outIndex)
{
    return hasUniform(QString(uniform.getName()), outIndex);
}
///////////////////////////////////////////////////////////////////////////////////////////////
float DrawCommand::getDepth()
{
    // Calculate depth
    int idx;
    if (!hasUniform(Shader::s_worldMatrixUniformName, &idx)) {
        throw("Error, command requires a world matrix");
    }

    Vector3 position = m_uniforms[idx].get<Matrix4x4g>().getTranslationVector();
    float depth = m_camera->getDepth(position);

    return depth;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::setWorldBounds(const AABB & bounds)
{
    m_worldBounds = bounds;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::depthPrePass(MainRenderer & renderer)
{
    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

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
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::shadowPass(MainRenderer & renderer)
{
    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    // Bind shader
    ShaderProgram* prepassShader;
    if (m_prepassShaderProgram) {
        // Use specified prepass shader for depth pass
        prepassShader = m_prepassShaderProgram;
    }
    else {
        // Default to same shader program as render
        prepassShader = m_shaderProgram;
    }
    updateShaderUniforms(renderer, *prepassShader, true);

    // Perform depth render into shadow map
    m_renderable->draw(*prepassShader,
        &renderer.m_renderContext,
        &m_renderSettings,
        //(size_t)Renderable::RenderPassFlag::kIgnoreSettings |
        (size_t)RenderableIgnoreFlag::kIgnoreUniformMismatch);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::updateCameraSettings(MainRenderer & renderer)
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
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::switchCameras(MainRenderer & renderer)
{
    RenderFrameState& rs = renderer.m_renderState;

    // Release framebuffer of other camera
    bool hadCamera = rs.m_camera != nullptr;
    if (hadCamera) {
        rs.m_camera->releaseFrame(FrameBuffer::BindType::kWrite);
        rs.m_camera = nullptr;
    }

    // Set camera in current render state
    rs.m_camera = m_camera;

    // Bind framebuffer and set viewport
    // Do not clear framebuffer if swapping between sphere cameras during light pass, 
    // or else shadow map data will be lost from framebuffer
    bool isRenderingPointMaps = 
        (m_camera->cameraType() == CameraType::kSphereCamera) &&
        (rs.m_stage == RenderFrameState::kShadowMapping) && hadCamera;
    m_camera->bindFrame(FrameBuffer::BindType::kWrite, !isRenderingPointMaps);

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
/////////////////////////////////////////////////////////////////////////////////////////////////
//void DrawCommand::switchShaders(MainRenderer & renderer)
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
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::updateShaderUniforms(MainRenderer & renderer, ShaderProgram & shaderProgram, bool ignoreMismatch, int commandIndex)
{
    Q_UNUSED(renderer);

    // TODO: Somehow integrate shader program checks as independent commands
    shaderProgram.bind();
    if (!m_uniforms.size()) {
        return;
    }

    ShaderProgram::UniformInfoIter iter;
    for (const Uniform& uniform : m_uniforms) {
        if (!shaderProgram.hasUniform(uniform.getName(), iter)) {
            //throw("Error, shader program does not support uniform");
            // No longer raise error, this is better for prepass
            continue;
        }

        shaderProgram.setUniformValue(uniform);

        // FIXME: Doesn't work
        // TODO: Make this more efficient, don't want to have to update every draw call
        // TODO: Replace map of uniforms with vector
        // If renderable has the uniform, then copy value to avoid overriding DrawCommand value
        //if (Map::HasKey(m_renderable->uniforms(), uniform.getName())) {
        //    m_renderable->uniforms()[uniform.getName()] = uniform;
        //}
    }

    // Set command ID as a uniform
    static const char* const colorUniformName = "g_colorId";
    if (shaderProgram.hasUniform(colorUniformName)) {
        if (commandIndex > -1) {
            shaderProgram.setUniformValue(colorUniformName, MousePicker::GetChannelId<4, float>(commandIndex));
        }
        //else {
        //    static Vector4 white(255.0f, 255.0f, 255.0f, 255.0f);
        //    shaderProgram.setUniformValue(colorUniformName, white);
        //    //const GString& uniformName = shaderProgram.handle()->getName();
        //    //uniformName;
        //}
    }

    shaderProgram.updateUniforms(ignoreMismatch);
}
///////////////////////////////////////////////////////////////////////////////////////////////
float DrawCommand::s_farthestDepth = std::numeric_limits<float>::max();
///////////////////////////////////////////////////////////////////////////////////////////////
float DrawCommand::s_nearestDepth = -std::numeric_limits<float>::max();



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#include "GbRenderCommand.h"

#include "../../GbCoreEngine.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../../rendering/renderer/GbMainRenderer.h"
#include "../../containers/GbFlags.h"
#include "../../containers/GbColor.h"
#include "../../GbCoreEngine.h"
#include "../../scene/GbScenario.h"
#include "../../rendering/materials/GbMaterial.h"

#include "../../rendering/shaders/GbShaders.h"
#include "../../components/GbCamera.h"
#include "../../containers/GbSortingLayer.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// RenderCommand
/////////////////////////////////////////////////////////////////////////////////////////////
RenderCommand::RenderCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//QJsonValue RenderCommand::asJson() const
//{
//    return QJsonValue();
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void RenderCommand::loadFromJson(const QJsonValue & json)
//{
//    Q_UNUSED(json)
//}



/////////////////////////////////////////////////////////////////////////////////////////////
// DrawCommand
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::resetDepths()
{
    s_farthestDepth = std::numeric_limits<float>::max();
    s_nearestDepth = -std::numeric_limits<float>::max();
}
/////////////////////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
DrawCommand::DrawCommand(Renderable & renderable, ShaderProgram& program, Camera& camera) :
    m_renderable(&renderable),
    m_shaderProgram(&program),
    m_camera(&camera)
{
    //m_frameBuffer = &m_camera->frameBuffer();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::perform(MainRenderer& renderer)
{
    if (renderer.m_renderState.m_stage == RenderFrameState::kDepthPrepass) {
        // Performing depth pre-pass
        depthPrePass(renderer);
    }
    else {
        // Performing actual render
        render(renderer);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::setUniform(const Uniform & uniform)
{
    m_uniforms[uniform.getName()] = uniform;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::onAddToQueue()
{
    size_t maxBit = (sizeof(unsigned long long) * 8) - 1;
    size_t currentBit = maxBit;

    // For now, make a render layer a pre-requisite
    if (!renderLayer()) {
        throw("Error, no render layer assigned");
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
    m_sortKey.setBits(m_renderLayer->getPositiveOrder(), currentBit, 7);

    // Translucency, Opaque, normal, additive, or subtractive
    // 3 bits, 8 possible types of translucency (in case of future modes)
    currentBit -= 3; // 48
    m_sortKey.setBits(m_renderable->transparencyType(), currentBit, 3);

    // If transparent, depth sort will be last
    size_t shaderBit;
    size_t materialBit;
    if (m_renderable->transparencyType() == Renderable::kOpaque) {
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
    if (m_renderable->transparencyType() != Renderable::kOpaque) {
        // Back-to-front sorting for transparent objects
        depth = 1.0 - depth;
    }

    size_t rightShift = 8 * sizeof(float) - 26;
    //m_sortKey.setBits(SortKey::FloatToSortedBinary(depth), currentBit, 26, rightShift);
    
    // For opaque objects, depth will be more significant than shader or material
    size_t currentBit = 22;
    if (m_renderable->transparencyType() != Renderable::TransparencyType::kOpaque) {
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
float DrawCommand::getDepth()
{
#ifdef DEBUG_MODE
    // Calculate depth
    if (!Map::HasKey(m_uniforms, QStringLiteral("worldMatrix"))) {
        throw("Error, command requires a world matrix");
    }
#endif

    //bool isMatrix = m_uniforms["worldMatrix"].is<Matrix4x4g>();
    Vector3g position = m_uniforms["worldMatrix"].get<Matrix4x4g>().getTranslationVector();
    float depth = m_camera->getDepth(position);

    return depth;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::render(MainRenderer & renderer)
{

    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    m_camera->frameBuffer().bind();

    // Bind shader 
    updateShaderUniforms(renderer);

    // Bind render settings
    for (RenderSettings* settings : m_renderSettings) {
        settings->bind();
    }

    // Perform actual render
    m_renderable->draw(*m_shaderProgram, nullptr, 0);

    // Release render settings
    for (RenderSettings* settings : m_renderSettings) {
        settings->release();
    }

    m_camera->frameBuffer().release();

    // Set render key to this key
    renderer.m_renderState.m_lastRenderedKey = m_sortKey;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::depthPrePass(MainRenderer & renderer)
{
    //const std::shared_ptr<ShaderProgram>& prepassShader = renderer.prepassShader();

    if (m_renderable->transparencyType() != Renderable::kOpaque) {
        // Do not write to depth buffer for transparent objects
        // TODO: Enable depth write for transparent objects in render settings
        // e.g., glDepthMask(true), glDepthFunc(GL_LEQUAL)
        return;
    }

    // Set viewport and camera uniforms
    updateCameraSettings(renderer);

    m_camera->frameBuffer().bind();

    // Bind shader 
    // FIXME: Add discards for transparent objects in prepass shader
    // Also somehow update the animation-related uniforms
    //prepassShader->bind();
    //prepassShader->setUniformValue(m_uniforms["worldMatrix"], true);
    updateShaderUniforms(renderer);

    // Perform actual render
    //size_t flags = (size_t)Renderable::RenderPassFlag::kIgnoreSettings |
    //    (size_t)Renderable::RenderPassFlag::kIgnoreTextures |
    //    (size_t)Renderable::RenderPassFlag::kIgnoreUniforms;
    m_renderable->draw(*m_shaderProgram, 
        nullptr,
        (size_t)Renderable::RenderPassFlag::kIgnoreSettings);

    m_camera->frameBuffer().release();

    // Set render key to this key
    renderer.m_renderState.m_lastRenderedKey = m_sortKey;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::updateCameraSettings(MainRenderer & renderer)
{
    // blue violet
    static Color clearColor = Color(Vector4g( 0.55f, 0.6f, 0.93f, 1.0f ));

    // TODO: Somehow integrate viewport and GL settings checks as independent commands
    RenderFrameState& rs = renderer.m_renderState;
    if (rs.m_camera) {
        // If a camera is assigned
        if (rs.m_camera->getUuid() != m_camera->getUuid()) {
            // If a different camera has been assigned

            // Set camera in current render state
            rs.m_camera = m_camera;

            // Set the viewport size for camera rendering
            m_camera->setGLViewport(renderer);

            // Resize framebuffer dimensions if screen resized
            if (rs.resized() || rs.playModeChanged()) {
                m_camera->resizeFrameBuffer(renderer);
            }

            // Clear depth buffer for camera renders
            m_camera->frameBuffer().bind();
            m_camera->frameBuffer().clear(clearColor);
            m_camera->frameBuffer().release();
        }
    }
    else {
        // If no camera is assigned (first loop)

        // Set camera in current render state
        rs.m_camera = m_camera;

        // Set the viewport size for camera rendering
        m_camera->setGLViewport(renderer);

        // Resize framebuffer dimensions if screen resized
        if (rs.resized() || rs.playModeChanged()) {
            m_camera->resizeFrameBuffer(renderer);
        }

        // Clear depth buffer for camera renders
        m_camera->frameBuffer().bind();
        m_camera->frameBuffer().clear(clearColor);
        m_camera->frameBuffer().release();
    }

    // Bind camera uniforms
    // TODO: Check if camera has moved before binding uniforms
    m_camera->bindUniforms();
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::updateShaderUniforms(MainRenderer & renderer)
{
    updateShaderUniforms(renderer, *m_shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////
void DrawCommand::updateShaderUniforms(MainRenderer & renderer, ShaderProgram & shaderProgram)
{
    // TODO: Somehow integrate shader program checks as independent commands
    shaderProgram.bind();
    if (m_uniforms.size()) {
        for (const auto& uniformPair : m_uniforms) {
#ifdef DEBUG_MODE
            if (!shaderProgram.hasUniform(uniformPair.first)) {
                throw("Error, shader program does not support uniform");
            }
#endif
            shaderProgram.setUniformValue(uniformPair.second);

            // If renderable has the uniform, then remove from renderable
            // to avoid overriding DrawCommand value
            //if (Map::HasKey(m_renderable->uniforms(), uniformPair.first)) {
            //    m_renderable->uniforms().erase(uniformPair.first);
            //}
        }
        shaderProgram.updateUniforms();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////
//QJsonValue DrawCommand::asJson() const
//{
//    return QJsonValue();
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void DrawCommand::loadFromJson(const QJsonValue & json)
//{
//}
///////////////////////////////////////////////////////////////////////////////////////////////
float DrawCommand::s_farthestDepth = std::numeric_limits<float>::max();
///////////////////////////////////////////////////////////////////////////////////////////////
float DrawCommand::s_nearestDepth = -std::numeric_limits<float>::max();



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

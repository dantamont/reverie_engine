#include "core/rendering/view/GPointLightCamera.h"

#include "core/GCoreEngine.h"
#include "fortress/containers/GColor.h"
#include "core/components/GTransformComponent.h"

#include "core/rendering/lighting/GLightSettings.h"

#include "core/resource/GResourceCache.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

#include "core/loop/GSimLoop.h"
#include "core/debugging/GDebugManager.h"

#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "fortress/layer/framework/GFlags.h"


namespace rev {

PointLightCamera::PointLightCamera(TransformInterface* transform, RenderContext * context, uint32_t pointLightIndex):
    m_frameBuffer(&context->lightingSettings().pointLightFrameBuffer()),
    m_pointLightIndex(pointLightIndex),
    m_transform(transform),
    m_nearClip(0.1f),
    m_context(context)
{  
    UniformContainer& uc = context->uniformContainer();
    m_uniforms.m_pointLightIndex.setValue((Int32_t)pointLightIndex, uc);
}

PointLightCamera::~PointLightCamera()
{
}

void PointLightCamera::resizeFrame(uint32_t width, uint32_t height)
{
    m_frameBuffer->reinitialize(width, height);
}

void PointLightCamera::bindUniforms(const OpenGlRenderer*, ShaderProgram* shaderProgram)
{
    shaderProgram->setUniformValue(
        shaderProgram->uniformMappings().m_pointLightIndex,
        m_uniforms.m_pointLightIndex);
}

void PointLightCamera::bindBuffers()
{
    m_context->lightingSettings().pointLightMatrixBuffers().readBuffer().bindToPoint();
    m_context->lightingSettings().pointLightAttributeBuffers().readBuffer().bindToPoint();
}

void PointLightCamera::bindFrame(FrameBuffer::BindType readOrWrite, bool clear)
{
    Q_UNUSED(readOrWrite);

    // Use framebuffer dimensions to set viewport
    glViewport(0, 0, m_frameBuffer->width(), m_frameBuffer->height());

    // Bind framebuffer
    m_frameBuffer->bind();
    if (clear) {
        // Clear 
        static Color clearColor = Color(Vector4(0.85f, 0.6f, 0.43f, 0.0f));
        m_frameBuffer->clear();
    }
}

void PointLightCamera::releaseFrame(FrameBuffer::BindType readOrWrite)
{
    Q_UNUSED(readOrWrite);
    m_frameBuffer->release();
}

void PointLightCamera::updateFrustum(CameraUpdateFlags flags)
{
    G_UNUSED(flags);

    // Update light view-projection matrices
    ShaderStorageBufferQueue& lightMatrixBuffers = m_context->lightingSettings().pointLightMatrixBuffers();
    m_lightMatrices.updateViewProjectionMatrices();
    lightMatrixBuffers.queueUpdate<std::array<Matrix4x4g, 6>>(m_lightMatrices.m_viewProjectionMatrices, m_pointLightIndex /*buffer offset*/);

    // Update position and far clip of frustum
    m_frustum.setOrigin(m_transform->getPosition().asFloat());

    // Update light attributes
    ShaderStorageBufferQueue& lightAttributeBuffers = m_context->lightingSettings().pointLightAttributeBuffers();
    lightAttributeBuffers.queueUpdate<BoundingSphereData>(m_frustum.data(), m_pointLightIndex /*buffer offset*/);
}

float PointLightCamera::getDepth(const Vector3 & position)
{
    return -1 * (position - m_frustum.origin()).length();
}

const std::array<Vector3, 6> PointLightCamera::ViewProjectionMatrices::s_lookAtDirs{ {
    {1.0, 0.0, 0.0}, // right
    {-1.0, 0.0, 0.0}, // left
    {0.0, 1.0, 0.0}, // top
    {0.0, -1.0, 0.0}, // bottom
    {0.0, 0.0, 1.0}, // near
    {0.0, 0.0, -1.0} // far
} };

const std::array<Vector3, 6> PointLightCamera::ViewProjectionMatrices::s_upDirs{ {
    {0.0, -1.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0},
    {0.0, -1.0, 0.0},
    {0.0, -1.0, 0.0},
} };


void PointLightCamera::ViewProjectionMatrices::updateViewMatrices(const Vector3& cameraPosition)
{
    for (size_t i = 0; i < m_viewMatrices.size(); i++) {
        m_viewMatrices[i] = Camera::LookAtRH(cameraPosition, cameraPosition + s_lookAtDirs[i], s_upDirs[i]);
    }
}

void PointLightCamera::ViewProjectionMatrices::updateViewProjectionMatrices()
{
    for (Uint32_t i = 0; i < m_viewProjectionMatrices.size(); i++) {
        m_viewProjectionMatrices[i] = m_projectionMatrices[i] * m_viewMatrices[i];
    }
}

// End namespaces
}

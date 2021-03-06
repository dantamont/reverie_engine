#include "GPointLightCamera.h"

#include "../../GCoreEngine.h"
#include "../../containers/GColor.h"
#include "../../components/GTransformComponent.h"

#include "../../rendering/lighting/GLightSettings.h"

#include "../../resource/GResourceCache.h"
#include "../models/GModel.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"

#include "../../loop/GSimLoop.h"
#include "../../debugging/GDebugManager.h"

#include "../renderer/GMainRenderer.h"
#include "../renderer/GRenderCommand.h"
#include "../renderer/GRenderContext.h"

#include "../../../view/GWidgetManager.h"
#include "../../../view/GL/GGLWidget.h"
#include "../../containers/GFlags.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

PointLightCamera::PointLightCamera(Transform* transform, RenderContext * context, size_t cubemapIndex):
    m_frameBuffer(&context->lightingSettings().pointLightFrameBuffer()),
    m_cubeTextureLayer(cubemapIndex),
    m_transform(transform),
    m_nearClip(0.1f),
    m_context(context)
{
    // Initialize sub-cameras
    for (size_t i = 0; i < 6; i++) {
        // TODO: Maybe make a sub-camera class, without any framebuffer
        // Create sub-camera (framebuffers are not used)
        m_cameras[i] = Camera(transform, context->context(), AliasingType::kDefault, FrameBuffer::BufferAttachmentType::kTexture, 0, 0);
        m_cameras[i].renderProjection().setNearClipPlane(m_nearClip);
    }    
}

PointLightCamera::~PointLightCamera()
{
}

void PointLightCamera::resizeFrame(size_t width, size_t height)
{
    m_frameBuffer->reinitialize(width, height);
}

void PointLightCamera::bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram)
{
    Q_UNUSED(renderer)
    //const std::vector<Matrix4x4g>& cameraMatrices = m_cameraMatrices.get<std::vector<Matrix4x4g>>();
    //shaderProgram->setUniformValue("nearPlane", m_nearClip);

    //shaderProgram->setUniformValue("farPlane", m_frustum.radius());
    //shaderProgram->setUniformValue("sphereCameraPos", m_frustum.origin());

    //shaderProgram->setUniformValue(m_cameraMatrices);
    //shaderProgram->setUniformValue("cubemapLayer", (int)m_cubeTextureLayer);
    shaderProgram->setUniformValue("numCubemapLayers", (int)NUM_SHADOWS_PER_LIGHT_TYPE);
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
        static Color clearColor = Color(Vector4(0.85f, 0.6f, 0.43f, 1.0f));
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
    // Update light view-projection matrices
    ShaderStorageBufferQueue& lightMatrixBuffers = m_context->lightingSettings().pointLightMatrixBuffers();
    for (size_t i = 0; i < 6; i++) {
        Camera& subCamera = m_cameras[i];
        const Matrix4x4g& viewMatrix = subCamera.getViewMatrix();
        const Matrix4x4g& projMatrix = subCamera.renderProjection().projectionMatrix();
        m_lightMatrices[i] = projMatrix * viewMatrix;
    }
    lightMatrixBuffers.queueUpdate<std::array<Matrix4x4g, 6>>(m_lightMatrices, m_cubeTextureLayer /*buffer offset*/);

    // Update sub-camera frusutums
    for (size_t i = 0; i < 6; i++) {
        // Update frustum size
        Camera& subCamera = m_cameras[i];
        subCamera.updateFrustum(flags);
    }

    // Update position and far clip of frustum
    if (flags.testFlag(CameraUpdateFlag::kProjectionUpdated)) {
        RenderProjection& projection = m_cameras[0].renderProjection();
        m_nearClip = projection.nearClipPlane();
        m_frustum.setRadius(projection.farClipPlane());
    }
    m_frustum.setOrigin(m_transform->getPosition().asFloat());

    // Update light attributes
    ShaderStorageBufferQueue& lightAttributeBuffers = m_context->lightingSettings().pointLightAttributeBuffers();
    lightAttributeBuffers.queueUpdate<BoundingSphereData>(m_frustum.data(), m_cubeTextureLayer /*buffer offset*/);
}

float PointLightCamera::getDepth(const Vector3 & position)
{
    return -1 * (position - m_frustum.origin()).length();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

#include "GbPointLightCamera.h"

#include "../../GbCoreEngine.h"
#include "../../containers/GbColor.h"
#include "../../components/GbTransformComponent.h"

#include "../../rendering/lighting/GbLightSettings.h"

#include "../../resource/GbResourceCache.h"
#include "../models/GbModel.h"
#include "../shaders/GbShaders.h"
#include "../buffers/GbUniformBufferObject.h"

#include "../../loop/GbSimLoop.h"
#include "../../debugging/GbDebugManager.h"

#include "../renderer/GbMainRenderer.h"
#include "../renderer/GbRenderCommand.h"
#include "../renderer/GbRenderContext.h"

#include "../../../view/GbWidgetManager.h"
#include "../../../view/GL/GbGLWidget.h"
#include "../../containers/GbFlags.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// Sphere Camera
//////////////////////////////////////////////////////////////////////////////////////////////////

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

    // Clear out buffer values (not really necessary)
    //ShaderStorageBuffer& lightMatrices = m_context->lightingSettings().pointLightMatrixBuffer();
    //Matrix4x4g* matrices = lightMatrices.data<Matrix4x4g>();
    //for (size_t i = 0; i < 6; i++) {
    //    matrices[m_cubeTextureLayer*6 + i] = Matrix4x4g();
    //}
    //lightMatrices.unmap(true);

    // Initialize uniform member
    //std::vector<Matrix4x4g> matrices;
    //matrices.resize(6);
    //m_cameraMatrices = Uniform("cameraMatrices", matrices);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
PointLightCamera::~PointLightCamera()
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::resizeFrame(size_t width, size_t height)
{
    m_frameBuffer->reinitialize(width, height);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::bindUniforms(const MainRenderer* renderer, ShaderProgram* shaderProgram)
{
    Q_UNUSED(renderer)
    //const std::vector<Matrix4x4g>& cameraMatrices = m_cameraMatrices.get<std::vector<Matrix4x4g>>();
    //shaderProgram->setUniformValue("nearPlane", m_nearClip);
    shaderProgram->setUniformValue("farPlane", m_frustum.radius());
    shaderProgram->setUniformValue("sphereCameraPos", m_frustum.origin());
    //shaderProgram->setUniformValue(m_cameraMatrices);
    shaderProgram->setUniformValue("cubemapLayer", (int)m_cubeTextureLayer);
    //shaderProgram->setUniformValue("numCubemapLayers", (int)NUM_SHADOWS_PER_LIGHT_TYPE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::bindBuffers()
{
    m_context->lightingSettings().pointLightMatrixBuffers().readBuffer().bindToPoint();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::bindFrame(bool clear)
{
    // Use framebuffer dimensions to set viewport
    glViewport(0, 0, m_frameBuffer->width(), m_frameBuffer->height());

    // Bind framebuffer
    m_frameBuffer->bind();
    if (clear) {
        static Color clearColor = Color(Vector4(0.85f, 0.6f, 0.43f, 1.0f));
        m_frameBuffer->clear(clearColor);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::releaseFrame()
{
    m_frameBuffer->release();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void PointLightCamera::updateFrustum(CameraUpdateFlags flags)
{
    //std::vector<Matrix4x4g>& cameraMatrices = m_cameraMatrices.get<std::vector<Matrix4x4g>>();
    ShaderStorageBufferQueue& lightMatrixBuffers = m_context->lightingSettings().pointLightMatrixBuffers();
    //Matrix4x4g* lightMatrices = lightMatrixBuffer.data<Matrix4x4g>(m_cubeTextureLayer * 6, 6, GL::RangeBufferAccessFlag::kWrite | GL::RangeBufferAccessFlag::kInvalidateRange);
    for (size_t i = 0; i < 6; i++) {
        Camera& subCamera = m_cameras[i];
        const Matrix4x4g& viewMatrix = subCamera.getViewMatrix();
        const Matrix4x4g& projMatrix = subCamera.renderProjection().projectionMatrix();
        m_lightMatrices[i] = projMatrix * viewMatrix;
        //lightMatrixBuffer.subData(projMatrix * viewMatrix, m_cubeTextureLayer * 6 + i);
    }
    lightMatrixBuffers.queueUpdate<std::array<Matrix4x4g, 6>>(m_lightMatrices, m_cubeTextureLayer * 6);
    //lightMatrixBuffer.unmap(true);

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
}
//////////////////////////////////////////////////////////////////////////////////////////////////
float PointLightCamera::getDepth(const Vector3 & position)
{
    return -1 * (position - m_frustum.origin()).length();
}

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

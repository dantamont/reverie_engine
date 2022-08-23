#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/lighting/GLight.h"

#include "core/rendering/materials/GCubeTexture.h"

#include "core/rendering/view/GPointLightCamera.h"
#include "core/GCoreEngine.h"
#include "core/debugging/GDebugManager.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/scene/GScene.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScenario.h"
#include "core/components/GCameraComponent.h"
#include "core/components/GLightComponent.h"
#include "core/components/GTransformComponent.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

namespace rev {


ShadowMap::ShadowMap(RenderContext& context, LightComponent* lightComponent, size_t mapIndex) :
    m_lightComponent(lightComponent)
{
    if (!context.isCurrent()) {
        context.makeCurrent();
    }

    initializeCamera(context, mapIndex);
}


ShadowMap::~ShadowMap()
{
}

void ShadowMap::resizeShadowMap(size_t width, size_t height)
{
    m_camera->resizeFrame(width, height);
    m_camera->updateBufferUniforms(m_lightComponent->scene()->engine()->openGlRenderer()->renderContext());
}

void ShadowMap::updateShadowAttributes(const Scene & sc)
{
    // NOTE: All near clip stuff is deprecated, but left in in case there's a need for that control

    setView(sc);
    setProjection(sc);
    CameraUpdateFlags updateFlags;
    updateFlags.setFlag(CameraUpdateFlag::kViewUpdated, true);
    updateFlags.setFlag(CameraUpdateFlag::kProjectionUpdated, true);
    m_camera->updateFrustum(updateFlags); // Need to update frustum for objects in view

    switch (m_lightComponent->getLightType()) {
    case Light::kPoint:
    {
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera.get());

        // Set light-space matrix in SSBO
        const ShadowInfo& shadow = m_lightComponent->cachedShadow();
        const BoundingSphere& sphere = static_cast<const BoundingSphere&>(cam.frustum());
        //float camNearClip = cam.nearClip(); 
        //shadow.setNearClipPlane(camNearClip);
        m_lightComponent->setShadowFarClipPlane(sphere.radius());
        break;
    }
    case Light::kDirectional:
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera.get());

        // Set light-space matrix in SSBO
        const ShadowInfo& shadow = m_lightComponent->cachedShadow();
        const Matrix4x4g& proj = cam.renderProjection().projectionMatrix();
        const Matrix4x4g& view = cam.getViewMatrix();
        m_lightComponent->setShadowLightMatrix(proj * view);
        break;
    }
    default:
        Logger::Throw("Error, light type unrecognized");
    }

    m_camera->updateBufferUniforms(sc.engine()->openGlRenderer()->renderContext());
}

void ShadowMap::setView(const Scene& scene)
{
    // Create view matrix from light's point of view, to origin of scene
    switch (m_lightComponent->getLightType()) {
    case Light::kDirectional: {
        Camera& cam = *static_cast<Camera*>(m_camera.get());

        // This approach obtains a view matrix such that the light always emanates
        // from the edge of the frustum bounds

        // Create frustum of correct size/orientation, but not position
        const Vector3& lightDir = m_lightComponent->cachedLight().direction();

        // Get world-space frustum bounds that contain every camera's view frustum
        // TODO: Store frustum bounds in camera
        const AABBData& sceneFrustumBounds = scene.getVisibleFrustumBounds().boxData();

        // Use size of visible scene frustum to get correct lightPos
        Vector3 frustumCentroid = sceneFrustumBounds.getOrigin();
        float halfBoxLength = sceneFrustumBounds.getDimensions().z() / 2.0;
        Vector3 tmpLightPos = frustumCentroid - lightDir * halfBoxLength; // Temporary working position of camera

        // Obtain final view matrix with correctly scaled position
        // NOTE: This will update the translation of the scene object accordingly
        //m_camera.setLookAt(tmpLightPos, frustumCentroid, Vector3f(0.0, 1.0, 0.0));
        cam.setViewMatrix(SceneCamera::LookAtRH(tmpLightPos, frustumCentroid, Vector3f(0.0, 1.0, 0.0)));
        break;
    }
    case Light::kPoint:
    {
        // Generate six view matrices, one for each sub-camera
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera.get());
        const Vector3& lightPos = m_lightComponent->sceneObject()->transform().getPosition().asFloat();
        cam.viewProjectionMatrices().updateViewMatrices(lightPos);

        break;
    }
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera.get());

        const Vector3& lightPos = m_lightComponent->sceneObject()->transform().getPosition().asFloat();
        const Vector3& lightDir = m_lightComponent->cachedLight().direction();
        
        // TODO: Use this by default for LookAtRH, doesn't seem super important right now
        // Get "up" vector. See: 
        // https://www.gamedev.net/forums/topic/388559-getting-a-up-vector-from-only-having-a-forward-vector/
        Vector3 forward = lightPos - (lightPos + lightDir);
        forward.normalize();
        Vector3 up = Vector3f::Up(); // temp up
        Vector3 right = forward.cross(up);
        if (right.lengthSquared() > 1e-6) {
            // If forward is not in up direction, find camera up that is "up-ish"
            up = right.cross(forward);
        }
        else {
            up = Vector3f(1.0, 0.0, 0.0);
        }
        
        // This will update the translation of the scene object accordingly
        //m_camera.setLookAt(lightPos, lightPos + lightDir, up);
        cam.setViewMatrix(SceneCamera::LookAtRH(lightPos, lightPos + lightDir, up));
        break;
    }
    default:
        Logger::Throw("Error, type of light unrecognized");
        break;
    }
}


void ShadowMap::setProjection(const Scene& scene)
{

    // Create perspective matrix
    switch (m_lightComponent->getLightType()) {
    case Light::kDirectional: {
        // Orthographic, since light does not originate from a point
        // Trying to fit the entire world in the orthographic box will cause low
        // shadow resolution, so build size around currently viewable area (view frustum)
        /// \see https://www.gamedev.net/forums/topic/505893-orthographic-projection-for-shadow-mapping/

        // FIXME: Note, this approach will not take into account any shadow casters
        // That lay outside of the view frustum, that might cast shadows into the frustum,
        // May need to move the near clip plane further back (towards the light) to render
        // all shadow casters
        Camera& cam = *static_cast<Camera*>(m_camera.get());

        // Get world-space frustum bounds
        const AABBData& sceneFrustumBounds = scene.getVisibleFrustumBounds().boxData();
        std::vector<Vector3> frustumPoints;
        sceneFrustumBounds.getPoints(frustumPoints);

        // TODO: Check for objects near far clip bound and fix, e.g.:
        // for closeObject
        // if(point.z + closeObject.radius > maxZ) maxZ = point.z + closeObject.radius;

        // Convert points to the light's view space
        std::array<Vector4, 8> viewPoints;
        for (size_t i = 0; i < 8; i++) {
            viewPoints[i] = cam.getViewMatrix() * Vector4(frustumPoints[i], 1.0);
        }

        // Create bounding box of camera's frustum in the light's view space
        // This is used to size the bounds of the orthographic projection matrix
        AABBData vb;
        vb.resize(viewPoints);
        cam.renderProjection().setOrthographic(
            vb.minX(),   // left
            vb.maxX(),   // right
            vb.minY(),   // bottom
            vb.maxY(),   // top
            -vb.maxZ(),  // near, inversion because light looks at negative z axis
            //0.01,
            -vb.minZ()); // far, inversion because light looks at negative z axis
        
        break;
    }
    case Light::kPoint: {
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera.get());

        // Perspective projection, since light fans out from a point
        float fovDeg = 90.0f; // make sure the viewing field is exactly large enough to fill a single face of the cubemap such that all faces align correctly to each other at the edges
        float aspectRatio = 1.0f; // Aspect area will just be 1, since we just want a cone's frustum
        float nearClip = 1.0f;
        //float nearClip = lightComponent()->cachedShadow().nearClipPlane(); 
        float farClip = m_lightComponent->cachedLight().getRange();  // Far clip: Range of spot light
        
        cam.setFarClip(farClip);
        std::array<Matrix4x4g, 6>& projectionMatrices = cam.viewProjectionMatrices().m_projectionMatrices;
        for (size_t i = 0; i < 6; i++) {
            RenderProjection::Perspective(projectionMatrices[i], fovDeg, aspectRatio, nearClip, farClip);
        }

        break;
    }
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera.get());

        // Perspective projection, since light fans out from a point
        float fovDeg = m_lightComponent->cachedLight().getFOVRad() * Constants::RadToDeg; // Obtain from spot-light's outer cutoff value
        float aspectRatio = 1.0f; // Aspect area will just be 1, since we just want a cone's frustum
        float nearClip = 0.01f; // Near clip, something close to zero
        float farClip = m_lightComponent->cachedLight().getRange();  // Far clip: Range of spot light
        cam.renderProjection().setPerspective(fovDeg, aspectRatio, nearClip, farClip);
        
        //SceneCamera& debugCam = scene.scenario()->engine()->debugManager()->camera()->camera();
        //m_camera.renderProjection().projectionMatrix() = debugCam.renderProjection().projectionMatrix();
        break;
    }
    default:
        Logger::Throw("Error, type of light unrecognized");
        break;
    }
}

void ShadowMap::initializeCamera(RenderContext& context, size_t mapIndex)
{
    // Retrieve textures containing all shadow maps supported by the engine    Light::LightType lightType = m_lightComponent->getLightType();
    Light::LightType lightType = m_lightComponent->getLightType();
    const auto& textures = context.lightingSettings().shadowTextures();
    const std::shared_ptr<Texture>& tex = textures[(int)lightType];
    
    TransformInterface* lightObjectTransform = &m_lightComponent->sceneObject()->transform();
    
    if (lightType != Light::kPoint) {
        auto camera = std::make_unique<Camera>(
            lightObjectTransform,
            &context,
            AliasingType::kDefault,
            FrameBuffer::BufferAttachmentType::kTexture,
            4,  // Not using MSAA, so doesn't matter
            0); // Don't need any color attachments

        // Set camera properties
        camera->renderProjection().setAspectRatio(1.0, false);

        // Initialize framebuffer, attaching shadow map textures
        /// @note Only use write framebuffer for shadow-map
        /// @todo Create a generic "LightCamera" with only one framebuffer so this is less ambiguous,
        // and doesn't take up the extra memory from the additional framebuffer
        camera->frameBuffers().writeBuffer().reinitialize(tex->width(), tex->height(), tex, mapIndex);
    
        m_camera = std::move(camera);
    }
    else {
        auto camera = std::make_unique<PointLightCamera>(lightObjectTransform, &context, mapIndex);

        // Initialize framebuffer, attaching entire shadow map texture
        // Was using a cubemap array, but now each point light renders to its own cubemap
        camera->frameBuffer().reinitialize(tex->width(),
            tex->height(),
            tex,
            mapIndex); // Unused

        m_camera = std::move(camera);
    }

    m_camera->updateBufferUniforms(context);
}


// End namespaces
}
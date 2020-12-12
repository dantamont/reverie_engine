#include "GbShadowMap.h"
#include "GbLight.h"

#include "../materials/GbCubeTexture.h"

#include "../view/GbPointLightCamera.h"
#include "../../GbCoreEngine.h"
#include "../../debugging/GbDebugManager.h"
#include "../../resource/GbResourceCache.h"
#include "../renderer/GbMainRenderer.h"
#include "../../view/GL/GbGLWidget.h"
#include "../../scene/GbScene.h"
#include "../../scene/GbSceneObject.h"
#include "../../scene/GbScenario.h"
#include "../../components/GbCameraComponent.h"
#include "../../components/GbLightComponent.h"
#include "../../components/GbTransformComponent.h"
#include "../../resource/GbResourceCache.h"
#include "../shaders/GbShaders.h"
#include "../buffers/GbUniformBufferObject.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
ShadowMap::ShadowMap(RenderContext& context, LightComponent* lightComponent, size_t mapIndex) :
    m_lightComponent(lightComponent)
    //m_mapIndex(0)
{
    if (!context.isCurrent()) {
        throw("Context is not current");
    }

    initializeCamera(context, mapIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////
ShadowMap::~ShadowMap()
{
    delete m_camera;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::createDrawCommands(Scene & scene, MainRenderer & renderer)
{
    // Only need one set of draw commands for point lights
    if (lightComponent()->getLightType() == Light::kPoint) {
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera);
        if (cam.cubemapTextureLayer() > 0) {
            return;
        }
    }

    // Iterate through scene objects to generate draw commands
    auto& topLevelObjects = scene.topLevelSceneObjects();
    for (const std::shared_ptr<SceneObject>& sceneObject : topLevelObjects) {
        sceneObject->createDrawCommands(*this, renderer);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::resize(size_t width, size_t height)
{
    m_camera->resizeFrame(width, height);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::updateShadowAttributes(const Scene & sc)
{
    // NOTE: All near clip stuff is deprecated, but left in in case there's a need for that control

    setView(sc);
    setProjection(sc);
    QFlags updateFlags = CameraUpdateFlags();
    updateFlags.setFlag(CameraUpdateFlag::kViewUpdated, true);
    updateFlags.setFlag(CameraUpdateFlag::kProjectionUpdated, true);
    m_camera->updateFrustum(updateFlags); // Need to update frustum for objects in view

    switch (m_lightComponent->getLightType()) {
    case Light::kPoint:
    {
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera);

        // Set light-space matrix in SSBO
        ShadowInfo& shadow = m_lightComponent->cachedShadow();
        const BoundingSphere& sphere = static_cast<const BoundingSphere&>(cam.frustum());
        shadow.setFarClipPlane(sphere.radius());
        //float camNearClip = cam.nearClip(); 
        //shadow.setNearClipPlane(camNearClip);
        m_lightComponent->updateShadow();
        break;
    }
    case Light::kDirectional:
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera);

        // Set light-space matrix in SSBO
        ShadowInfo& shadow = m_lightComponent->cachedShadow();
        const Matrix4x4g& proj = cam.renderProjection().projectionMatrix();
        const Matrix4x4g& view = cam.getViewMatrix();
        shadow.m_attributesOrLightMatrix = proj * view;
        m_lightComponent->updateShadow();
        break;
    }
    default:
        throw("Error, light type unrecognized");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::setView(const Scene& scene)
{
    // Create view matrix from light's point of view, to origin of scene
    switch (m_lightComponent->getLightType()) {
    case Light::kDirectional: {
        Camera& cam = *static_cast<Camera*>(m_camera);

        // This approach obtains a view matrix such that the light always emanates
        // from the edge of the frustum bounds

        // Create frustum of correct size/orientation, but not position
        Vector3 lightDir = m_lightComponent->cachedLight().getDirection();

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
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera);
        Vector3 lightPos = m_lightComponent->sceneObject()->transform()->getPosition().asFloat();
        static std::array<Vector3, 6> lookAtDirs = { {
            {1.0, 0.0, 0.0}, // right
            {-1.0, 0.0, 0.0}, // left
            {0.0, 1.0, 0.0}, // top
            {0.0, -1.0, 0.0}, // bottom
            {0.0, 0.0, 1.0}, // near
            {0.0, 0.0, -1.0} // far
        } };
        static std::array<Vector3, 6> upDirs = { {
            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, -1.0},
            {0.0, -1.0, 0.0},
            {0.0, -1.0, 0.0},
        } };
        for (size_t i = 0; i < 6; i++) {
            Camera& subCamera = cam.cameras()[i];
            subCamera.setViewMatrix(SceneCamera::LookAtRH(lightPos, lightPos + lookAtDirs[i], upDirs[i]));
        }
        break;
    }
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera);

        Vector3 lightPos = m_lightComponent->sceneObject()->transform()->getPosition().asFloat();
        Vector3 lightDir = m_lightComponent->cachedLight().getDirection();
        
        // TODO: Use this by default for LookAtRH, doesn't seem super important right now
        // Get "up" vector. See: 
        // https://www.gamedev.net/forums/topic/388559-getting-a-up-vector-from-only-having-a-forward-vector/
        Vector3 forward = lightPos - (lightPos + lightDir);
        forward.normalize();
        Vector3 up = Vector3f(0.0, 1.0, 0.0); // temp up
        Vector3 right = forward.cross(up);
        if (right.lengthSquared() > 1e-6) {
            // If forward is not in up direction, find camera up that is "up-ish"
            up = right.cross(forward);
        }
        else {
            up = Vector3f(1.0, 0.0, 0.0);
        }
        
        // NOTE: This will update the translation of the scene object accordingly
        //m_camera.setLookAt(lightPos, lightPos + lightDir, up);
        cam.setViewMatrix(SceneCamera::LookAtRH(lightPos, lightPos + lightDir, up));
        break;
    }
    default:
        throw("Error, type of light unrecognized");
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::setProjection(const Scene& scene)
{

    // Create perspective matrix
    switch (m_lightComponent->getLightType()) {
    case Light::kDirectional: {
        // Orthographic, since light does not originate from a point
        // Trying to fit the entire world in the orthographic box will cause low
        // shadow resolution, so build size around currently viewable area (view frustum)
        // See: https://www.gamedev.net/forums/topic/505893-orthographic-projection-for-shadow-mapping/

        // FIXME: Note, this approach will not take into account any shadow casters
        // That lay outside of the view frustum, that might cast shadows into the frustum,
        // May need to move the near clip plane further back (towards the light) to render
        // all shadow casters
        Camera& cam = *static_cast<Camera*>(m_camera);

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
        PointLightCamera& cam = *static_cast<PointLightCamera*>(m_camera);

        // Perspective projection, since light fans out from a point
        float fovDeg = 90.0f; // make sure the viewing field is exactly large enough to fill a single face of the cubemap such that all faces align correctly to each other at the edges
        float aspectRatio = 1.0f; // Aspect area will just be 1, since we just want a cone's frustum
        float nearClip = 1.0f;
        //float nearClip = lightComponent()->cachedShadow().nearClipPlane(); 
        float farClip = m_lightComponent->cachedLight().getRange();  // Far clip: Range of spot light
        for (size_t i = 0; i < 6; i++) {
            Camera& subCamera = cam.cameras()[i];
            subCamera.renderProjection().setPerspective(fovDeg, aspectRatio, nearClip, farClip);
        }

        break;
    }
    case Light::kSpot:
    {
        Camera& cam = *static_cast<Camera*>(m_camera);

        // Perspective projection, since light fans out from a point
        float fovDeg = m_lightComponent->cachedLight().getFOVRad() * Constants::RAD_TO_DEG; // Obtain from spot-light's outer cutoff value
        float aspectRatio = 1.0f; // Aspect area will just be 1, since we just want a cone's frustum
        float nearClip = 0.01f; // Near clip, something close to zero
        float farClip = m_lightComponent->cachedLight().getRange();  // Far clip: Range of spot light
        cam.renderProjection().setPerspective(fovDeg, aspectRatio, nearClip, farClip);
        
        //SceneCamera& debugCam = scene.scenario()->engine()->debugManager()->camera()->camera();
        //m_camera.renderProjection().projectionMatrix() = debugCam.renderProjection().projectionMatrix();
        break;
    }
    default:
        throw("Error, type of light unrecognized");
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void ShadowMap::initializeCamera(RenderContext& context, size_t mapIndex)
{
    // Retrieve textures containing all shadow maps supported by the engine    Light::LightType lightType = m_lightComponent->getLightType();
    Light::LightType lightType = m_lightComponent->getLightType();
    const auto& textures = context.lightingSettings().shadowTextures();
    const std::shared_ptr<Texture>& tex = textures[(int)lightType];
    
    Transform* lightObjectTransform = m_lightComponent->sceneObject()->transform().get();
    
    if (lightType != Light::kPoint) {
        Camera* camera = new Camera(
            lightObjectTransform,
            context.context(),
            AliasingType::kDefault,
            FrameBuffer::BufferAttachmentType::kTexture,
            4,  // Not using MSAA, so doesn't matter
            0); // Don't need any color attachments

        // Set camera properties
        camera->renderProjection().setAspectRatio(1.0, false);

        // Initialize framebuffer, attaching shadow map textures
        camera->frameBuffer().reinitialize(tex->width(), tex->height(), tex, mapIndex);
    
        m_camera = camera;
    }
    else {
        PointLightCamera* camera = new PointLightCamera(lightObjectTransform, &context, mapIndex);

        // Set camera properties
        for (size_t i = 0; i < 6; i++) {
            Camera& subCamera = camera->cameras()[i];
            subCamera.renderProjection().setAspectRatio(1.0, false);
        }

        // TODO: Get multiple point lights working
        // Initialize framebuffer, attaching entire shadow map texture
        if(context.lightingSettings().shadowMaps().size() == 0){
            // Only need to attach for first point light
            camera->frameBuffer().reinitialize(tex->width(),
                tex->height(),
                tex,
                mapIndex); // Unused
        }

        m_camera = camera;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
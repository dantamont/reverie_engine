#include "core/components/GCubemapComponent.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include "core/components/GTransformComponent.h"

#include "logging/GLogger.h"

#include "core/rendering/models/GModel.h"
#include "core/rendering/materials/GCubeTexture.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/geometry/GMesh.h"

#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

namespace rev {

CubeMapComponent::CubeMapComponent() :
    Component(ComponentType::kCubeMap, true),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
}

CubeMapComponent::CubeMapComponent(const CubeMapComponent & component) :
    Component(component.sceneObject(), ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    sceneObject()->setComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}

CubeMapComponent::CubeMapComponent(const std::shared_ptr<SceneObject>& object, const nlohmann::json& json) :
    Component(object, ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    json.get_to(*this);
    sceneObject()->setComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}

CubeMapComponent::CubeMapComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    sceneObject()->setComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}

rev::CubeMapComponent::~CubeMapComponent()
{
    if (!scene()) {
        Logger::LogError("Error, no scene for cubemap component");
    }
    scene()->removeCubeMap(this);
}

void CubeMapComponent::enable()
{
    Component::enable();
}
 
void CubeMapComponent::disable()
{
    Component::disable();
}

void CubeMapComponent::setDefault()
{
    if (sceneObject()) {
        if (sceneObject()->scene()) {
            sceneObject()->scene()->setDefaultCubeMap(this);
        }
    }
}

bool CubeMapComponent::isDefault() const
{
    CubeMapComponent* defaultCubeMap = sceneObject()->scene()->defaultCubeMap();
    if (!defaultCubeMap)
        return false;
    else
        return m_uuid == sceneObject()->scene()->defaultCubeMap()->getUuid();
}

void CubeMapComponent::draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings * settings, size_t drawFlags)
{
    G_UNUSED(drawFlags);
    G_UNUSED(context);
    G_UNUSED(settings);
    gl::OpenGLFunctions& functions = *gl::OpenGLFunctions::Functions();

    if (!m_cubeTextureHandle) return;
    if (m_cubeTextureHandle->isLoading()) {
#ifdef DEBUG_MODE
        Logger::LogInfo("Cube texture not yet loaded, returning");
#endif
        return;
    }

#ifdef DEBUG_MODE
    if (!m_cubeTextureHandle->isConstructed()) {
        Logger::Throw("Error, loading flag should be sufficient");
        return;
    }
#endif

    if (!shaderProgram.handle()->isConstructed()) return;

    // Lock mutex to mesh and texture handles
    QMutexLocker meshLocker(&m_meshHandle->mutex());

    // Cast resource
    CubeTexture* cubeTexture = texture();

    // Return if no resource
    if (!cubeTexture) {
#ifdef DEBUG_MODE
        Logger::Throw("Error, cube texture should be found");
#endif
        return;
    }

    QMutexLocker texLocker(&m_cubeTextureHandle->mutex());

    DepthSetting currentGLSetting = DepthSetting::Current(*context);

    // Turn off depth mask (stop writing to depth buffer)
    // The depth buffer will be filled with values of 1.0 for the skybox, so we need to make sure the skybox passes the depth tests with values less than or equal to the depth buffer instead of less than. 
    //functions.glDepthMask(GL_FALSE);
    //functions.glDepthFunc(GL_LEQUAL); 
    DepthSetting depthSetting(
        DepthPassMode::kLessEqual,
        false);
	depthSetting.bind(*context);

    // Bind shader program
    shaderProgram.bind();

    // Set shader uniforms
    bindUniforms(shaderProgram, *cubeTexture);
    //shaderProgram.updateUniforms(); ///< Removed on 8/2/22. See if this breaks the cubemap

    // Bind texture
    cubeTexture->bind(0);

#ifdef DEBUG_MODE
    functions.printGLError("Error binding cube texture");
#endif

    // Draw geometry (is only one set of mesh data for a cubemap mesh)
    mesh()->vertexData().drawGeometry(PrimitiveMode::kTriangles, 1);

    // Release texture
    cubeTexture->release();

    // Turn depth mask (depth writing) back on
    //functions.glDepthMask(GL_TRUE);
	depthSetting.release(*context);
}

void CubeMapComponent::setCubeTexture(const GString & filepath)
{
    m_cubeTextureHandle = ResourceCache::Instance().getTopLevelHandleWithPath(filepath);
    if (!m_cubeTextureHandle) {
        m_cubeTextureHandle = CubeTexture::CreateHandle(m_scene->engine(), filepath);
        m_cubeTextureHandle->loadResource();
    }
}

CubeTexture* CubeMapComponent::texture()
{
    return m_cubeTextureHandle->resourceAs<CubeTexture>();
}


void to_json(json& orJson, const CubeMapComponent& korObject)
{
    ToJson<Component>(orJson, korObject);
    orJson["uuid"] = korObject.m_uuid;
    if (!korObject.m_name.isEmpty()) {
        orJson["name"] = korObject.m_name.c_str();
    }
    if (korObject.m_cubeTextureHandle) {
        orJson["texture"] = *korObject.m_cubeTextureHandle;
    }
    orJson["diffuseColor"] = korObject.m_color.toVector<Real_t, 3>();

    orJson["isDefault"] = korObject.isDefault(); ///< For widgets
}

void from_json(const json& korJson, CubeMapComponent& orObject)
{
    FromJson<Component>(korJson, orObject);
    FromJson<Renderable>(korJson, orObject);
    
    if (korJson.contains("name")) {
        orObject.m_name = korJson["name"].get_ref<const std::string&>().c_str();
    }
    if (korJson.contains("uuid")) {
        orObject.m_uuid = korJson["uuid"];
    }
    if (korJson.contains("texture")) {
        orObject.m_cubeTextureHandle = ResourceCache::Instance().getHandle(korJson.at("texture"));
    }
    if (korJson.contains("diffuseColor")) {
        orObject.m_color = Color(Vector3f(korJson["diffuseColor"]));
    }
}

void CubeMapComponent::initialize()
{
    // Roundabout way of getting shared pointer to the cube handle
    const Uuid& cubeId = ResourceCache::Instance().polygonCache()->getCube()->handle()->getUuid();
    m_meshHandle = ResourceCache::Instance().getHandle(cubeId);

    // Initialize default uniform values
    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_uniforms.m_cubeTextureIndex.setValue(Int32_t(0), uc);
}


void CubeMapComponent::bindUniforms(ShaderProgram& shaderProgram, CubeTexture& cubeTexture)
{
    G_UNUSED(cubeTexture);
    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    // Set texture
    // Cubemap shader uses 0 texture unit by default
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_cubeTexture, 
        m_uniforms.m_cubeTextureIndex);

    // Set diffuse color
    /// @todo Only do this when color changes
    m_uniforms.m_diffuseColor.setValue(m_color.toVector<Float32_t, 3>(), uc);
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_diffuseColor,
        m_uniforms.m_diffuseColor
    );

    /// @todo Only do this when scene object moves
    // Set world matrix uniform
    const auto& worldMatrix = sceneObject()->transform().worldMatrix();
    m_uniforms.m_worldMatrix.setValue(worldMatrix, uc);
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_worldMatrix, 
        m_uniforms.m_worldMatrix
    );

}


} // end namespacing
#include "GCubemapComponent.h"

#include "../GCoreEngine.h"
#include "../resource/GResource.h"
#include "../resource/GResourceCache.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../components/GTransformComponent.h"

#include "../rendering/models/GModel.h"
#include "../rendering/materials/GCubeTexture.h"
#include "../rendering/renderer/GRenderContext.h"
#include "../rendering/geometry/GMesh.h"

#include "../rendering/shaders/GShaderProgram.h"

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent::CubeMapComponent() :
    Component(ComponentType::kCubeMap, true),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent::CubeMapComponent(const CubeMapComponent & component) :
    Component(component.sceneObject(), ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    sceneObject()->addComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent::CubeMapComponent(const std::shared_ptr<SceneObject>& object, const QJsonValue & json) :
    Component(object, ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    loadFromJson(json);
    sceneObject()->addComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent::CubeMapComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCubeMap),
    m_cubeTextureHandle(nullptr),
    m_color(255, 255, 255)
{
    initialize();
    sceneObject()->addComponent(this);
    sceneObject()->scene()->addCubeMap(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::CubeMapComponent::~CubeMapComponent()
{
    if (!scene()) {
        logError("Error, no scene for cubemap component");
    }
    scene()->removeCubeMap(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::setDefault()
{
    if (sceneObject()) {
        if (sceneObject()->scene()) {
            sceneObject()->scene()->setDefaultCubeMap(this);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CubeMapComponent::isDefault() const
{
    CubeMapComponent* defaultCubeMap = sceneObject()->scene()->defaultCubeMap();
    if (!defaultCubeMap)
        return false;
    else
        return m_uuid == sceneObject()->scene()->defaultCubeMap()->getUuid();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings * settings, size_t drawFlags)
{
    Q_UNUSED(drawFlags);
    Q_UNUSED(context);
    Q_UNUSED(settings);
    GL::OpenGLFunctions& functions = *GL::OpenGLFunctions::Functions();

    if (!m_cubeTextureHandle) return;
    if (m_cubeTextureHandle->isLoading()) {
#ifdef DEBUG_MODE
        logInfo("Cube texture not yet loaded, returning");
#endif
        return;
    }

#ifdef DEBUG_MODE
    if (!m_cubeTextureHandle->isConstructed()) {
        throw("Error, loading flag should be sufficient");
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
        throw("Error, cube texture should be found");
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
    shaderProgram.updateUniforms();

    // Bind texture
    cubeTexture->bind(0);

#ifdef DEBUG_MODE
    functions.printGLError("Error binding cube texture");
#endif

    // Draw geometry (is only one set of mesh data for a cubemap mesh)
    mesh()->vertexData().drawGeometry(PrimitiveMode::kTriangles);

    // Release texture
    cubeTexture->release();

    // Turn depth mask (depth writing) back on
    //functions.glDepthMask(GL_TRUE);
	depthSetting.release(*context);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::setCubeTexture(const QString & filepath)
{
    m_cubeTextureHandle = m_scene->engine()->resourceCache()->getTopLevelHandleWithPath(filepath);
    if (!m_cubeTextureHandle) {
        m_cubeTextureHandle = CubeTexture::CreateHandle(m_scene->engine(), filepath);
        m_cubeTextureHandle->loadResource();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTexture* CubeMapComponent::texture()
{
    return m_cubeTextureHandle->resourceAs<CubeTexture>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CubeMapComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    object.insert("uuid", m_uuid.asQString());
    if(!m_name.isEmpty())
        object.insert("name", m_name.c_str());
    if(m_cubeTextureHandle)
        object.insert("texture", m_cubeTextureHandle->asJson());
    object.insert("diffuseColor", m_color.toVector3g().asJson());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Component::loadFromJson(json, context);
    Renderable::loadFromJson(json, context);
    const QJsonObject& object = json.toObject();
    
    if(object.contains("name"))
        m_name = object["name"].toString();
    if(object.contains("uuid"))
        m_uuid = Uuid(object["uuid"].toString());
    if (object.contains("texture")) {
        m_cubeTextureHandle = m_scene->engine()->resourceCache()->getHandle(object.value("texture"));
    }
    if (object.contains("diffuseColor")) {
        m_color = Color(Vector3f(object["diffuseColor"]));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::initialize()
{
    // Roundabout way of getting shared pointer to the cube handle
    const Uuid& cubeId = m_scene->engine()->resourceCache()->polygonCache()->getCube()->handle()->getUuid();
    m_meshHandle = m_scene->engine()->resourceCache()->getHandle(cubeId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::bindUniforms(ShaderProgram& shaderProgram, CubeTexture& cubeTexture)
{
    Q_UNUSED(cubeTexture)

    // Set texture
    // Cubemap shader uses 0 texture unit by default
    shaderProgram.setUniformValue("cubeTexture", 0);

    // Set diffuse color
    shaderProgram.setUniformValue("diffuseColor", m_color.toVector3g());

    // Set world matrix uniform
    const auto& worldMatrix = sceneObject()->transform().worldMatrix();
    shaderProgram.setUniformValue("worldMatrix", worldMatrix);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
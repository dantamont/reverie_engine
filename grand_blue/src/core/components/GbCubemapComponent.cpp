#include "GbCubemapComponent.h"

#include "../GbCoreEngine.h"
#include "../resource/GbResource.h"
#include "../resource/GbResourceCache.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../components/GbTransformComponent.h"

#include "../rendering/models/GbModel.h"
#include "../rendering/materials/GbCubeTexture.h"
#include "../rendering/renderer/GbRenderContext.h"

#include "../rendering/shaders/GbShaders.h"

namespace Gb {
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
Gb::CubeMapComponent::~CubeMapComponent()
{
    if (sceneObject()) {
        if (sceneObject()->scene())
            sceneObject()->scene()->removeCubeMap(this);
    }
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
    QMutexLocker meshLocker(&m_cubeMesh->handle()->mutex());

    // Cast resource
    std::shared_ptr<CubeTexture> cubeTexture = texture();

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
    bindUniforms(shaderProgram, cubeTexture);
    shaderProgram.updateUniforms();

    // Bind texture
    cubeTexture->bind();

#ifdef DEBUG_MODE
    functions.printGLError("Error binding cube texture");
#endif

    // Draw geometry (is only one set of mesh data for a cubemap mesh)
    m_cubeMesh->vertexData().drawGeometry(GL_TRIANGLES);

    // Release texture
    cubeTexture->release();

    // Turn depth mask (depth writing) back on
    //functions.glDepthMask(GL_TRUE);
	depthSetting.release(*context);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::setCubeTexture(const QString & filepath)
{
    m_cubeTextureHandle = m_engine->resourceCache()->getTopLevelHandleWithPath(filepath);
    if (!m_cubeTextureHandle) {
        m_cubeTextureHandle = CubeTexture::createHandle(m_engine, filepath);
        m_cubeTextureHandle->loadResource();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CubeTexture> CubeMapComponent::texture()
{
    return m_cubeTextureHandle->resourceAs<CubeTexture>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CubeMapComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
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
    if(object.contains("texture"))
        m_cubeTextureHandle = m_engine->resourceCache()->getHandle(object.value("texture"));
    if (object.contains("diffuseColor")) {
        m_color = Color(Vector3f(object["diffuseColor"]));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::initialize()
{
    m_cubeMesh = m_engine->resourceCache()->polygonCache()->getCube();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponent::bindUniforms(ShaderProgram& shaderProgram, const std::shared_ptr<CubeTexture>& cubeTexture)
{
    Q_UNUSED(cubeTexture)

    // Set texture
    // Cubemap shader uses 0 texture unit by default
    shaderProgram.setUniformValue("cubeTexture", 0);

    // Set diffuse color
    shaderProgram.setUniformValue("diffuseColor", m_color.toVector3g());

    // Set world matrix uniform
    const auto& worldMatrix = sceneObject()->transform()->worldMatrix();
    shaderProgram.setUniformValue("worldMatrix", worldMatrix);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
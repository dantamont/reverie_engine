#include "GbGlyph.h"
#include "../GbCoreEngine.h"

#include "../resource/GbResourceCache.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbPolygon.h"
#include "../rendering/shaders/GbShaders.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbTransformComponent.h"
#include "../components/GbCamera.h"
#include "../scene/GbSceneObject.h"
#include "../rendering/shaders/GbUniformBufferObject.h"

#include "GbLabel.h"
#include "GbIcon.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Glyph> Glyph::createGlyph(CanvasComponent* canvas, const QJsonObject & object)
{
    auto engine = canvas->sceneObject()->engine();

    // Set OpenGL context to main context to avoid VAO struggles
    engine->setGLContext();

    GlyphType type = GlyphType(object.value("type").toInt());
    std::shared_ptr<Glyph> glyph;
    switch (type) {
    case kLabel:
        glyph = std::make_shared<Label>(canvas);
        break;
    case kIcon:
        glyph = std::make_shared<Icon>(canvas);
        break;
    default:
        throw("Error, glyph of this type is not recognized");
    }
    glyph->loadFromJson(object);

    return glyph;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Glyph::Glyph(CanvasComponent* canvas, GlyphType type, GlyphMode mode) :
    m_canvas(canvas),
    m_type(type),
    m_mode(mode),
    m_flags({ kFaceCamera }),
    m_verticalAlignment(VerticalAlignment::kMiddle),
    m_horizontalAlignment(HorizontalAlignment::kCenter),
    m_transform(std::make_shared<Transform>())
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
Glyph::~Glyph()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::setAlignment(VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment)
{
    m_verticalAlignment = verticalAlignment;
    m_horizontalAlignment = horizontalAlignment;
    reload();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::setTrackedObject(const std::shared_ptr<SceneObject>& object)
{
    m_object = object;
    m_transform = object->transform();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Vector4g Glyph::screenPos() const
//{
//    const Vector3& p = m_transform.getPosition();
//    Vector4g pos(p.x(), p.y(), p.z(), real_g(1.0));
//    pos[3] = real_g(1.0);
//    Matrix4x4g projMatrix = m_canvas->renderProjection().projectionMatrix();
//    switch (m_mode) {
//    case kGUI:
//        pos = projMatrix * pos;
//        break;
//    case kBillboardScaled:
//        if (m_canvas->camera()) {
//            pos = projMatrix * m_canvas->camera()->getViewMatrix() * pos;
//        }
//        else {
//            pos = projMatrix * pos;
//        }
//        break;
//    case kBillboardFixed:
//        break;
//    }
//    return pos;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Glyph::asJson() const
{
    QJsonObject object = Renderable::asJson().toObject();
    object.insert("type", int(m_type));
    object.insert("mode", int(m_mode));

    //// Get name of mesh
    //object.insert("mesh", m_mesh->getName());

    // Get transform or owner scene object
    if (sceneObject()) {
        object.insert("sceneObject", sceneObject()->getName());
    }
    else {
        object.insert("transform", m_transform->asJson());
    }
    

    // Get coordinates
    object.insert("coordinates", m_coordinates.asJson());

    // Get flags
    if (m_mode == kBillboard) {
        object.insert("faceCamera", facesCamera());
        object.insert("scaleWithDistance", scalesWithDistance());
        object.insert("onTop", onTop());
    }

    // Get alignment
    object.insert("vertAlign", (int)m_verticalAlignment);
    object.insert("horAlign", (int)m_horizontalAlignment);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);

    m_type = GlyphType(object.value("type").toInt());
    m_mode = GlyphMode(object.value("mode").toInt(0));

    //// Load mesh
    //QString meshName = object.value("mesh").toString();
    //m_mesh = engine->resourceCache()->getMesh(meshName);

    // Load transform
    if (object.contains("transform")) {
        m_transform = std::make_shared<Transform>(object.value("transform"));
    }
    else if (object.contains("sceneObject")) {
        m_objectName = object["sceneObject"].toString();
        auto sceneObject = SceneObject::getByName(m_objectName);
        if (sceneObject) {
            m_object = sceneObject;
            m_transform = sceneObject->transform();
        }
        else {
            // Set transform to nullptr, to be set on first draw attempt
            m_transform = nullptr;
        }
    }

    // Load coordinates
    if (object.contains("coordinates")) {
        m_coordinates = Vector2g(object.value("coordinates"));
    }

    // Load flags
    if (m_mode == kBillboard) {
        setFaceCamera(object["faceCamera"].toBool(false));
        setScaleWithDistance(object["scaleWithDistance"].toBool(true));
        setOnTop(object["onTop"].toBool(false));
    }

    // Load alignment
    m_verticalAlignment = (VerticalAlignment)object["vertAlign"].toInt(1);
    m_horizontalAlignment = (HorizontalAlignment)object["horAlign"].toInt(1);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VertexArrayData * Glyph::getMeshData()
//{
//    return std::static_pointer_cast<Mesh>(m_mesh->resource(false))->meshData().begin()->second;
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//std::shared_ptr<ResourceHandle> Glyph::getSquare()
//{
//    std::shared_ptr<PolygonCache> cache = CoreEngine::engines().begin()->second->resourceCache()->polygonCache();
//    std::shared_ptr<ResourceHandle> mesh = cache->getSquare();
//    return mesh;
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::bindUniforms(const std::shared_ptr<Gb::ShaderProgram>& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);

    if (!UBO::getCameraBuffer()) return;
    std::shared_ptr<UBO> cameraBuffer = UBO::getCameraBuffer();

    // Set projection, view and world matrices
    const Vector2g& p = m_coordinates;
    Vector4g pos(p.x(), p.y(), real_g(0.0), real_g(1.0));
    switch (m_mode) {
    case kGUI:
    {
        // Cache previous uniforms
        Vec::EmplaceBack(m_uniformCache, std::move(cameraBuffer->getUniformValue("viewMatrix")));
        Vec::EmplaceBack(m_uniformCache, std::move(cameraBuffer->getUniformValue("projectionMatrix")));

        m_canvas->renderProjection().setProjectionType(RenderProjection::kOrthographic);
        shaderProgram->setUniformValue("perspectiveInverseScale", Matrix3x3g());
        cameraBuffer->setUniformValue("projectionMatrix", m_canvas->renderProjection().projectionMatrix());

        // View matrix should be identity
        cameraBuffer->setUniformValue("viewMatrix", m_canvas->ViewMatrix());
        shaderProgram->setUniformValue("faceCamera", false);
        shaderProgram->setUniformValue("onTop", onTop());
        shaderProgram->setUniformValue("scaleWithDistance", false);

        break;
    }
    case kBillboard:
    {
        // Fix perspective scaling
        const Matrix4x4g& projectionMatrix = cameraBuffer->getUniformValue("projectionMatrix").get<Matrix4x4g>();
        Matrix3x3g inversedPerspectiveScale = Matrix3x3g(projectionMatrix).inversed();
        shaderProgram->setUniformValue("perspectiveInverseScale", inversedPerspectiveScale);

        // Set flag uniforms
        shaderProgram->setUniformValue("scaleWithDistance", scalesWithDistance());
        shaderProgram->setUniformValue("faceCamera", facesCamera());
        shaderProgram->setUniformValue("onTop", onTop());

        break;
    }
    default:
        throw("Type of glyph unrecognized");
    }

    // Set world matrix uniform
    checkTransform();
    shaderProgram->setUniformValue("worldMatrix", m_transform->worldMatrix());

    // Set screen offset uniform
    shaderProgram->setUniformValue("screenOffset", m_coordinates);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::releaseUniforms(const std::shared_ptr<Gb::ShaderProgram>& shaderProgram)
{
    auto cameraBuffer = UBO::getCameraBuffer();
    if (!cameraBuffer) return;
    bool updateShader = false;
    for (const Uniform& uniform : m_uniformCache) {
        if (shaderProgram->hasUniform(uniform.getName())) {
            updateShader = true;
            shaderProgram->setUniformValue(uniform);
        }
        else {
            cameraBuffer->setUniformValue(uniform);
        }
    }
    shaderProgram->updateUniforms();
    m_uniformCache.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::checkTransform()
{
    if (m_transform) return;

    // If there is no transform, need to set from scene object
    if (!sceneObject()) {
        // Retrieve scene object if not yet set
        if (!m_objectName.isEmpty()) {
            m_object = SceneObject::getByName(m_objectName);
            if (sceneObject()) {
                m_transform = sceneObject()->transform();
            }
            else {
                logWarning("Glyph::checkTransform:: Scene object with the name " 
                    + m_objectName + " does not exist");
            }
        }
        else {
            // No object name specified, so set to default transform
            m_transform = std::make_shared<Transform>();
        }
    }
    else {
        logError("Error, scene object not found, but glyph missing transform");
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

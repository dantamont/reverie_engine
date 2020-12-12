#include "GbGlyph.h"
#include "../GbCoreEngine.h"

#include "../resource/GbResourceCache.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbPolygon.h"
#include "../rendering/shaders/GbShaders.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbTransformComponent.h"
#include "../components/GbCameraComponent.h"
#include "../scene/GbSceneObject.h"
#include "../rendering/buffers/GbUniformBufferObject.h"

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
    m_glyphMode(mode),
    m_flags({ kFaceCamera }),
    m_verticalAlignment(VerticalAlignment::kMiddle),
    m_horizontalAlignment(HorizontalAlignment::kCenter),
    m_moveMode(kIndependent),
    m_transform(std::make_shared<Transform>())
{
    m_transparencyType = TransparencyType::kTransparentNormal;
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
    if (!object) {
        if (m_moveMode != kIndependent) {
            throw("Error, movement mode is not set to be independent");
        }
        //m_moveMode = kIndependent;
        m_transform = std::make_shared<Transform>();
    }
    else {
        if (m_moveMode != kTrackObject) {
            throw("Error, glyph not set to track object");
        }

        m_trackedObject = object;
        m_transform = object->transform();
    }

    checkTransform();
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
    object.insert("mode", int(m_glyphMode));

    // Get movement mode
    object.insert("moveMode", int(m_moveMode));

    // Get transform or owner scene object
    if (trackedObject()) {
        object.insert("sceneObject", trackedObject()->getName().c_str());
    }
    else {
        object.insert("transform", m_transform->asJson());
    }
    

    // Get coordinates
    object.insert("coordinates", m_coordinates.asJson());

    // Get flags
    if (m_glyphMode == kBillboard) {
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
void Glyph::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);

    m_type = GlyphType(object.value("type").toInt());
    m_glyphMode = GlyphMode(object.value("mode").toInt(0));

    // Set movement mode
    m_moveMode = MovementMode(object["moveMode"].toInt(0));

    // Load transform
    if (object.contains("transform")) {
        m_transform = std::make_shared<Transform>(object.value("transform"));
    }
    else if (object.contains("sceneObject")) {
        QString sceneObjectName = object["sceneObject"].toString();
        auto sceneObject = SceneObject::getByName(sceneObjectName);
        if (sceneObject) {
            m_trackedObject = sceneObject;
            m_transform = sceneObject->transform();
        }
        else {
            // Set transform to nullptr, to be set on first draw attempt
            m_transform = nullptr;
        }
    }

    // Load coordinates
    if (object.contains("coordinates")) {
        m_coordinates = Vector2(object.value("coordinates"));
    }

    // Load flags
    if (m_glyphMode == kBillboard) {
        setFaceCamera(object["faceCamera"].toBool(false));
        setScaleWithDistance(object["scaleWithDistance"].toBool(true));
        setOnTop(object["onTop"].toBool(false));
    }

    // Load alignment
    m_verticalAlignment = (VerticalAlignment)object["vertAlign"].toInt(1);
    m_horizontalAlignment = (HorizontalAlignment)object["horAlign"].toInt(1);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::preDraw()
{
    m_canvas->resizeViewport();
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
void Glyph::bindUniforms(Gb::ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);

    if (!UBO::getCameraBuffer()) return;
    std::shared_ptr<UBO> cameraBuffer = UBO::getCameraBuffer();

    // Set projection, view and world matrices
    //const Vector2g& p = m_coordinates;
    //Vector4g pos(p.x(), p.y(), real_g(0.0), real_g(1.0));
    switch (m_glyphMode) {
    case kGUI:
    {
        // Cache previous uniforms
        Vec::EmplaceBack(m_uniformCache, cameraBuffer->getUniformValue("viewMatrix"));
        Vec::EmplaceBack(m_uniformCache, cameraBuffer->getUniformValue("projectionMatrix"));

        m_canvas->renderProjection().setProjectionType(RenderProjection::kOrthographic);
        shaderProgram.setUniformValue("perspectiveInverseScale", Matrix3x3g());
        cameraBuffer->setUniformValue("projectionMatrix", m_canvas->renderProjection().projectionMatrix());

        // View matrix should be identity
        cameraBuffer->setUniformValue("viewMatrix", m_canvas->ViewMatrix());
        shaderProgram.setUniformValue("faceCamera", false);
        shaderProgram.setUniformValue("onTop", onTop());
        shaderProgram.setUniformValue("scaleWithDistance", false);

        break;
    }
    case kBillboard:
    {
        // Fix perspective scaling
        const Matrix4x4g& projectionMatrix = cameraBuffer->getUniformValue("projectionMatrix").get<Matrix4x4g>();
        Matrix3x3g inversedPerspectiveScale = Matrix3x3g(projectionMatrix).inversed();
        shaderProgram.setUniformValue("perspectiveInverseScale", inversedPerspectiveScale);

        // Set flag uniforms
        shaderProgram.setUniformValue("scaleWithDistance", scalesWithDistance());
        shaderProgram.setUniformValue("faceCamera", facesCamera());
        shaderProgram.setUniformValue("onTop", onTop());

        break;
    }
    default:
        throw("Type of glyph unrecognized");
    }

    // Set world matrix uniform
    shaderProgram.setUniformValue("worldMatrix", m_transform->worldMatrix());

    // Set screen offset uniform
    shaderProgram.setUniformValue("screenOffset", m_coordinates);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::releaseUniforms(Gb::ShaderProgram& shaderProgram)
{
    auto cameraBuffer = UBO::getCameraBuffer();
    if (!cameraBuffer) return;
    bool updateShader = false;
    ShaderProgram::UniformInfoIter iter;
    for (const Uniform& uniform : m_uniformCache) {
        if (shaderProgram.hasUniform(uniform.getName(), iter)) {
            updateShader = true;
            shaderProgram.setUniformValue(uniform);
        }
        else {
            cameraBuffer->setUniformValue(uniform);
        }
    }
    shaderProgram.updateUniforms();
    m_uniformCache.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::checkTransform()
{
    if (m_transform) return;

    switch (m_moveMode) {
    case kIndependent:
        m_transform = std::make_shared<Transform>();
        break;
    case kTrackObject:
        // If there is no transform, need to set from scene object
        if (!trackedObject()) {
            throw("Glyph::checkTransform:: Scene object does not exist");
        }
        m_transform = trackedObject()->transform();
        break;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#include "GGlyph.h"
#include "../GCoreEngine.h"

#include "../resource/GResourceCache.h"
#include "../rendering/geometry/GMesh.h"
#include "../rendering/geometry/GPolygon.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../components/GCanvasComponent.h"
#include "../components/GTransformComponent.h"
#include "../components/GCameraComponent.h"
#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"
#include "../rendering/buffers/GUniformBufferObject.h"

#include "GLabel.h"
#include "GIcon.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Glyph> Glyph::createGlyph(CanvasComponent* canvas, const QJsonObject & object)
{
    auto engine = canvas->sceneObject()->scene()->engine();

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
Glyph::Glyph(CanvasComponent* canvas) :
    m_canvas(canvas),
    m_verticalAlignment(VerticalAlignment::kMiddle),
    m_horizontalAlignment(HorizontalAlignment::kCenter),
    m_parentIndex(-1)
{
    // Need to mark glyphs as transparent so that blending can happen
    m_renderSettings.setTransparencyType(TransparencyRenderMode::kTransparentNormal);
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
Glyph * Glyph::getParent() const
{
    if (m_parentIndex < 0) {
        return nullptr;
    }
    else {
        return m_canvas->glyphs()[m_parentIndex].get();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::setParent(int idx)
{
    // Set parent index in glyph
    m_parentIndex = idx;

    if (idx >= 0) {
        // Index is an actual parent
        const std::shared_ptr<Glyph>& parent = m_canvas->glyphs()[idx];

        // Set transform's parent
        m_transform.setParent(&parent->transform());
    }
    else {
        // Index is invalid, so parent to canvas
        setParent(m_canvas);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::setParent(Glyph * glyph)
{
    if (glyph) {
        m_parentIndex = m_canvas->getIndex(*glyph);

        // Set transform's parent
        m_transform.setParent(&glyph->transform());
    }
    else {
        // Glyph is null, so parent to canvas
        m_parentIndex = -1;
        setParent(m_canvas);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::setParent(CanvasComponent * canvas)
{
    m_parentIndex = -1;
    const std::shared_ptr<SceneObject>& so = canvas->sceneObject();
    m_transform.setParent(&so->transform());
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
QJsonValue Glyph::asJson(const SerializationContext& context) const
{
    QJsonObject object = Renderable::asJson(context).toObject();
    object.insert("type", int(glyphType()));

    // Get transform
    object.insert("transform", m_transform.asJson());
    
    // Get alignment
    object.insert("vertAlign", (int)m_verticalAlignment);
    object.insert("horAlign", (int)m_horizontalAlignment);

    // Get parent index
    object.insert("parent", m_parentIndex);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);

    // Load transform
    if (!object.contains("transform")) {
        throw("No transform found for glyph");
    }
    m_transform = Transform(object.value("transform"));

    // Load alignment
    m_verticalAlignment = (VerticalAlignment)object["vertAlign"].toInt(1);
    m_horizontalAlignment = (HorizontalAlignment)object["horAlign"].toInt(1);

    // Load parent
    m_parentIndex = object["parent"].toInt(-1);
    if (m_parentIndex >= 0) {
        setParent(m_parentIndex);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::preDraw()
{
    m_canvas->resizeViewport();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::bindUniforms(rev::ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);

    if (!UBO::getCameraBuffer()) return;
    std::shared_ptr<UBO> cameraBuffer = UBO::getCameraBuffer();

    // Set projection, view and world matrices
    //const Vector2g& p = m_coordinates;
    //Vector4g pos(p.x(), p.y(), real_g(0.0), real_g(1.0));
    switch (m_canvas->glyphMode()) {
    case GlyphMode::kGUI:
    {
        // Since GUI projection is always the same, just make it a static local
        static Matrix4x4g orthoProjection = RenderProjection::Orthographic(-1.0f, 1.0f, -1.0f, 1.0f);

        // Cache previous uniforms
        Vec::EmplaceBack(m_uniformCache, cameraBuffer->getUniformValue("viewMatrix"));
        Vec::EmplaceBack(m_uniformCache, cameraBuffer->getUniformValue("projectionMatrix"));

        //m_canvas->renderProjection().setProjectionType(RenderProjection::kOrthographic);
        //cameraBuffer->setUniformValue("projectionMatrix", m_canvas->renderProjection().projectionMatrix());
        cameraBuffer->setUniformValue("projectionMatrix", orthoProjection);

        // View matrix should be identity
        cameraBuffer->setUniformValue("viewMatrix", Matrix4x4g::Identity());

        break;
    }
    case GlyphMode::kBillboard:
    {
        // Fix perspective scaling
        const Matrix4x4g& projectionMatrix = cameraBuffer->getUniformValue("projectionMatrix").get<Matrix4x4g>();
        Matrix3x3g inversedPerspectiveScale = Matrix3x3g(projectionMatrix).inversed();
        shaderProgram.setUniformValue("perspectiveInverseScale", inversedPerspectiveScale);

        // Set flag uniforms
        shaderProgram.setUniformValue("scaleWithDistance", m_canvas->scalesWithDistance());
        shaderProgram.setUniformValue("faceCamera", m_canvas->facesCamera());

        break;
    }
    default:
        throw("Glyph mode unrecognized");
    }

    // Set whether or not glyph is always on top
    shaderProgram.setUniformValue("onTop", m_canvas->alwaysOnTop());

    // Set world matrix uniform
    shaderProgram.setUniformValue("worldMatrix", m_transform.worldMatrix());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Glyph::releaseUniforms(rev::ShaderProgram& shaderProgram)
{
    auto cameraBuffer = UBO::getCameraBuffer();
    if (!cameraBuffer) return;
    ShaderProgram::UniformInfoIter iter;
    for (const Uniform& uniform : m_uniformCache) {
        if (cameraBuffer->hasUniform(uniform.getName())) {
            // If uniform belongs to camera buffer, set
            cameraBuffer->setUniformValue(uniform);
        }
        else if (shaderProgram.hasUniform(uniform.getName(), iter)) {
            // Otherwise, simply set in shader
            shaderProgram.setUniformValue(uniform);
        }
    }
    shaderProgram.updateUniforms();
    m_uniformCache.clear();
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

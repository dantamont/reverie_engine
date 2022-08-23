#include "core/canvas/GGlyph.h"
#include "core/GCoreEngine.h"

#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/components/GCanvasComponent.h"
#include "core/components/GTransformComponent.h"
#include "core/components/GCameraComponent.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

#include "core/canvas/GLabel.h"
#include "core/canvas/GIcon.h"
#include "core/canvas/GSprite.h"

namespace rev {

std::shared_ptr<Glyph> Glyph::CreateGlyph(CanvasComponent* canvas, const json & korJson)
{
    auto engine = canvas->sceneObject()->scene()->engine();

    // Set OpenGL context to main context to avoid VAO struggles
    engine->setGLContext();

    // Create world matrix uniform
    RenderContext& context = engine->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    CanvasUniformIndices indices;
    indices.m_worldMatrix = canvas->m_glyphUniforms.m_worldMatrix.push(Matrix4x4::Identity(), uc);
    const UniformData& uniformData = canvas->m_glyphUniforms.m_worldMatrix[indices.m_worldMatrix];
#ifdef DEBUG_MODE
    assert(!uniformData.isEmpty() && "Invalid uniform data");
#endif
    std::vector<Matrix4x4>& matrixUniformVector = uc.getUniformVector<Matrix4x4>().m_uniforms;

    GlyphType type = GlyphType(korJson.at("type").get<Int32_t>());
    std::shared_ptr<Glyph> glyph;
    switch (type) {
    case kLabel:
    {
        std::shared_ptr<Label> label = prot_make_shared<Label>(canvas, matrixUniformVector, uniformData.m_storageIndex);
        korJson.get_to(*label);
        glyph = label;
        break;
    }
    case kIcon:
    {
        std::shared_ptr<Icon> icon = prot_make_shared<Icon>(canvas, matrixUniformVector, uniformData.m_storageIndex);
        korJson.get_to(*icon);
        glyph = icon;
        break;
    }
    case kSprite:
    {
        std::shared_ptr<Sprite> sprite = prot_make_shared<Sprite>(canvas, matrixUniformVector, uniformData.m_storageIndex);
        korJson.get_to(*sprite);
        glyph = sprite;
        break;
    }
    default:
        Logger::Throw("Error, glyph of this type is not recognized");
    }


    glyph->postConstruct(indices);

    return glyph;
}

Glyph::Glyph(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex) :
    m_canvas(canvas),
    m_verticalAlignment(VerticalAlignment::kMiddle),
    m_horizontalAlignment(HorizontalAlignment::kCenter),
    m_parentIndex(-1),
    m_transform(worldMatrixVec, worldMatrixIndex)
{
    // Need to mark glyphs as transparent so that blending can happen
    m_renderSettings.setTransparencyType(TransparencyRenderMode::kTransparentNormal);
}


Glyph::~Glyph()
{
}

void Glyph::setAlignment(VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment)
{
    m_verticalAlignment = verticalAlignment;
    m_horizontalAlignment = horizontalAlignment;
    reload();
}

json Glyph::asJson() const
{
    json j;
    switch (glyphType()) {
    case GlyphType::kLabel:
        ToJson<Label>(j, *this);
        break;
    case GlyphType::kIcon:
        ToJson<Icon>(j, *this);
        break;
    case GlyphType::kSprite:
        ToJson<Sprite>(j, *this);
        break;
    default:
        assert(false && "Unrecognized type");
    }
    return j;
}

Glyph * Glyph::getParent() const
{
    if (m_parentIndex < 0) {
        return nullptr;
    }
    else {
        return m_canvas->glyphs()[m_parentIndex].get();
    }
}

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

void Glyph::setParent(CanvasComponent * canvas)
{
    m_parentIndex = -1;
    const std::shared_ptr<SceneObject>& so = canvas->sceneObject();
    m_transform.setParent(&so->transform());
}

void Glyph::postConstruct(const CanvasUniformIndices& indices) {
    // Ensure that the glyph has valid uniform indices
    m_canvasIndices = indices;
#ifdef DEBUG_MODE
    assert(m_canvasIndices.m_worldMatrix != CanvasUniformIndices::s_invalidIndex && "Invalid world matrix index");
#endif

    RenderContext& context = m_canvas->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    if (m_canvasIndices.m_textureOffset == CanvasUniformIndices::s_invalidIndex) {
        m_canvasIndices.m_textureOffset = m_canvas->m_glyphUniforms.m_textureOffset.push(Vector2(), uc);
    }
    if (m_canvasIndices.m_textureScale == CanvasUniformIndices::s_invalidIndex) {
        m_canvasIndices.m_textureScale = m_canvas->m_glyphUniforms.m_textureScale.push(Vector2::Ones(), uc);
    }
    if (m_canvasIndices.m_textColor == CanvasUniformIndices::s_invalidIndex) {
        m_canvasIndices.m_textColor = m_canvas->m_glyphUniforms.m_textColor.push(Vector3::Ones(), uc);
    }
}

//Vector4g Glyph::screenPos() const
//{
//    const Vector3& p = m_transform.getPosition();
//    Vector4g pos(p.x(), p.y(), p.z(), Real_t(1.0));
//    pos[3] = Real_t(1.0);
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

void to_json(json& orJson, const Glyph& korObject)
{
    ToJson<Renderable>(orJson, korObject);
    orJson["type"] = int(korObject.glyphType());

    // Get transform
    orJson["transform"] = korObject.m_transform;
    
    // Get alignment
    orJson["vertAlign"] = korObject.m_verticalAlignment;
    orJson["horAlign"] = korObject.m_horizontalAlignment;

    // Get parent index
    orJson["parent"] = korObject.m_parentIndex;

    orJson["uuid"] = korObject.getUuid(); ///< @todo Pull out, for widgets only
}

void from_json(const json& korJson, Glyph& orObject)
{
    FromJson<Renderable>(korJson, orObject);

    // Load transform
    if (!korJson.contains("transform")) {
        Logger::Throw("No transform found for glyph");
    }
    korJson.at("transform").get_to(orObject.m_transform);

    // Load alignment
    orObject.m_verticalAlignment = (Glyph::VerticalAlignment)korJson.value("vertAlign", 1);
    orObject.m_horizontalAlignment = (Glyph::HorizontalAlignment)korJson.value("horAlign", 1);

    // Load parent
    orObject.m_parentIndex = korJson.value("parent", 0);
    if (orObject.m_parentIndex >= 0) {
        orObject.setParent(orObject.m_parentIndex);
    }
}

void Glyph::preDraw()
{
    m_canvas->resizeViewport();
}

void Glyph::bindUniforms(rev::ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);

    if (!Ubo::GetCameraBuffer()) return;
    const std::shared_ptr<CameraUbo>& cameraBuffer = Ubo::GetCameraBuffer();

    // Set projection, view and world matrices
    //const Vector2g& p = m_coordinates;
    //Vector4g pos(p.x(), p.y(), Real_t(0.0), Real_t(1.0));
    RenderContext& context = m_canvas->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    switch (m_canvas->glyphMode()) {
    case GlyphMode::kGUI:
    {
        // Cache previous uniforms
        Vec::EmplaceBack(m_cameraBufferUniformCache, cameraBuffer->getBufferUniformData<ECameraBufferUniformName::eViewMatrix>());
        Vec::EmplaceBack(m_cameraBufferUniformCache, cameraBuffer->getBufferUniformData<ECameraBufferUniformName::eProjectionMatrix>());

        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eProjectionMatrix>(m_canvas->orthoProjectionBufferUniformData());
        cameraBuffer->setBufferUniformValue<ECameraBufferUniformName::eViewMatrix>(m_canvas->viewMatrixBufferUniformData());

        break;
    }
    case GlyphMode::kBillboard:
    {
        // Fix perspective scaling
        /// @todo Update when camera perspective changes instead
        const Matrix4x4g& projectionMatrix = cameraBuffer->getBufferUniformValue<Matrix4x4>("projectionMatrix");
        Matrix3x3 inversedPerspectiveScale = Matrix3x3(projectionMatrix).inversed();
        m_canvas->m_glyphUniforms.m_perspectiveInverseScale.setValue(inversedPerspectiveScale, uc);
        shaderProgram.setUniformValue(
            shaderProgram.uniformMappings().m_perspectiveInverseScale,
            m_canvas->m_glyphUniforms.m_perspectiveInverseScale
        );

        // Set flag uniforms
        /// @todo Update when this flag is set instead of here.
        m_canvas->m_glyphUniforms.m_scaleWithDistance.setValue(m_canvas->scalesWithDistance(), uc);
        shaderProgram.setUniformValue(
            shaderProgram.uniformMappings().m_scaleWithDistance, 
            m_canvas->m_glyphUniforms.m_scaleWithDistance
        );

        /// @todo Update when this flag is set instead of here.
        m_canvas->m_glyphUniforms.m_faceCamera.setValue(m_canvas->facesCamera(), uc);
        shaderProgram.setUniformValue(
            shaderProgram.uniformMappings().m_faceCamera,
            m_canvas->m_glyphUniforms.m_faceCamera
        );

        break;
    }
    default:
        Logger::Throw("Glyph mode unrecognized");
    }

    // Set whether or not glyph is always on top
    /// @todo Update when this flag is set instead of here.
    m_canvas->m_glyphUniforms.m_onTop.setValue(m_canvas->alwaysOnTop(), uc);
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_onTop, 
        m_canvas->m_glyphUniforms.m_onTop
    );

    // Set world matrix uniform
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_worldMatrix, 
        m_canvas->m_glyphUniforms.m_worldMatrix[m_canvasIndices.m_worldMatrix]
    );

    // Set isAnimated uniform to false by default
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_texOffset, 
        m_canvas->m_glyphUniforms.m_textureOffset[m_canvasIndices.m_textureOffset]
    );
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_texScale,
        m_canvas->m_glyphUniforms.m_textureScale[m_canvasIndices.m_textureScale]
    );
}

void Glyph::releaseUniforms(rev::ShaderProgram& shaderProgram)
{
    /// @todo Only call this after drawing a set of glyphs with the same settings.
    auto cameraBuffer = Ubo::GetCameraBuffer();
    if (!cameraBuffer) {
        return;
    }

    RenderContext& context = m_canvas->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    for (const BufferUniformData& uniformData : m_cameraBufferUniformCache) {
        cameraBuffer->setBufferUniformValue(uniformData);
    }
    shaderProgram.updateUniforms(uc);
    m_cameraBufferUniformCache.clear();
}




} // End namespaces

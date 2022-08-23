#include "core/canvas/GLabel.h"

#include "fortress/json/GJson.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GFile.h"

#include "geppetto/qt/fonts/GFontManager.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/components/GCanvasComponent.h"
#include "core/rendering/renderer/GRenderSettings.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"

namespace rev {

tsl::robin_map<Uint32_t, std::unique_ptr<GlFontFaceDispatcher>> GlFontFaceDispatcher::s_dispatchers{};

GlFontFaceDispatcher& GlFontFaceDispatcher::Get(FontFace& fontFace)
{
    if (!s_dispatchers.contains(fontFace.m_id)) {
        s_dispatchers[fontFace.m_id] = prot_make_unique<GlFontFaceDispatcher>(fontFace);
    }
    return *s_dispatchers[fontFace.m_id];
}

void GlFontFaceDispatcher::Clear()
{
    s_dispatchers.clear();
}

GlFontFaceDispatcher::GlFontFaceDispatcher(FontFace& fontFace):
    m_fontFace(&fontFace)
{
    // Make sure the textures here get cleared when the font face does
    m_fontFace->s_clearedFontFaceSignal.connect(
        [this](Uint32_t id) {
            if (id == m_fontFace->m_id) {
                m_textures.clear();
            }
        }
    );
}

void GlFontFaceDispatcher::bindTexture(float fontSize)
{
    uint32_t pixelSize = m_fontFace->pointToPixelSize(fontSize);

    auto texture = m_textures.at(pixelSize)->resourceAs<Texture>();
    //int h = texture->height();
    //int w = texture->width();

    if (texture->handle()->isConstructed()) {
        // bind texture to first unreserved unit
        //texture->bind(NUM_SHADOW_MAP_TEXTURES);
        texture->bind(0);
    }
}

void GlFontFaceDispatcher::releaseTexture(float fontSize)
{
    uint32_t pixelSize = m_fontFace->pointToPixelSize(fontSize);
    if (m_textures.at(pixelSize)->isConstructed()) {
        m_textures.at(pixelSize)->resourceAs<Texture>()->release(); // bind texture to this unit
    }
}

void GlFontFaceDispatcher::ensureFontSize(float fontSize, CoreEngine* core)
{
    uint32_t pixelSize = m_fontFace->pointToPixelSize(fontSize);
    if (!Map::HasKey(m_textures, pixelSize)) {
#ifdef DEBUG_MODE
        std::cout << "Warning, font size not found, loading";
#endif
        loadGLTexture(core, pixelSize);
    }
}

void GlFontFaceDispatcher::loadGLTexture(CoreEngine* core, uint32_t fontPixelSize)
{
    // Ensure that there is a valid bitmap image
    FontBitmap* bm = m_fontFace->bitmap(fontPixelSize);
    if (!bm) {
        m_fontFace->loadBitmap(m_fontFace->pixelToPointSize(fontPixelSize));
        bm = m_fontFace->m_bitmaps.back().get();
    }

    // Create an image with the correct format
    Image bitmap = bm->m_texPacker.getImage(Image::ColorFormat::kGrayscale8);
#ifdef DEBUG_MODE
    if (bitmap.isNull()) {
        assert(false && "Error, bitmap is null");
    }
#endif

    // Create unique name for texture
    static std::atomic<uint32_t> fontCount = 0;
    GString filename(GFile(m_fontFace->m_path.c_str()).getFileName(false));
    GString uniqueName = "font_" + filename + "_" + GString::FromNumber(fontCount.load()) + "_" + GString::FromNumber(fontPixelSize);
    fontCount.fetch_add(1);

    // Create OpenGL texture, taking ownership of bitmap image
    m_textures[fontPixelSize] = Texture::CreateHandle(core,
        bitmap,
        TextureUsageType::kDiffuse,
        TextureFilter::kLinear,
        TextureFilter::kLinear,
        TextureWrapMode::kClampToEdge);
    m_textures[fontPixelSize]->setName(uniqueName);
    //m_textures[fontPixelSize]->postConstruct();

    // If this is a core font-face, preserve textures
    m_textures[fontPixelSize]->setCore(m_fontFace->m_isCore);
}




Label::Label(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex):
    Label(canvas, worldMatrixVec, worldMatrixIndex, kLabel, 40, "arial", "Default Text", ResourceBehaviorFlag::kRuntimeGenerated)
{
}

Label::Label(CanvasComponent * canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex,
    const QString & text, const QString & font, float fontSize,
    ResourceBehaviorFlags rFlags):
    Label(canvas, worldMatrixVec, worldMatrixIndex, kLabel, fontSize, font, text, rFlags)
{
}


Label::~Label()
{
}

void Label::setText(const QString & text)
{
    // Set text member
    m_text = text;
    populateVertexData();
}

void Label::setFontSize(float pointSize)
{
    m_fontSize = pointSize;
    populateVertexData();
}

void Label::setFontFace(const QString & faceName)
{
    m_fontFace = faceName;
    populateVertexData();
}

void Label::setLineMaxSize(float width)
{
    m_lineMaxWidth = width;
    populateVertexData();
}

void Label::setLineSpacing(float spacing)
{
    m_lineSpacing = spacing;
    populateVertexData();
}

void Label::reload()
{
    populateVertexData();
}

void to_json(json& orJson, const Label& korObject)
{
    ToJson<Glyph>(orJson, korObject);
    orJson["text"] = korObject.m_text.toStdString();
    orJson["fontSize"] = korObject.m_fontSize;
    orJson["font"] = korObject.m_fontFace.toStdString();
    orJson["color"] = korObject.m_color.toVector<Real_t, 4>();
    orJson["maxLineWidth"] = korObject.m_lineMaxWidth;
    orJson["lineSpacing"] = korObject.m_lineSpacing;
}

void from_json(const json& korJson, Label& orObject)
{
    FromJson<Glyph>(korJson, orObject);

    // Force transparency mode to be correct
    orObject.m_renderSettings.setTransparencyType(TransparencyRenderMode::kTransparentNormal);

    orObject.m_text = korJson.value("text", "Default Text").c_str();
    orObject.m_fontSize = korJson.value("fontSize", 40);
    orObject.m_fontFace = korJson.value("font", "arial").c_str();

    if (korJson.contains("color")) {
        orObject.m_color = Color(Vector4(korJson.at("color")));
    }
    
    orObject.m_lineMaxWidth = korJson.value("maxLineWidth", 1.0);
    orObject.m_lineSpacing = korJson.value("lineSpacing", 1.0);

    orObject.populateVertexData();
}

Label::Label(CanvasComponent * canvas, 
    std::vector<Matrix4x4>& worldMatrixVec, 
    Uint32_t worldMatrixIndex,
    GlyphType type,
    float fontSize,
    const QString& fontFace,
    const QString& text,
    ResourceBehaviorFlags resourceFlags) :
    Glyph(canvas, worldMatrixVec, worldMatrixIndex),
    //m_vertexData(std::make_shared<VertexArrayData>()),
    m_fontSize(fontSize),
    m_fontFace(fontFace),
    m_text(text),
    m_lineMaxWidth(1.0),
    m_lineSpacing(1.0)
{
    // Initialize an empty mesh 
    initializeEmptyMesh(ResourceCache::Instance(), resourceFlags);

    // Initialize render settings to enable blending and disable face culling
    m_renderSettings.addDefaultBlend();
    m_renderSettings.addSetting<CullFaceSetting>(false);
}

FontFace * Label::getFontFace()
{
    return FontManager::GetFontFace((GString)m_fontFace.toStdString());
}

void Label::populateVertexData()
{
    // Clear old attribute data
    Mesh* mesh = Renderable::mesh();
    //if (!mesh) {
    //    // May be null for debug labels on scenario load
    //    return;
    //}

    VertexArrayData& vertexData = mesh->vertexData();
    vertexData.m_attributes.clear();
    vertexData.m_indices.clear();

    // Iterate to generate lines
    QSize widgetSize = canvas()->mainGLWidgetDimensions();

    float xScale = 1.0f / widgetSize.width(); // m_transform.getScaleVec().x();
    float yScale = 1.0f / widgetSize.height(); // m_transform.getScaleVec().y();
    float x = 0.0f; // m_coordinates.x() * widgetSize.width();
    float y = 0.0f; // m_coordinates.y() * widgetSize.height();
    float startX = x;
    std::array<Vector3, 6> vertices;
    std::array<Vector2, 6> quadUVs{ {
            { 0.0, 0.0 },
            { 0.0, 1.0 },
            { 1.0, 1.0 },

            { 0.0, 0.0 },
            { 1.0, 1.0 },
            { 1.0, 0.0 }
    } };
    std::array<Vector2, 6> uvs;
    int count = 0;

    FontFace* face = getFontFace();
    face->loadBitmap(m_fontSize);
    float baseLineSpacing = face->getLineSpacing(m_fontSize) * yScale;

    // Adjust x and y position for alignment
    float verticalAdjustment = 0;
    float horizontalAdjustment = 0;
    TextMetrics metrics = getTextMetrics();
    float lineHeight = metrics.m_textHeight / metrics.m_numLines;
    switch (m_horizontalAlignment) {
    case kRight:
        horizontalAdjustment = -metrics.m_textWidth;
        break;
    case kCenter:
        horizontalAdjustment = -metrics.m_textWidth / 2.0;
        break;
    case kLeft:
        horizontalAdjustment = 0;
        break;
    }

    switch (m_verticalAlignment) {
    case kTop:
        //verticalAdjustment = (metrics.m_numLines - 1) * lineHeight;
        verticalAdjustment = - lineHeight / 2.0;
        break;
    case kMiddle:
        //verticalAdjustment = (metrics.m_numLines - 1) * lineHeight - 
        //    metrics.m_textHeight / 2.0;
        // TODO: Be more precise, these values are not scientific, they just look right.
        if (metrics.m_numLines == 1) {
            // Want to center directly on text, since there will be no line spacing
            verticalAdjustment = -0.5 * lineHeight / 2.0;
        }
        else {
            // - lineHeight / 2.0 was okay, but this is just better
            verticalAdjustment = (metrics.m_textHeight - 1.5 * lineHeight) / 2.0;
        }
        break;
    case kBottom:
        //verticalAdjustment = (metrics.m_numLines - 1) * lineHeight - metrics.m_textHeight;
        verticalAdjustment = metrics.m_textHeight - lineHeight;
        //verticalAdjustment = metrics.m_textHeight / 2.0 - lineHeight;
        break;
    }
    ////verticalAdjustment = 0;

    for (int j = 0; j < m_text.length(); j++) {
        QCharRef c = m_text[j];

        // Get the character corresponding to the letter
        unsigned long code = c.unicode();
        code = code;
        FontCharacter& ch = face->getCharacter(code, m_fontSize);

        // If x is too large, move to next line
        float advance = ch.m_advance >> 6;
        float lineDistance = x + advance * xScale - startX;
        if (lineDistance > m_lineMaxWidth) {
            x = startX;
            y -= baseLineSpacing * m_lineSpacing;
        }

        // Get the x and y positions of the character
        float xpos = x + ch.m_bearing.x() * xScale + horizontalAdjustment;
        float ypos = y - (ch.m_size.y() - ch.m_bearing.y()) * yScale + verticalAdjustment;

        float w = ch.m_size.x() * xScale;
        float h = ch.m_size.y() * yScale;

        // Update VBO for each character
        vertices = { {
            { xpos,     ypos + h,   0.0f },
            { xpos,     ypos,       0.0f },
            { xpos + w, ypos,       0.0f },

            { xpos,     ypos + h,   0.0f },
            { xpos + w, ypos,       0.0f },
            { xpos + w, ypos + h,   0.0f }
        } };

        // Generate UVs (coordinates are (0, 0) in top-left of texture)
        uvs = quadUVs;
        float glyphWidth = ch.m_size.x();
        float glyphHeight = ch.m_size.y();
        const Vector2i& textureSize = face->getBitmapSize(m_fontSize);
        float uv_x_scale = glyphWidth / float(textureSize.x());
        float uv_y_scale = glyphHeight / float(textureSize.y());
        Vector2f uv_origin = { ch.m_origin.x() / float(textureSize.x()),
            (ch.m_origin.y() / float(textureSize.y())) };
        for (size_t i = 0; i < uvs.size(); i++) {
            float uv_x = uvs[i].x();
            float uv_y = uvs[i].y();
            uvs[i] = Vector2(uv_x * uv_x_scale + uv_origin.x(),
                uv_y * uv_y_scale + uv_origin.y());
        }

        // Convert value of advance to pixels (2^6 = 64), and then scale
        x += (ch.m_advance >> 6) * xScale;

        vertexData.m_attributes.m_vertices.insert(vertexData.m_attributes.m_vertices.end(),
            vertices.begin(),
            vertices.end());

        vertexData.m_attributes.m_texCoords.insert(vertexData.m_attributes.m_texCoords.end(),
            uvs.begin(),
            uvs.end());

        //vertices.clear();

        for (int i = 0; i < 6; i++) {
            vertexData.m_indices.push_back(count++);
        }
    }

    // Update the vertex array data
    vertexData.loadIntoVAO();

    // Update the bounding box for the mesh
    mesh->generateBounds();
}

Label::TextMetrics Label::getTextMetrics()
{
    // Iterate to generate lines
    QSize widgetSize = canvas()->mainGLWidgetDimensions();

    float xScale = 1.0f / widgetSize.width(); // m_transform.getScaleVec().x();
    float yScale = 1.0f / widgetSize.height(); // m_transform.getScaleVec().y();
    float x = 0.0f; // m_coordinates.x() * widgetSize.width();
    float y = 0.0f; // m_coordinates.y() * widgetSize.height();
    float startX = x;

    FontFace* face = getFontFace();
    face->loadBitmap(m_fontSize);
    float baseLineSpacing = face->getLineSpacing(m_fontSize) * yScale;

    // Get metrics for the size of the text
    int numLines = 1;
    float lineDistance;
    for (int j = 0; j < m_text.length(); j++) {
        QCharRef c = m_text[j];

        // Get the character corresponding to the letter
        unsigned long code = c.unicode();
        FontCharacter& ch = face->getCharacter(code, m_fontSize);

        // If x is too large, move to next line
        float advance = ch.m_advance >> 6;
        lineDistance = x + advance * xScale - startX;
        if (lineDistance > m_lineMaxWidth) {
            x = startX;
            y -= baseLineSpacing * m_lineSpacing;
            numLines += 1;
        }

        // Convert value of advance to pixels (2^6 = 64), and then scale
        x += (ch.m_advance >> 6) * xScale;
    }

    float textWidth = lineDistance;
    if (numLines > 1) {
        textWidth = 1.0;
    }
    float textHeight;
    if (m_text.length() == 1) {
        // Get more precise spacing for a single character (icon)
        QCharRef c = m_text[0];
        unsigned long code = c.unicode();
        FontCharacter& ch = face->getCharacter(code, m_fontSize);
        textHeight = ch.m_size.y() * yScale;
    }
    else {
        textHeight = baseLineSpacing * numLines * m_lineSpacing;;
    }
    return {numLines, textWidth, textHeight};
}

void Label::bindUniforms(ShaderProgram& shaderProgram)
{
    Glyph::bindUniforms(shaderProgram);
    RenderContext& context = m_canvas->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    // Set text color
    /// @todo Update color only when the label color changes
    Vector3 color = m_color.toVector<Real_t, 3>();
    UniformData& textColorUniformData = m_canvas->m_glyphUniforms.m_textColor[m_canvasIndices.m_textColor];
    textColorUniformData.setValue(color, uc);

    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_textColor, 
        textColorUniformData);

    // Set texture uniform
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_textureUniforms[(Int32_t)TextureUsageType::kDiffuse],
        m_canvas->m_glyphUniforms.m_textureUnits[(Int32_t)TextureUsageType::kDiffuse]);
	shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_textureUniforms[(Int32_t)TextureUsageType::kOpacity],
        m_canvas->m_glyphUniforms.m_textureUnits[(Int32_t)TextureUsageType::kOpacity]);
}

void Label::bindTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    GlFontFaceDispatcher& glFontFace = GlFontFaceDispatcher::Get(*getFontFace());
    if (!m_matHandle) {
        glFontFace.ensureFontSize(m_fontSize, shaderProgram->handle()->engine());
        glFontFace.bindTexture(m_fontSize);
    }
    else {
        // TODO: Allow material assignment to labels so this can be reached
        // I'm thinking just a button on the label/icon widget, use resource widget
        // to create material from existing textures
        m_matHandle->resourceAs<Material>()->bind(*shaderProgram, *context);
    }
}

void Label::releaseTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    G_UNUSED(shaderProgram);
    G_UNUSED(context);
    GlFontFaceDispatcher& glFontFace = GlFontFaceDispatcher::Get(*getFontFace());
    if (!m_matHandle) {
        glFontFace.releaseTexture(m_fontSize);
    }
    else {
        // TODO: Allow material assignment to labels so this can be reached
        m_matHandle->resourceAs<Material>()->release(*context);
    }
}

void Label::drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings)
{
    G_UNUSED(settings);
    G_UNUSED(shaderProgram);

    // Draw quads for text
    mesh()->vertexData().drawGeometry(m_renderSettings.shapeMode(), 1);
}


} // End namespaces

#include "GLabel.h"

#include "GFontManager.h"
#include "../GCoreEngine.h"
#include "../resource/GResource.h"
#include "../resource/GResourceCache.h"
#include "../rendering/geometry/GMesh.h"
#include "../rendering/geometry/GPolygon.h"
#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/GGLFunctions.h"
#include "../readers/GJsonReader.h"
#include "../components/GCanvasComponent.h"
#include "../rendering/renderer/GRenderSettings.h"
#include "../scene/GSceneObject.h"
#include "../scene/GScene.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
Label::Label(CanvasComponent* canvas):
    Label(canvas, kLabel, 40, "arial", "Default Text", ResourceBehaviorFlag::kRuntimeGenerated)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Label::Label(CanvasComponent * canvas, 
    const QString & text, const QString & font, float fontSize,
    ResourceBehaviorFlags rFlags):
    Label(canvas, kLabel, fontSize, font, text, rFlags)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
Label::~Label()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::setText(const QString & text)
{
    // Set text member
    m_text = text;
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::setFontSize(float pointSize)
{
    m_fontSize = pointSize;
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::setFontFace(const QString & faceName)
{
    m_fontFace = faceName;
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::setLineMaxSize(float width)
{
    m_lineMaxWidth = width;
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::setLineSpacing(float spacing)
{
    m_lineSpacing = spacing;
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::reload()
{
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Label::asJson(const SerializationContext& context) const
{
    QJsonObject object = Glyph::asJson(context).toObject();
    object.insert("text", m_text);
    object.insert("fontSize", m_fontSize);
    object.insert("font", m_fontFace);
    object.insert("color", m_color.toVector4g().asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    Glyph::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_text = object.value("text").toString("Default Text");
    m_fontSize = object.value("fontSize").toDouble(40);
    m_fontFace = object.value("font").toString("arial");

    if (object.contains("color")) {
        m_color = Color(Vector4(object.value("color")));
    }
    
    populateVertexData();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Label::Label(CanvasComponent * canvas,
    GlyphType type,
    float fontSize,
    const QString& fontFace,
    const QString& text,
    ResourceBehaviorFlags resourceFlags) :
    Glyph(canvas),
    //m_vertexData(std::make_shared<VertexArrayData>()),
    m_fontSize(fontSize),
    m_fontFace(fontFace),
    m_text(text),
    m_lineMaxWidth(1.0),
    m_lineSpacing(1.0)
{
    // Initialize an empty mesh 
    initializeEmptyMesh(*canvas->sceneObject()->scene()->engine()->resourceCache(), resourceFlags);

    // Initialize render settings to enable blending and disable face culling
    m_renderSettings.addDefaultBlend();
    m_renderSettings.addSetting<CullFaceSetting>(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
FontFace * Label::getFontFace()
{
    return FontManager::GetFontFace(m_fontFace);
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::bindUniforms(ShaderProgram& shaderProgram)
{
    Glyph::bindUniforms(shaderProgram);

    // Set text color
    Vector3 color = m_color.toVector3g();
    shaderProgram.setUniformValue("textColor", color);

    // Set texture uniform
    shaderProgram.setUniformValue("guiTexture", 0);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::bindTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);

    getFontFace()->ensureFontSize(m_fontSize, shaderProgram->handle()->engine());
    getFontFace()->bindTexture(m_fontSize);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::releaseTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);
    getFontFace()->releaseTexture(m_fontSize);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings)
{
    Q_UNUSED(settings)
    Q_UNUSED(shaderProgram)

    // Draw quads for text
    mesh()->vertexData().drawGeometry(m_renderSettings.shapeMode());
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

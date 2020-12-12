#include "GbLabel.h"

#include "../resource/GbResource.h"
#include "../rendering/geometry/GbMesh.h"
#include "../rendering/geometry/GbPolygon.h"
#include "GbFonts.h"
#include "../rendering/shaders/GbShaders.h"
#include "../rendering/GbGLFunctions.h"
#include "../readers/GbJsonReader.h"
#include "../components/GbCanvasComponent.h"
#include "../rendering/renderer/GbRenderSettings.h"
#include "../scene/GbSceneObject.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
Label::Label(CanvasComponent* canvas):
    Label(canvas, kLabel, 40, "arial", "Default Text")
{
    //QString folder("C:/Users/dante/Documents/Projects/grand-blue-engine/grand_blue/resources/images/bitmaps/fonts/");
    //getFontFace()->saveBitmap(m_fontSize,
    //    folder + "free-solid-900.png");
}
/////////////////////////////////////////////////////////////////////////////////////////////
Label::Label(CanvasComponent * canvas, 
    const QString & text, const QString & font, float fontSize,
    std::shared_ptr<SceneObject> target):
    Label(canvas, kLabel, fontSize, font, text)
{
    if(target)
        setTrackedObject(target);
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
    m_lineMaxSize = width;
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
QJsonValue Label::asJson() const
{
    QJsonObject object = Glyph::asJson().toObject();
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
    const QString& text) :
    Glyph(canvas, type),
    m_vertexData(std::make_shared<VertexArrayData>()),
    m_fontSize(fontSize),
    m_fontFace(fontFace),
    m_text(text),
    m_lineMaxSize(1.0),
    m_lineSpacing(1.0)
{
    // Initialize render settings to enable blending and disable face culling
    m_renderSettings.addDefaultBlend();
    m_renderSettings.addSetting<CullFaceSetting>(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
FontFace * Label::getFontFace()
{
    return FontManager::getFontFace(m_fontFace);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Label::populateVertexData()
{
    // Clear old attribute data
    m_vertexData->m_attributes.clear();
    m_vertexData->m_indices.clear();

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
        verticalAdjustment = (metrics.m_numLines - 1) * lineHeight;
        break;
    case kMiddle:
        verticalAdjustment = (metrics.m_numLines - 1) * lineHeight - 
            metrics.m_textHeight / 2.0;
        break;
    case kBottom:
        verticalAdjustment = (metrics.m_numLines - 1) * lineHeight - metrics.m_textHeight;
        break;
    }

    for (int j = 0; j < m_text.length(); j++) {
        QCharRef c = m_text[j];

        // Get the character corresponding to the letter
        unsigned long code = c.unicode();
        code = code;
        FontFace::Character& ch = face->getCharacter(code, m_fontSize);

        // If x is too large, move to next line
        float advance = ch.m_advance >> 6;
        float lineDistance = x + advance * xScale - startX;
        if (lineDistance > m_lineMaxSize) {
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
        QSize textureSize = face->getBitmapSize(m_fontSize);
        float uv_x_scale = glyphWidth / float(textureSize.width());
        float uv_y_scale = glyphHeight / float(textureSize.height());
        Vector2f uv_origin = { ch.m_origin.x() / float(textureSize.width()),
            (ch.m_origin.y() / float(textureSize.height())) };
        for (size_t i = 0; i < uvs.size(); i++) {
            float uv_x = uvs[i].x();
            float uv_y = uvs[i].y();
            uvs[i] = Vector2(uv_x * uv_x_scale + uv_origin.x(),
                uv_y * uv_y_scale + uv_origin.y());
        }

        // Convert value of advance to pixels (2^6 = 64), and then scale
        x += (ch.m_advance >> 6) * xScale;

        m_vertexData->m_attributes.m_vertices.insert(m_vertexData->m_attributes.m_vertices.end(),
            vertices.begin(),
            vertices.end());

        m_vertexData->m_attributes.m_texCoords.insert(m_vertexData->m_attributes.m_texCoords.end(),
            uvs.begin(),
            uvs.end());

        //vertices.clear();

        for (int i = 0; i < 6; i++) {
            m_vertexData->m_indices.push_back(count++);
        }
    }

    // Update the vertex array data
    m_vertexData->loadIntoVAO();
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
        FontFace::Character& ch = face->getCharacter(code, m_fontSize);

        // If x is too large, move to next line
        float advance = ch.m_advance >> 6;
        lineDistance = x + advance * xScale - startX;
        if (lineDistance > m_lineMaxSize) {
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
        FontFace::Character& ch = face->getCharacter(code, m_fontSize);
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
    m_vertexData->drawGeometry(m_renderSettings.shapeMode());
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

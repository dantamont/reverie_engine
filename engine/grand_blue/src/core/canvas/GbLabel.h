/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LABEL_H
#define GB_LABEL_H

// QT

// Internal
#include "GbGlyph.h"
#include "../containers/GbColor.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class FontFace;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Label class
class Label : public Glyph {

public:

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Label(CanvasComponent* canvas);
    Label(CanvasComponent* canvas, const QString& text, 
        const QString& font = "arial", float fontSize = 40, 
        std::shared_ptr<SceneObject> target = nullptr);
	~Label();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    float getFontSize() const { return m_fontSize; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    void setColor(const Color& color) { m_color = color; }

    /// @brief Set the label text
    void setText(const QString& text);

    void setFontSize(float pointSize);

    virtual void reload() override;

	/// @}

     //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:

    struct TextMetrics {
        int m_numLines;
        float m_textWidth;
        float m_textHeight;
    };

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    Label(CanvasComponent* canvas, 
        GlyphType type,
        float fontSize, 
        const QString& fontFace,
        const QString& text);

    FontFace* getFontFace();

    /// @brief Create actual text objects
    void populateVertexData();

    /// @brief Get metrics for the text
    TextMetrics getTextMetrics();

    /// @brief Set uniforms for the label
    virtual void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures() override;

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures() override;

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{
    
    std::shared_ptr<VertexArrayData> m_vertexData;

    /// @brief The text for this label
    QString m_text;

    /// @brief The font size for this label (in points)
    float m_fontSize;

    /// @brief The name of the font face used by this label
    QString m_fontFace;

    size_t m_numberOfLines;

    /// @brief Max length of the line in NDC space
    float m_lineMaxSize;
    float m_lineSpacing;

    Color m_color;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
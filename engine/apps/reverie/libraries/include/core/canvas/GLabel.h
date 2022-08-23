/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LABEL_H
#define GB_LABEL_H

// QT

// Internal
#include "GGlyph.h"
#include "fortress/containers/GColor.h"
#include "fortress/layer/framework/GSingleton.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class FontFace;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class GlFontFace
/// @brief Encapsulates a font-face to render a texture with OpenGL
class GlFontFaceDispatcher {
public:

    /// @brief Retrieve or create a font face dispatcher for the given font face
    static GlFontFaceDispatcher& Get(FontFace& fontFace);

    /// @brief Delete all stored GL font faces
    static void Clear();

    /// @brief Bind the font-face texture
    void bindTexture(float fontSize);

    /// @brief Release the font-face texture
    void releaseTexture(float fontSize);

    /// @brief Check if has a texture of the size, and load if it doesn't
    void ensureFontSize(float fontSize, CoreEngine* core);

protected:

    GlFontFaceDispatcher(FontFace& fontFace);

private:

    /// @brief Load bitmap into GL for rendering
    void loadGLTexture(CoreEngine* core, uint32_t fontPixelSize);

    /// @note Indexed by font pixel sizes
    tsl::robin_map<Uint32_t, std::shared_ptr<ResourceHandle>> m_textures; ///< The GL textures corresponding to the bitmaps for this font
    FontFace* m_fontFace{ nullptr }; ///< The font face being used for rendering

    static tsl::robin_map<Uint32_t, std::unique_ptr<GlFontFaceDispatcher>> s_dispatchers; ///< The global list of dispatchers
};

/// @brief Label class
class Label : public Glyph {
public:

    ~Label();

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    virtual GlyphType glyphType() const override { return GlyphType::kLabel; };

    Color& color() { return m_color; }

    /// @brief The font size for this label (in points)
    float getFontSize() const { return m_fontSize; }
    const QString& text() const { return m_text; }

    /// @brief the name of the font face used by this label
    const QString& getFontFaceName() const { return m_fontFace; }

    float lineMaxWidth() const { return m_lineMaxWidth; }
    float lineSpacing() const { return m_lineSpacing; }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    void setColor(const Color& color) { m_color = color; }

    /// @brief Set the label text
    void setText(const QString& text);

    void setFontSize(float pointSize);
    void setFontFace(const QString& faceName);

    void setLineMaxSize(float width);
    void setLineSpacing(float spacing);

    virtual void reload() override;

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

	/// @}

     //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Label& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Label& orObject);


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

    Label(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex);
    Label(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex,
        const QString& text,
        const QString& font = "arial", float fontSize = 40,
        ResourceBehaviorFlags rFlags = ResourceBehaviorFlag::kRuntimeGenerated);

    Label(CanvasComponent* canvas, 
        std::vector<Matrix4x4>& worldMatrixVec, 
        Uint32_t worldMatrixIndex,
        GlyphType type,
        float fontSize, 
        const QString& fontFace,
        const QString& text,
        ResourceBehaviorFlags resourceFlags);

    FontFace* getFontFace();

    /// @brief Create actual text objects
    void populateVertexData();

    /// @brief Get metrics for the text
    TextMetrics getTextMetrics();

    /// @brief Set uniforms for the label
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures(ShaderProgram* shaderProgram, RenderContext* context) override;

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures(ShaderProgram* shaderProgram, RenderContext* context) override;

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{
    
    /// @brief The text for this label
    QString m_text;

    /// @brief The font size for this label (in points)
    float m_fontSize;

    /// @brief The name of the font face used by this label
    QString m_fontFace;

    /// @brief Max length of the line in NDC space
    float m_lineMaxWidth;
    float m_lineSpacing;

    Color m_color;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
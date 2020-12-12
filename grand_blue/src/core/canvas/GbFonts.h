/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FONTS_H
#define GB_FONTS_H

// Standard
#include <memory>

// External
#include <ft2build.h>
#include FT_FREETYPE_H  

// Internal
#include "../GbManager.h"
#include "../mixins/GbLoadable.h"
#include "../resource/GbImage.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class TexturePacker;
class Texture;
template <typename D, size_t size> class Vector;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class FontFace
/// @brief Wrapper for a freetype font face
class FontFace: public Object, public Loadable{
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Encoding will determine which character codes to use to generate bitmaps
    enum FontEncoding {
        kASCII,
        kUnicode
    };

    /// @brief A character representing a a fontface glyph
    struct Character {
        unsigned long m_charCode;      // Character code representing the glyph
        unsigned int m_glyphIndex;     // Freetype ID of the glyph
        Vector<int, 2> m_size;      // Size of glyph (in pixels)
        Vector<int, 2> m_bearing;   // Offset from baseline to left/top of glyph
        unsigned int m_advance;        // Offset to advance to next glyph (in 1/64th pixels)
        Vector<int, 2> m_origin;    // Coordinates of the top-left of the character in it's bitmap
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    FontFace(CoreEngine* core = nullptr, bool isCore = false, FontEncoding encoding = kASCII);
    FontFace(CoreEngine* core, bool isCore, const QString& path, FontEncoding encoding = kASCII);
    ~FontFace();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Get a character for the font face given the specified character code
    Character& getCharacter(unsigned long charCode, float fontPointSize);

    /// @brief Get line spacing (in pixels) for the given font point size
    int getLineSpacing(float fontPointSize) const;

    /// @brief Bind the fontface texture
    void bindTexture(float fontSize);
    void releaseTexture(float fontSize);

    /// @brief Retrieve the bitmap for the specified font size
    QSize getBitmapSize(float height);

    /// @brief Generate a bitmap for the specified font point size
    void loadBitmap(float height);

    /// @brief Load glyphs for this font
    void loadGlyphs();

    /// @brief Save bitmap to a file
    bool saveBitmap(float fontSize, const QString& filepath);

    /// @}

    /// @brief the Wrapped freetype face
    FT_Face m_face = nullptr;

private:
    typedef tsl::robin_map<unsigned long, Character> CharacterMap;

    static int pointToPixelSize(float pointSize);
    static float pixelToPointSize(size_t pixelSize);

    /// @brief Clear the font face's bitmap data
    void clear();

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Load the font file
    void loadFont();

    /// @brief Load all glyphs for this font
    void loadAllGlyphs(size_t pixelSize);

    /// @brief Load all ascii glyphs
    void loadAsciiGlyphs(size_t pixelSize);

    /// @brief Load a glyph, given it's freetype glyph ID
    /// @details Load flags default too FT_LOAD_RENDER, which calls FT_Render_Glyph after the load
    void loadGlyph(unsigned int glyphIndex, FT_Int32 loadFlags = FT_LOAD_RENDER);

    /// @brief Load a character, given the character code
    void loadCharacter(unsigned long charCode, FT_Int32 loadFlags = FT_LOAD_RENDER);

    /// @brief Get the index of the specified character code
    int getIndex(unsigned long code);

    /// @brief Load bitmap into GL for rendering
    void loadGLTexture(size_t fontPixelSize);


    /// @brief Get pixel size
    int getPixelSize() const { return pointToPixelSize(m_fontSize); }
    /// @brief Set the pixel size, dynamically setting the width based on height
    void setPixelSize(size_t height);
    /// @brief Set the pixel size of the font
    void setPixelSize(size_t width, size_t height);

    /// @brief Get point size
    float getPointSize() const { return m_fontSize; }
    /// @brief Set the point size of the font face (a point is 1/72th of an inch)
    void setPointSize(float width, float height);
    /// @brief Set the point size, dynamically setting the width based on height
    void setPointSize(float height);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Whether or not this is loaded in as one of the core fonts
    bool m_isCore;

    CoreEngine* m_engine;

    /// @brief The encoding for the font
    FontEncoding m_encoding;

    /// @brief The currently set font size
    float m_fontSize = -1;

    /// @brief Texture packers for the font, indexed by font pixel sizes
    tsl::robin_map<size_t, std::shared_ptr<TexturePacker>> m_bitmaps;

    /// @brief Line heights (in pixels)
    tsl::robin_map<size_t, size_t> m_lineHeights;

    /// @brief Character set for the font, indexed by font pixel sizes
    tsl::robin_map<size_t, CharacterMap> m_characterMaps;

    /// @brief The GL textures corresponding to the bitmaps for this font
    tsl::robin_map<size_t, std::shared_ptr<ResourceHandle>> m_textures;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class FontManager
/// @brief Manages fonts
/// See: https://www.freetype.org/freetype2/docs/tutorial/step1.html
class FontManager : public Manager {
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static tsl::robin_map<QString, FontFace>& FontFaces() {
        return s_faces;
    }

    static const QJsonObject& FontAwesomeInfo() {
        return s_faInfo;
    }

    static QString faUnicodeCharacter(const QString& fontAwesomeIcon);

    /// @brief Retrieve a font face with a given file name
    static FontFace* getFontFace(const QString& name);

    /// @brief Load a font from a given file, with the specified encoding, and generates bitmap
    static void loadFontFace(CoreEngine* core, const QString& path, bool isCore, FontFace::FontEncoding encoding = FontFace::kASCII);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    FontManager(CoreEngine* engine);
	~FontManager();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    virtual void postConstruction() override;

    /// @brief Font awesome font families
    static QString regularFontAwesomeFamily();
    static QString brandFontAwesomeFamily();
    static QString solidFontAwesomeFamily();

	/// @}


protected:
    friend class Label;
    friend class FontFace;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    static void initializeFreeType();

    /// @brief Initialize fonts to use in GL
    void initializeGLFonts();

    /// @brief Initialize font awesome for application
    void initializeWidgetFonts();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The wrapped freetype library
    static std::unique_ptr<FT_Library> s_freeType;
    
    /// @brief Map of fonts wrapping "faces", i.e. fonts loaded via freetype
    static tsl::robin_map<QString, FontFace> s_faces;

    /// @brief Font Awesome font family IDs
    static int s_faBrands;
    static int s_faRegular;
    static int s_faSolid;

    /// @brief JSON document containing font-awesome icon metadata
    static QJsonObject s_faInfo;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
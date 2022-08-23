/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FONT_FACE_H
#define GB_FONT_FACE_H

// Standard
#include <memory>

// External
#include <ft2build.h>
#include FT_FREETYPE_H  

#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "fortress/image/GImage.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/image/GTexturePacker.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ResourceHandle;
//class TexturePacker;
class Texture;
template <typename D, size_t size> class Vector;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Encoding will determine which character codes to use to generate bitmaps
enum class FontEncoding {
    kASCII,
    kUnicode
};

/// @brief A character representing a a fontface glyph
struct FontCharacter {
    unsigned long m_charCode;      // Character code representing the glyph
    unsigned int m_glyphIndex;     // Freetype ID of the glyph
    Vector<int, 2> m_size;      // Size of glyph (in pixels)
    Vector<int, 2> m_bearing;   // Offset from baseline to left/top of glyph
    unsigned int m_advance;        // Offset to advance to next glyph (in 1/64th pixels)
    Vector<int, 2> m_origin;    // Coordinates of the top-left of the character in it's bitmap
};

/// @brief A struct for coupling pixel size with a texture packer
struct FontBitmap {
    FontBitmap();
    ~FontBitmap();

    uint32_t m_pixelSize;
    TexturePacker m_texPacker;
};

/// @brief A set of characters for a font face pixel size
struct CharacterSet {

    FontCharacter* character(unsigned long charCode) {
        return &m_characters[charCode];
    }

    uint32_t m_pixelSize;
    tsl::robin_map<unsigned long, FontCharacter> m_characters;
};


/// @class FontFace
/// @brief Wrapper for a freetype font face
class FontFace: public LoadableInterface, public NameableInterface{
public:

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Signal that a font face was cleared
    static Signal<Uint32_t> s_clearedFontFaceSignal;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    FontFace();
    FontFace(const GString& name, bool isCore, const GString& path, FontEncoding encoding = FontEncoding::kASCII);
    ~FontFace();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Return the bitmap with the given pixel size
    FontBitmap* bitmap(uint32_t fontPixelSize);

    /// @brief Return the character set with the given pixel size
    CharacterSet* characterSet(uint32_t fontPixelSize);

    /// @brief Get a character for the font face given the specified character code
    FontCharacter& getCharacter(unsigned long charCode, float fontPointSize);

    /// @brief Get line spacing (in pixels) for the given font point size
    int getLineSpacing(float fontPointSize) const;

    /// @brief Retrieve the bitmap for the specified font size
    const Vector<int, 2> getBitmapSize(float height);

    /// @brief Generate a bitmap for the specified font point size
    void loadBitmap(float height);

    /// @brief Load glyphs for this font
    void loadGlyphs();

    /// @brief Save bitmap to a file
    bool saveBitmap(float fontSize, const GString& filepath);

    /// @}

    /// @brief the Wrapped freetype face
    FT_Face m_face = nullptr;

private:
    friend class GlFontFaceDispatcher;

    static int pointToPixelSize(float pointSize);
    static float pixelToPointSize(uint32_t pixelSize);

    /// @brief Clear the font face's bitmap data
    void clear();

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Load the font file
    void loadFont();

    /// @brief Load all glyphs for this font
    void loadAllGlyphs(uint32_t pixelSize);

    /// @brief Load all ascii glyphs
    void loadAsciiGlyphs(uint32_t pixelSize);

    /// @brief Load a glyph, given it's freetype glyph ID, and pack into a bitmap
    /// @return Distance below origin in bitmap (pixels)
    //int packGlyph(unsigned int glyphIndex, unsigned int pixelSize, FontBitmap& bm, FT_Int32 loadFlags = FT_LOAD_RENDER);

    /// @brief Load a glyph, given it's freetype glyph ID
    /// @details Load flags default too FT_LOAD_RENDER, which calls FT_Render_Glyph after the load
    void ft_loadGlyph(unsigned int glyphIndex, FT_Int32 loadFlags = FT_LOAD_RENDER);

    /// @brief Load a character, given the character code
    void ft_loadCharacter(unsigned long charCode, FT_Int32 loadFlags = FT_LOAD_RENDER);

    /// @brief Get the index of the specified character code
    int getIndex(unsigned long code);

    /// @brief Get pixel size
    int getPixelSize() const { return pointToPixelSize(m_fontSize); }
    /// @brief Set the pixel size, dynamically setting the width based on height
    void setPixelSize(uint32_t height);
    /// @brief Set the pixel size of the font
    void setPixelSize(uint32_t width, uint32_t height);

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

    Uint32_t m_id; ///< The unique ID of the font face

    /// @brief Whether or not this is loaded in as one of the core fonts
    bool m_isCore;

    /// @brief The encoding for the font
    FontEncoding m_encoding;

    /// @brief The currently set font size
    float m_fontSize = -1;

    /// @brief Texture packers for the font
    /// @note Stored as shared_ptr because Manager is derived from QObject, which does NOT like unique pointers
    std::vector<std::shared_ptr<FontBitmap>> m_bitmaps;

    /// @brief Line heights (in pixels), mapped by pixel size
    tsl::robin_map<uint32_t, uint32_t> m_lineHeights;

    /// @brief Character set for the font
    std::vector<CharacterSet> m_characterSets;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
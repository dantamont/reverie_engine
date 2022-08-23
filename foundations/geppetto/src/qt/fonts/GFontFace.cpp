#include "geppetto/qt/fonts/GFontFace.h"
#include "geppetto/qt/fonts/GFontManager.h"
#include "fortress/types/GNameable.h"
#include "fortress/image/GTexturePacker.h"
#include "fortress/json/GJson.h"
#include "fortress/thread/GParallelLoop.h"
#include "fortress/containers/math/GVector.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "fortress/system/path/GFile.h"

#include <QScreen>


#define USE_THREADING false


namespace rev {



// FontBitmap

FontBitmap::FontBitmap()
{
}

FontBitmap::~FontBitmap()
{
}


// FontFace

Signal<Uint32_t> FontFace::s_clearedFontFaceSignal{};

FontFace::FontFace()
{
}

FontFace::FontFace(const GString& name, bool isCore, const GString & path, FontEncoding encoding):
    LoadableInterface(path),
    NameableInterface(name),
    m_encoding(encoding),
    m_isCore(isCore)
{
    // Set unique ID of the font face
    static Uint32_t faceCount = 0;
    m_id = faceCount;
    faceCount++;

    loadFont();
}

FontFace::~FontFace()
{
}

FontBitmap * FontFace::bitmap(uint32_t fontPixelSize)
{
    auto iter = std::find_if(m_bitmaps.begin(), m_bitmaps.end(),
        [&fontPixelSize](const std::shared_ptr<FontBitmap>& bm) {return bm->m_pixelSize == fontPixelSize; });

    if (iter == m_bitmaps.end()) {
        return nullptr;
    }
    else {
        return (*iter).get();
    }
}

CharacterSet * FontFace::characterSet(uint32_t fontPixelSize)
{
    auto iter = std::find_if(m_characterSets.begin(), m_characterSets.end(),
        [&fontPixelSize](const CharacterSet& cs) {return cs.m_pixelSize == fontPixelSize; });

    if (iter == m_characterSets.end()) {
        return nullptr;
    }
    else {
        return &(*iter);
    }
}

FontCharacter& FontFace::getCharacter(unsigned long charCode, float fontPointSize)
{
    uint32_t pixelSize = pointToPixelSize(fontPointSize);
    CharacterSet* cs = characterSet(pixelSize);
    if (!cs) {
        loadBitmap(pixelSize);
        cs = &m_characterSets.back();
    }

    assert(cs && "Error, failed to load bitmap for Font Face");

    FontCharacter* c = cs->character(charCode);
    assert(c && "Error, character not found in character map for font face");

    return *c;
}

int FontFace::getLineSpacing(float fontPointSize) const
{
    int pixelSize = pointToPixelSize(fontPointSize);
    if (m_lineHeights.find(pixelSize) == m_lineHeights.end()) {
        assert(false && "Error, pixel size for line heights not found");
    }
    return m_lineHeights.at(pixelSize);
}

const Vector2i FontFace::getBitmapSize(float height)
{
    uint32_t pixelSize = pointToPixelSize(height);
    return bitmap(pixelSize)->m_texPacker.getTextureSize();
}

void FontFace::loadBitmap(float height)
{
    uint32_t pixelSize = pointToPixelSize(height);
    FontBitmap* bm;
    if (bm = bitmap(pixelSize)) {
        return;
    }

    m_bitmaps.emplace_back(std::make_shared<FontBitmap>());
    m_bitmaps.back()->m_pixelSize = pixelSize;

    setPointSize(height);
    loadGlyphs();
}

void FontFace::setPixelSize(uint32_t width, uint32_t height)
{
    int error = FT_Set_Pixel_Sizes(
        m_face,     /* handle to face object */
        width,      /* pixel_width           */
        height);    /* pixel_height          */

    if (error) {
        assert(false && "Error setting pixel size");
    }

    m_fontSize = true;
}

void FontFace::setPixelSize(uint32_t height)
{
    setPixelSize(0, height);
}

void FontFace::setPointSize(float width, float height)
{
    int xRes = QGuiApplication::primaryScreen()->logicalDotsPerInchX();
    int yRes = QGuiApplication::primaryScreen()->logicalDotsPerInchY();
    int error = FT_Set_Char_Size(
        m_face,             /* handle to face object           */
        int(width * 64),    /* char_width in 1/64th of points  */
        int(height * 64),   /* char_height in 1/64th of points */
        xRes,               /* horizontal device resolution    */
        yRes);              /* vertical device resolution      */
    if (error) {
        assert(false && "Error setting point size");
    }
    m_fontSize = height;
}

void FontFace::setPointSize(float height)
{
    setPointSize(0, height);
}

void FontFace::loadGlyphs()
{
    if (m_fontSize < 0) assert(false && "Error, font size not set for font face");

    uint32_t pixelSize = getPixelSize();
    switch (m_encoding) {
    case FontEncoding::kASCII:
        // Load only ASCII Character set
        loadAsciiGlyphs(pixelSize);
        break;
    case FontEncoding::kUnicode:
        // Load all characters
        loadAllGlyphs(pixelSize);
        break;
    }

}

void FontFace::loadAllGlyphs(uint32_t pixelSize)
{
    // Prepare to iterate over all known characters
    FT_ULong  charcode;
    FT_UInt   gindex;
    charcode = FT_Get_First_Char(m_face, &gindex);
    unsigned int count = 0;

    // gindex will be zero once all the characters have been found
    unsigned long numGlyphs = m_face->num_glyphs;
    //int maxBelowOriginDistance = 0;
    FontBitmap& bm = *bitmap(pixelSize);
    //int distanceBelowOrigin;
    m_characterSets.emplace_back();
    m_characterSets.back().m_pixelSize = pixelSize;
    while (gindex != 0)
    {
        // Load glyph into glyph slot, and call FT_Render_Glyph to generate bitmap
        ft_loadGlyph(gindex, FT_LOAD_RENDER);

        // Pack glyph into bitmap
        Vector<int, 2> size(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
        Vector<int, 2> origin = bm.m_texPacker.packTexture(m_face->glyph->bitmap.buffer, size.x(), size.y()).m_origin;

        // Create character
        Vector<int, 2> bearing(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top);
        unsigned int advance = m_face->glyph->advance.x; // stored in 1/64ths of pixels
        m_characterSets.back().m_characters[charcode] = {
            charcode,
            gindex,
            size,
            bearing,
            advance,
            origin
        };

        //distanceBelowOrigin = packGlyph(gindex, pixelSize, bm, FT_LOAD_RENDER);

        //maxBelowOriginDistance = std::max(maxBelowOriginDistance, size.y() - bearing.y());

        if (count >= numGlyphs) {
            assert(false && "Error, iterated too many times while loading glyphs");
            break;
        }
        count++;

        // Hop to the next character
        charcode = FT_Get_Next_Char(m_face, charcode, &gindex);
    }

    // Was calculating manually, but free type stores this, which is less error-prone
    /// \see https://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype
    //m_lineHeights[pixelSize] = maxBelowOriginDistance + pixelSize;
    m_lineHeights[pixelSize] = m_face->size->metrics.height / 64.0; // Stored in 64ths of a pixel;
}

void FontFace::loadAsciiGlyphs(uint32_t pixelSize)
{   
    // Prepare to iterate over all known characters
    FT_UInt gindex;

    // Iterate over ASCII character set    
    //int maxBelowOriginDistance = 0;
    FontBitmap& bm = *bitmap(pixelSize);
    //int distanceBelowOrigin;
    m_characterSets.emplace_back();
    m_characterSets.back().m_pixelSize = pixelSize;
    for (unsigned long c = 0; c < 128; c++)
    {
        // Skip if no valid character found
        gindex = getIndex(c);
        if (!gindex) {
            continue;
        }

        // Load glyph into glyph slot, and call FT_Render_Glyph to generate bitmap
        ft_loadCharacter(c, FT_LOAD_RENDER);

        // Pack glyph into bitmap
        Vector<int, 2> size(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
        Vector<int, 2> origin = bm.m_texPacker.packTexture(m_face->glyph->bitmap.buffer, size.x(), size.y()).m_origin;

        // Create character
        Vector<int, 2> bearing(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top);
        unsigned int advance = m_face->glyph->advance.x;
        m_characterSets.back().m_characters[c] = {
            c,
            gindex,
            size,
            bearing,
            advance,
            origin
        };
        //distanceBelowOrigin = packGlyph(gindex, pixelSize, bm, FT_LOAD_RENDER);

        //maxBelowOriginDistance = std::max(maxBelowOriginDistance, size.y() - bearing.y());
    }

    // Was calculating manually, but free type stores this, which is less error-prone
    /// \see https://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype
    //m_lineHeights[pixelSize] = maxBelowOriginDistance + pixelSize;
    m_lineHeights[pixelSize] = m_face->size->metrics.height / 64.0; // Stored in 64ths of a pixel
}

bool FontFace::saveBitmap(float fontSize, const GString & filepath)
{
    uint32_t pixelSize = pointToPixelSize(fontSize);
    FontBitmap* bm = bitmap(pixelSize);
    if (!bm) {
        loadBitmap(fontSize);
        bm = m_bitmaps.back().get();
    }

    // The bitmap generated from the glyph is a grayscale 8-bit image 
    // where each color is represented by a single byte
    Image image = bm->m_texPacker.getImage(Image::ColorFormat::kGrayscale8);
    return image.save(filepath);
}

int FontFace::pointToPixelSize(float pointSize)
{
    // Points are a physical unit, where 1 point equals 1/72th of an inch in digital typography
    // Resolution is in DPI
    float resolution = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    int pixelSize = int(pointSize * resolution / 72.0);
    return pixelSize;
}

float FontFace::pixelToPointSize(uint32_t pixelSize)
{
    float resolution = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    float pointSize = pixelSize * 72.0 / resolution;
    return pointSize;
}

void FontFace::clear()
{
    m_characterSets.clear();
    m_bitmaps.clear();

    // Emit signal that font face was cleared
    s_clearedFontFaceSignal.emitForAll(m_id);
}

void FontFace::loadFont()
{
    // Initialize freetype if not yet initialized
    if (!FontManager::s_freeType){
        FontManager::initializeFreeType();
    }

    // Check that file exists
#ifdef DEBUG_MODE
    assert(QFile::exists(m_path.c_str()) && "Error, file does not exist");
#endif

    // Load font face from file
    if (FT_New_Face(*FontManager::s_freeType, m_path.c_str(), 0, &m_face)) {
        assert(false && "ERROR::FREETYPE: Failed to load font");
    }
}

void FontFace::ft_loadGlyph(unsigned int glyphIndex, FT_Int32 loadFlags)
{
    int error = FT_Load_Glyph(
        m_face,          /* handle to face object */
        glyphIndex,      /* glyph index           */
        loadFlags);      /* load flags, see below */
    if (error) {
        assert(false && "Error loading glyph");
    }

}

void FontFace::ft_loadCharacter(unsigned long charCode, FT_Int32 loadFlags)
{
    FT_Error error = FT_Load_Char(
        m_face,          /* handle to face object */
        charCode,        /* character code        */
        loadFlags);      /* load flags, see below */
    if (error) {
        assert(false && "Error loading character");
    }
}

int FontFace::getIndex(unsigned long code)
{
    int index = FT_Get_Char_Index(m_face, code);
    return index;
}


} // End namespaces

#include "GbFonts.h"
#include "../GbCoreEngine.h"
#include "../utils/GbTexturePacker.h"
#include "../readers/GbJsonReader.h"
#include "../rendering/materials/GbMaterial.h"
#include "../rendering/GbGLFunctions.h"
#include "../components/GbCanvasComponent.h"
#include "../utils/GbParallelization.h"
#include "../geometry/GbVector.h"
#include "../../view/style/GbFontIcon.h"
#include "../rendering/lighting/GbShadowMap.h"

#define USE_THREADING false

namespace Gb {
//////////////////////////////////////////////////////////////////////////////////
FontFace::FontFace(CoreEngine* engine, bool isCore, FontEncoding encoding) :
    m_encoding(encoding),
    m_engine(engine),
    m_isCore(isCore)
{
}
//////////////////////////////////////////////////////////////////////////////////
FontFace::FontFace(CoreEngine* engine, bool isCore, const QString & path, FontEncoding encoding):
    Loadable(path),
    m_encoding(encoding),
    m_engine(engine),
    m_isCore(isCore)
{
    loadFont();
}
//////////////////////////////////////////////////////////////////////////////////
FontFace::~FontFace()
{
}
//////////////////////////////////////////////////////////////////////////////////
FontFace::Character& FontFace::getCharacter(unsigned long charCode, float fontPointSize)
{
    size_t pixelSize = pointToPixelSize(fontPointSize);
    if (!Map::HasKey(m_characterMaps, pixelSize)) {
        loadBitmap(fontPointSize);
    }

    if (!Map::HasKey(m_characterMaps, pixelSize)) {
        throw("Error, failed to load bitmap for Font Face");
    }

    if (!Map::HasKey(m_characterMaps[pixelSize], charCode)) {
        throw("Error, character not found in character map for font face");
    }

    return m_characterMaps[pixelSize][charCode];
}
//////////////////////////////////////////////////////////////////////////////////
int FontFace::getLineSpacing(float fontPointSize) const
{
    int pixelSize = pointToPixelSize(fontPointSize);
    if (m_lineHeights.find(pixelSize) == m_lineHeights.end()) {
        throw("Error, pixel size for line heights not found");
    }
    return m_lineHeights.at(pixelSize);
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::bindTexture(float fontSize)
{
    size_t pixelSize = pointToPixelSize(fontSize);
    if (!Map::HasKey(m_textures, pixelSize)) {
        loadGLTexture(pixelSize);
    }
    auto texture = m_textures.at(pixelSize)->resourceAs<Texture>();
    //int h = texture->height();
    //int w = texture->width();

    if (texture->handle()->isConstructed()) {
        // bind texture to first unreserved unit
        //texture->bind(NUM_SHADOW_MAP_TEXTURES);
        texture->bind(0);
    }
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::releaseTexture(float fontSize)
{
    size_t pixelSize = pointToPixelSize(fontSize);
    if (m_textures.at(pixelSize)->isConstructed()) {
        m_textures.at(pixelSize)->resourceAs<Texture>()->release(); // bind texture to this unit
    }
}
//////////////////////////////////////////////////////////////////////////////////
QSize FontFace::getBitmapSize(float height)
{
    size_t pixelSize = pointToPixelSize(height);
    Vector<int, 2> size = m_bitmaps[pixelSize]->getTextureSize();
    return QSize(size.x(), size.y());
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadBitmap(float height)
{
    size_t pixelSize = pointToPixelSize(height);
    if (Map::HasKey(m_bitmaps, pixelSize)) return;
    m_bitmaps[pixelSize] = std::make_shared<TexturePacker>();

    setPointSize(height);
    loadGlyphs();
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::setPixelSize(size_t width, size_t height)
{
    int error = FT_Set_Pixel_Sizes(
        m_face,     /* handle to face object */
        width,      /* pixel_width           */
        height);    /* pixel_height          */

    if (error) {
        throw("Error setting pixel size");
    }

    m_fontSize = true;
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::setPixelSize(size_t height)
{
    setPixelSize(0, height);
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::setPointSize(float width, float height)
{
    int xRes = Renderable::screenDPIX();
    int yRes = Renderable::screenDPIY();
    int error = FT_Set_Char_Size(
        m_face,             /* handle to face object           */
        int(width * 64),    /* char_width in 1/64th of points  */
        int(height * 64),   /* char_height in 1/64th of points */
        xRes,               /* horizontal device resolution    */
        yRes);              /* vertical device resolution      */
    if (error) {
        throw("Error setting point size");
    }
    m_fontSize = height;
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::setPointSize(float height)
{
    setPointSize(0, height);
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadGlyphs()
{
    if (m_fontSize < 0) throw("Error, font size not set for font face");

    size_t pixelSize = getPixelSize();
    switch (m_encoding) {
    case kASCII:
        // Load only ASCII Character set
        loadAsciiGlyphs(pixelSize);
        break;
    case kUnicode:
        // Load all characters
        loadAllGlyphs(pixelSize);
        break;
    }

}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadAllGlyphs(size_t pixelSize)
{
    // Prepare to iterate over all known characters
    FT_ULong  charcode;
    FT_UInt   gindex;
    charcode = FT_Get_First_Char(m_face, &gindex);
    unsigned int count = 0;

    // gindex will be zero once all the characters have been found
    unsigned long numGlyphs = m_face->num_glyphs;
    int maxBelowOriginDistance = 0;
    while (gindex != 0)
    {
        // Load glyph into glyph slot, and call FT_Render_Glyph to generate bitmap
        loadGlyph(gindex, FT_LOAD_RENDER);

        // Pack glyph into bitmap
        Vector<int, 2> size(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
        Vector<int, 2> origin = m_bitmaps[pixelSize]->packTexture(m_face->glyph->bitmap.buffer, size.x(), size.y());

        // Create character
        Vector<int, 2> bearing(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top);
        unsigned int advance = m_face->glyph->advance.x; // stored in 1/64ths of pixels
        m_characterMaps[pixelSize][charcode] = {
            charcode,
            gindex,
            size,
            bearing,
            advance,
            origin
        };

        // Hop to the next character
        charcode = FT_Get_Next_Char(m_face, charcode, &gindex);

        if (count >= numGlyphs) {
            throw("Error, iterated too many times while loading glyphs");
            break;
        }
        count++;

        maxBelowOriginDistance = std::max(maxBelowOriginDistance, size.y() - bearing.y());
    }

    m_lineHeights[pixelSize] = maxBelowOriginDistance + pixelSize;

}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadAsciiGlyphs(size_t pixelSize)
{   
    // Prepare to iterate over all known characters
    FT_UInt gindex;

    // Iterate over ASCII character set    
    int maxBelowOriginDistance = 0;
    for (unsigned long c = 0; c < 128; c++)
    {
        // Skip if no valid character found
        gindex = getIndex(c);
        if (!gindex) continue;

        // Load glyph into glyph slot, and call FT_Render_Glyph to generate bitmap
        loadCharacter(c, FT_LOAD_RENDER);

        // Pack glyph into bitmap
        Vector<int, 2> size(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
        Vector<int, 2> origin = m_bitmaps[pixelSize]->packTexture(m_face->glyph->bitmap.buffer, size.x(), size.y());

        // Create character
        Vector<int, 2> bearing(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top);
        unsigned int advance = m_face->glyph->advance.x;
        m_characterMaps[pixelSize][c] = {
            c,
            gindex,
            size,
            bearing,
            advance,
            origin
        };

        maxBelowOriginDistance = std::max(maxBelowOriginDistance, size.y() - bearing.y());
    }

    m_lineHeights[pixelSize] = maxBelowOriginDistance + pixelSize;

}
//////////////////////////////////////////////////////////////////////////////////
bool FontFace::saveBitmap(float fontSize, const QString & filepath)
{
    size_t pixelSize = pointToPixelSize(fontSize);
    if (!Map::HasKey(m_bitmaps, pixelSize)) {
        loadBitmap(fontSize);
    }

    // The bitmap generated from the glyph is a grayscale 8-bit image 
    // where each color is represented by a single byte
    Image image = m_bitmaps[pixelSize]->getImage(QImage::Format::Format_Grayscale8);
    return image.save(filepath);
}
//////////////////////////////////////////////////////////////////////////////////
int FontFace::pointToPixelSize(float pointSize)
{
    // Points are a physical unit, where 1 point equals 1/72th of an inch in digital typography
    // Resolution is in DPI
    float resolution = Renderable::screenDPI();
    int pixelSize = int(pointSize * resolution / 72.0);
    return pixelSize;
}
//////////////////////////////////////////////////////////////////////////////////
float FontFace::pixelToPointSize(size_t pixelSize)
{
    float resolution = Renderable::screenDPI();
    float pointSize = pixelSize * 72.0 / resolution;
    return pointSize;
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::clear()
{
    m_characterMaps.clear();
    m_bitmaps.clear();
    m_textures.clear();
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadFont()
{
    // Initialize freetype if not yet initialized
    if (!FontManager::s_freeType) FontManager::initializeFreeType();

    // Check that file exists
    if (!QFile::exists(m_path)) {
        logError("Error, " + m_path + " does not exist");
#ifdef DEBUG_MODE
        throw("Error, file does not exist");
#endif
    }

    // Load font face from file
    if (FT_New_Face(*FontManager::s_freeType, m_path.c_str(), 0, &m_face))
        throw("ERROR::FREETYPE: Failed to load font");
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadGlyph(unsigned int glyphIndex, FT_Int32 loadFlags)
{
    int error = FT_Load_Glyph(
        m_face,          /* handle to face object */
        glyphIndex,      /* glyph index           */
        loadFlags);      /* load flags, see below */
    if (error) {
        throw("Error loading glyph");
    }

}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadCharacter(unsigned long charCode, FT_Int32 loadFlags)
{
    int error = FT_Load_Char(
        m_face,          /* handle to face object */
        charCode,        /* character code        */
        loadFlags);      /* load flags, see below */
    if (error) {
        throw("Error loading character");
    }
}
//////////////////////////////////////////////////////////////////////////////////
int FontFace::getIndex(unsigned long code)
{
    int index = FT_Get_Char_Index(m_face, code);
    return index;
}
//////////////////////////////////////////////////////////////////////////////////
void FontFace::loadGLTexture(size_t fontPixelSize)
{
    // Ensure that there is a valid bitmap image
    if (!Map::HasKey(m_bitmaps, fontPixelSize)) {
        loadBitmap(pixelToPointSize(fontPixelSize));
    }
    Image bitmap = m_bitmaps[fontPixelSize]->getImage(QImage::Format::Format_Grayscale8);
#ifdef DEBUG_MODE
    if (bitmap.isNull()) {
        throw("Error, bitmap is null");
    }
#endif

    // Create unique name for texture
    GString uniqueName = "font_" + GString(FileReader::PathToName(m_path, false)) + "_" + m_uuid.asString() + "_" + GString::FromNumber(fontPixelSize);

    // Create OpenGL texture
    m_textures[fontPixelSize] = Texture::createHandle(m_engine,
        bitmap, 
        TextureUsageType::kDiffuse,
        TextureFilter::kLinear,
        TextureFilter::kLinear,
        TextureWrapMode::kClampToEdge);
    m_textures[fontPixelSize]->setName(uniqueName);
    //m_textures[fontPixelSize]->postConstruct();

    // If this is a core font-face, preserve textures
    m_textures[fontPixelSize]->setCore(m_isCore);
}




//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
QString FontManager::faUnicodeCharacter(const QString& fontAwesomeIcon)
{
    if (!s_faInfo.contains(fontAwesomeIcon)) {
        qDebug() << "Error, font awesome icon not found";
#ifdef DEBUG_MODE
        throw("Error, font awesome icon not found");
#endif
    }
    QString unicode = s_faInfo.value(fontAwesomeIcon).toObject().value("unicode").toString();
    return unicode;
}
//////////////////////////////////////////////////////////////////////////////////
FontManager::FontManager(CoreEngine* engine):
    Manager(engine, "FontManager")
{
    initializeFreeType();
    initializeWidgetFonts();

    // Load font-awesome icon metadata
    QString infoPath = QDir::currentPath() + "/resources/fonts/fontawesome/unicode_info.json";
    s_faInfo = JsonReader(infoPath).getContentsAsJsonObject();
}
//////////////////////////////////////////////////////////////////////////////////
FontManager::~FontManager()
{
    // Clear faces
    for (const std::pair<QString, FontFace>& facePair : s_faces) {
        FT_Done_Face(facePair.second.m_face);
    }

    // Clear resources
    FT_Done_FreeType(*s_freeType);
}
//////////////////////////////////////////////////////////////////////////////////
FontFace* FontManager::getFontFace(const QString & name)
{
    if (!Map::HasKey(s_faces, name)) {
#ifdef DEBUG_MODE
        throw("Error, no face found with the name " + name);
#endif
        return nullptr;
    }
    else {
        return &s_faces[name];
    }
}
//////////////////////////////////////////////////////////////////////////////////
void FontManager::loadFontFace(CoreEngine* engine, const QString & path, bool isCore, FontFace::FontEncoding encoding)
{
    QString fontName = FileReader::PathToName(path, false);

    // Return if font already loaded
    if (Map::HasKey(s_faces, fontName)) return;

    Map::Emplace(s_faces, fontName, engine, isCore, path, encoding);
}
//////////////////////////////////////////////////////////////////////////////////
void FontManager::postConstruction()
{
    initializeGLFonts();
    Manager::postConstruction();
}
//////////////////////////////////////////////////////////////////////////////////
QString FontManager::regularFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faRegular);
    return families[0];
}
//////////////////////////////////////////////////////////////////////////////////
QString FontManager::brandFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faBrands);
    return families[0];
}
//////////////////////////////////////////////////////////////////////////////////
QString FontManager::solidFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faSolid);
    return families[0];
}
//////////////////////////////////////////////////////////////////////////////////
void FontManager::initializeFreeType()
{
    if (!s_freeType) {
        s_freeType = std::make_unique<FT_Library>();
        if (FT_Init_FreeType(s_freeType.get())) {
            throw("ERROR::FREETYPE: Could not init FreeType Library");
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////
void FontManager::initializeGLFonts()
{
    QString currentPath = QDir::currentPath();

    // Load font awesome
    QString freeSolidPath = currentPath + "/resources/fonts/fontawesome/otfs/free-solid-900.otf";
    FontManager::loadFontFace(m_engine, freeSolidPath, true, FontFace::kUnicode);

    // Load windows fonts
    ParallelLoopGenerator loop(&CoreEngine::HELPER_THREADPOOL, USE_THREADING);
    QString windowsFontPath = currentPath + "/resources/fonts/windows_fonts/";
    QDir windowsFontDir(windowsFontPath);
    QStringList fontPaths = windowsFontDir.entryList(QStringList() << "*.ttf" << "*.TTF", QDir::Files);
    loop.parallelFor(fontPaths.size(), [&](int start, int end) {
        for (int i = start; i < end; i++) {
            const QString& fontFile = fontPaths[i];
            QString fontPath = windowsFontPath + fontFile;
            FontManager::loadFontFace(m_engine, fontPath, true, FontFace::kASCII);

            // Add font for qt to use
            //FontIcon::addFont(fontPath);
        }
    });

}
//////////////////////////////////////////////////////////////////////////////////
void FontManager::initializeWidgetFonts()
{
    // Load fonts
    if (s_faBrands < 0) {
        s_faBrands = FontIcon::addFont(":/fonts/brands-regular-400.otf");
        if (s_faBrands < 0) {
            logError("FontAwesome cannot be loaded!");
        }
    }
    if (s_faRegular < 0) {
        s_faRegular = FontIcon::addFont(":/fonts/free-regular-400.otf");
        if (s_faRegular < 0) {
            logError("FontAwesome cannot be loaded!");
        }
    }
    if (s_faSolid < 0) {
        s_faSolid = FontIcon::addFont(":/fonts/free-solid-900.otf");
        if (s_faSolid < 0) {
            logError("FontAwesome cannot be loaded!");
        }
    }

}
//////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<FT_Library>  FontManager::s_freeType = nullptr;

//////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<QString, FontFace> FontManager::s_faces = {};

//////////////////////////////////////////////////////////////////////////////////
int FontManager::s_faBrands = -1;

//////////////////////////////////////////////////////////////////////////////////
int FontManager::s_faRegular = -1;

//////////////////////////////////////////////////////////////////////////////////
int FontManager::s_faSolid = -1;
//////////////////////////////////////////////////////////////////////////////////
QJsonObject FontManager::s_faInfo = QJsonObject();

//////////////////////////////////////////////////////////////////////////////////
} // End namespaces

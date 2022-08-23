#include "geppetto/qt/fonts/GFontManager.h"

#include <QFontDatabase>

#include "fortress/containers/math/GVector.h"
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"

#include "fortress/image/GTexturePacker.h"
#include "geppetto/qt/style/GFontIcon.h"


namespace rev {

FontFace* FontManager::GetFontFace(const GString & fontName)
{
    auto iter = std::find_if(s_faces.begin(), s_faces.end(),
        [&](const FontFace& f) {return f.getName() == fontName; });
    if (iter == s_faces.end()) {
        return nullptr;
    }
    else {
        return &(*iter);
    }
}

GStringUtf8 FontManager::FaUnicodeCharacter(const GString& fontAwesomeIcon)
{
    assert(s_faInfo.contains(fontAwesomeIcon.c_str()) && "Error, font awesome icon not found");

    const json& unicodeJson = s_faInfo.at(fontAwesomeIcon.c_str()).at("unicode");
    return GStringUtf8(unicodeJson.get_ref<const std::string&>().c_str());
}

FontManager::FontManager(const GString& resourcesPath):
    ManagerInterface("FontManager")
{
    initializeFreeType();
    initializeWidgetFonts();

    // Load font-awesome icon metadata
    m_resourcesPath = resourcesPath;
    QString infoPath = m_resourcesPath + "fonts/fontawesome/unicode_info.json";
    GJson::FromFile(infoPath.toStdString().c_str(), s_faInfo);
}

FontManager::~FontManager()
{
    // Clear faces
    for (const FontFace& facePair : s_faces) {
        FT_Done_Face(facePair.m_face);
    }
    s_faces.clear();

    // Clear resources
    FT_Done_FreeType(*s_freeType);
}

void FontManager::LoadFontFace(const GString & path, bool isCore, FontEncoding encoding)
{
    GFile myFile(path);
    GString fontName = myFile.getFileName(false);

    // Return if font already loaded
    if (GetFontFace(fontName)) return;

    s_faces.emplace_back(fontName, isCore, path, encoding);
}

void FontManager::postConstruction()
{
    initializeGLFonts();
    ManagerInterface::postConstruction();
}

QString FontManager::RegularFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faRegular);
    return families[0];
}

QString FontManager::BrandFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faBrands);
    return families[0];
}

QString FontManager::SolidFontAwesomeFamily()
{
    QStringList families = QFontDatabase::applicationFontFamilies(s_faSolid);
    return families[0];
}

void FontManager::initializeFreeType()
{
    if (!s_freeType) {
        s_freeType = std::make_unique<FT_Library>();
        assert(!FT_Init_FreeType(s_freeType.get()) && "ERROR::FREETYPE: Could not init FreeType Library");
    }
}

void FontManager::initializeGLFonts()
{
    // Load font awesome
    GString freeSolidPath = m_resourcesPath + "fonts/fontawesome/otfs/free-solid-900.otf";
    FontManager::LoadFontFace(freeSolidPath, true, FontEncoding::kUnicode);

    // Load windows fonts
    // TODO: Check what needs to be mutex locked and turn threading on
    QString windowsFontPath = m_resourcesPath + "fonts/windows_fonts/";

    /// @todo Replace with a GDir function for this
    QDir windowsFontDir(windowsFontPath);
    QStringList fontPaths = windowsFontDir.entryList(QStringList() << "*.ttf" << "*.TTF", QDir::Files);
    for (int i = 0; i < fontPaths.size(); i++) {
        const QString& fontFile = fontPaths[i];
        QString fontPath = windowsFontPath + fontFile;
        FontManager::LoadFontFace(fontPath.toStdString().c_str(), true, FontEncoding::kASCII);

        // Add font for qt to use
        //FontIcon::addFont(fontPath);
    }
}

void FontManager::initializeWidgetFonts()
{
    // Load fonts
    if (s_faBrands < 0) {
        s_faBrands = FontIcon::addFont(":/fonts/brands-regular-400.otf");
        assert(s_faBrands >= 0 && "FontAwesome cannot be loaded!");
    }
    if (s_faRegular < 0) {
        s_faRegular = FontIcon::addFont(":/fonts/free-regular-400.otf");
        assert(s_faRegular >= 0 && "FontAwesome cannot be loaded!");

    }
    if (s_faSolid < 0) {
        s_faSolid = FontIcon::addFont(":/fonts/free-solid-900.otf");
        assert(s_faSolid >= 0 && "FontAwesome cannot be loaded!");
    }

}

std::unique_ptr<FT_Library>  FontManager::s_freeType = nullptr;
std::vector<FontFace> FontManager::s_faces = {};
int FontManager::s_faBrands = -1;
int FontManager::s_faRegular = -1;
int FontManager::s_faSolid = -1;
json FontManager::s_faInfo = json::object();




} // End namespaces

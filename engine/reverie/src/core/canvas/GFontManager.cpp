#include "GFontManager.h"
#include "../GCoreEngine.h"
#include "../utils/GTexturePacker.h"
#include "../readers/GJsonReader.h"
#include "../rendering/materials/GMaterial.h"
#include "../rendering/GGLFunctions.h"
#include "../components/GCanvasComponent.h"
#include "../threading/GParallelLoop.h"
#include "../geometry/GVector.h"
#include "../../view/style/GFontIcon.h"
#include "../rendering/lighting/GShadowMap.h"

#define USE_THREADING false

//////////////////////////////////////////////////////////////////////////////////
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

QString FontManager::FaUnicodeCharacter(const QString& fontAwesomeIcon)
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

FontManager::FontManager(CoreEngine* engine):
    Manager(engine, "FontManager")
{
    initializeFreeType();
    initializeWidgetFonts();

    // Load font-awesome icon metadata
    QString infoPath = QDir::currentPath() + "/resources/fonts/fontawesome/unicode_info.json";
    s_faInfo = JsonReader(infoPath).getContentsAsJsonObject();
}

FontManager::~FontManager()
{
    // Clear faces
    for (const FontFace& facePair : s_faces) {
        FT_Done_Face(facePair.m_face);
    }

    // Clear resources
    FT_Done_FreeType(*s_freeType);
}

void FontManager::LoadFontFace(const QString & path, bool isCore, FontEncoding encoding)
{
    GString fontName = (GString)FileReader::PathToName(path, false);

    // Return if font already loaded
    if (GetFontFace(fontName)) return;

    s_faces.emplace_back(fontName, isCore, path, encoding);
}

void FontManager::postConstruction()
{
    initializeGLFonts();
    Manager::postConstruction();
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
        if (FT_Init_FreeType(s_freeType.get())) {
            throw("ERROR::FREETYPE: Could not init FreeType Library");
        }
    }
}

void FontManager::initializeGLFonts()
{
    QString currentPath = QDir::currentPath();

    // Load font awesome
    QString freeSolidPath = currentPath + "/resources/fonts/fontawesome/otfs/free-solid-900.otf";
    FontManager::LoadFontFace(freeSolidPath, true, FontEncoding::kUnicode);

    // Load windows fonts
    // TODO: Check what needs to be mutex locked and turn threading on
    ParallelLoopGenerator loop(&CoreEngine::HELPER_THREADPOOL, USE_THREADING);
    QString windowsFontPath = currentPath + "/resources/fonts/windows_fonts/";
    QDir windowsFontDir(windowsFontPath);
    QStringList fontPaths = windowsFontDir.entryList(QStringList() << "*.ttf" << "*.TTF", QDir::Files);
    loop.parallelFor(fontPaths.size(), [&](int start, int end) {
        for (int i = start; i < end; i++) {
            const QString& fontFile = fontPaths[i];
            QString fontPath = windowsFontPath + fontFile;
            FontManager::LoadFontFace(fontPath, true, FontEncoding::kASCII);

            // Add font for qt to use
            //FontIcon::addFont(fontPath);
        }
    });

}

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

std::unique_ptr<FT_Library>  FontManager::s_freeType = nullptr;
std::vector<FontFace> FontManager::s_faces = {};
int FontManager::s_faBrands = -1;
int FontManager::s_faRegular = -1;
int FontManager::s_faSolid = -1;
QJsonObject FontManager::s_faInfo = QJsonObject();



//////////////////////////////////////////////////////////////////////////////////
} // End namespaces

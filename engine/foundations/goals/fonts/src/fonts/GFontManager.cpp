#include "fonts/GFontManager.h"


#include "fortress/containers/math/GVector.h"
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"

#include "fortress/image/GTexturePacker.h"
#include "logging/GLogger.h"


namespace rev {

FontFace* FontManager::GetFontFace(const GString & fontName)
{
    auto iter = std::find_if(s_faces.begin(), s_faces.end(),
        [&](const FontFace& f) {
            return f.getName() == fontName;
        }
    );

    if (iter == s_faces.end()) {
        return nullptr;
    }
    else {
        return &(*iter);
    }
}

FontManager::FontManager(const GString& resourcesPath):
    ManagerInterface("FontManager")
{
    initializeFreeType();
    m_resourcesPath = resourcesPath;
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
    if (GetFontFace(fontName)) {
        return;
    }

    s_faces.emplace_back(fontName, isCore, path, encoding);
}

void FontManager::postConstruction()
{
    initializeGLFonts();
    ManagerInterface::postConstruction();
}

void FontManager::initializeFreeType()
{
    if (!s_freeType) {
        s_freeType = std::make_unique<FT_Library>();
        if (FT_Init_FreeType(s_freeType.get())) {
            Logger::Throw("ERROR::FREETYPE: Could not init FreeType Library");
        }
    }
}

void FontManager::initializeGLFonts()
{
    // Load font awesome
    GString freeSolidPath = m_resourcesPath + "fonts/fontawesome/otfs/free-solid-900.otf";
    FontManager::LoadFontFace(freeSolidPath, true, FontEncoding::kUnicode);

    // Load windows fonts
    // TODO: Check what needs to be mutex locked and turn threading on
    GString windowsFontPath = m_resourcesPath + "fonts/windows_fonts/";

    /// @todo Replace with a GDir function for this
    GDir windowsFontDir(windowsFontPath);
    std::vector<GString>  fontPaths = windowsFontDir.getFiles(
        [](const GString& fileName) {
            return fileName.asLower().contains(".ttf");
        });
    for (int i = 0; i < fontPaths.size(); i++) {
        const GString& fontFile = fontPaths[i];
        GString fontPath = windowsFontPath + fontFile;
        FontManager::LoadFontFace(fontPath.c_str(), true, FontEncoding::kASCII);

        // Add font for qt to use
        //FontIcon::addFont(fontPath);
    }
}


std::unique_ptr<FT_Library>  FontManager::s_freeType = nullptr;
std::vector<FontFace> FontManager::s_faces = {};




} // End namespaces

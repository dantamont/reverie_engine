#pragma once

// Third-party
#include <QString>
#include <vector>

// External
#include "fortress/encoding/string/GEncodedString.h"
#include "fortress/json/GJson.h"
#include "fortress/layer/application/GManagerInterface.h"

// Internal
#include "geppetto/qt/fonts/GFontFace.h"

namespace rev {

/// @class FontManager
/// @brief Manages fonts
/// @todo Make this a singleton
/// See: https://www.freetype.org/freetype2/docs/tutorial/step1.html
class FontManager : public ManagerInterface {
public:
    /// @name Static
    /// @{

    static std::vector<FontFace>& FontFaces() {
        return s_faces;
    }

    static FontFace* GetFontFace(const GString& fontName);

    static const json& FontAwesomeInfo() {
        return s_faInfo;
    }

    static GStringUtf8 FaUnicodeCharacter(const GString& fontAwesomeIcon);

    /// @brief Load a font from a given file, with the specified encoding, and generates bitmap
    static void LoadFontFace(const GString& path, bool isCore, FontEncoding encoding = FontEncoding::kASCII);

    /// @}

	/// @name Constructors/Destructor
	/// @{

    FontManager(const GString& resourcesPath);
	~FontManager();

	/// @}

	/// @name Public Methods
	/// @{

    virtual void postConstruction() override;

    /// @brief Font awesome font families
    /// @note Only actually used for Qt functionality, i.e. to create QIcons
    /// @todo Separate this from main manager class
    static QString RegularFontAwesomeFamily();
    static QString BrandFontAwesomeFamily();
    static QString SolidFontAwesomeFamily();

	/// @}


protected:
    friend class Label;
    friend class FontFace;

    /// @name Protected Methods
    /// @{

    static void initializeFreeType();

    /// @brief Initialize fonts to use in GL
    void initializeGLFonts();

    /// @brief Initialize font awesome for application
    void initializeWidgetFonts();

    /// @}

    /// @name Protected members
    /// @{

    GString m_resourcesPath; ///< The path to the resources used by the font manager

    /// @brief The wrapped freetype library
    static std::unique_ptr<FT_Library> s_freeType;
    
    /// @brief Vector of fonts wrapping "faces", i.e. fonts loaded via freetype
    static std::vector<FontFace> s_faces;

    /// @brief Font Awesome font family IDs
    static int s_faBrands;
    static int s_faRegular;
    static int s_faSolid;

    /// @brief JSON document containing font-awesome icon metadata
    static json s_faInfo;

    /// @}
};


} // End rev namespaces

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FONT_MANAGER_H
#define GB_FONT_MANAGER_H

// Standard

// External

// Internal
#include <core/canvas/GFontFace.h>

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
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

    static std::vector<FontFace>& FontFaces() {
        return s_faces;
    }

    static FontFace* GetFontFace(const GString& fontName);

    static const QJsonObject& FontAwesomeInfo() {
        return s_faInfo;
    }

    static QString FaUnicodeCharacter(const QString& fontAwesomeIcon);

    /// @brief Load a font from a given file, with the specified encoding, and generates bitmap
    static void LoadFontFace(const QString& path, bool isCore, FontEncoding encoding = FontEncoding::kASCII);

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
    static QString RegularFontAwesomeFamily();
    static QString BrandFontAwesomeFamily();
    static QString SolidFontAwesomeFamily();

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
    static std::vector<FontFace> s_faces;

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
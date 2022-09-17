#pragma once

// std
#include <vector>

// External
#include "fortress/json/GJson.h"
#include "fortress/layer/application/GManagerInterface.h"

// Internal
#include "fonts/GFontFace.h"

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

    /// @brief Return a font face given the font name
    /// @param fontName the font name
    /// @return the font face object
    static FontFace* GetFontFace(const GString& fontName);

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

	/// @}


protected:
    friend class Label;
    friend class FontFace;

    /// @name Protected Methods
    /// @{

    /// @brief Initialize FreeType
    static void initializeFreeType();

    /// @brief Initialize fonts to use in GL
    void initializeGLFonts();

    /// @}

    /// @name Protected members
    /// @{

    GString m_resourcesPath; ///< The path to the resources used by the font manager
    static std::unique_ptr<FT_Library> s_freeType; ///< The wrapped freetype library
    static std::vector<FontFace> s_faces; ///<  Vector of fonts wrapping "faces", i.e. fonts loaded via freetype

    /// @}
};


} // End rev namespaces

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ICON_H
#define GB_ICON_H

// QT

// Internal
#include "GLabel.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Icon class
class Icon : public Label {

public:
	/// @name Constructors/Destructor
	/// @{
	~Icon();
	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    virtual GlyphType glyphType() const override { return GlyphType::kIcon; };


    const QString& iconName() const { return m_iconName; }

    /// @brief Set to given font awesome icon
    void setFontAwesomeIcon(const QString& iconName);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Icon& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Icon& orObject);


    /// @}

protected:

    Icon(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex);

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    QString m_iconName;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
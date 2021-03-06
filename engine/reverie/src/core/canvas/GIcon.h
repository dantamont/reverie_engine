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
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Icon(CanvasComponent* canvas);
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
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}


    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    QString m_iconName;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
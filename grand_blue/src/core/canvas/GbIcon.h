/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_ICON_H
#define GB_ICON_H

// QT

// Internal
#include "GbLabel.h"

namespace Gb {


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

    /// @brief Set to given font awesome icon
    void setFontAwesomeIcon(const QString& iconName);

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @}


};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_COLOR_H
#define GB_COLOR_H

// QT
#include <QColor>

// Internal
#include "../geometry/GVector.h"

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Color class
class Color : public QColor {

public:
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Color();
    Color(const std::vector<real_g>& vec);
    Color(const std::vector<int>& vec);
    Color(const Vector3& vec);
    Color(const Vector4& vec);
    Color(int r, int g, int b, int a = 255);
    Color(const QColor& color);
	~Color();
	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    std::vector<int> toVector4i() const;

    Vector4 toVector4g() const;

    Vector3 toVector3g() const;

	/// @}
protected:

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
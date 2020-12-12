/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_Nameable_H
#define GB_Nameable_H

// QT
//#include <QString>

// Internal
#include "../containers/GbString.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Class representing an object that can be named
class Nameable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum NameMode {
        kCaseSensitive,
        kCaseInsensitive
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    explicit Nameable(const GString& name, NameMode mode = kCaseSensitive);
    Nameable() {}
    virtual ~Nameable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Name
    inline const GString& getName() const {
        return m_name;
    }
    inline GString& getName() {
        return m_name;
    }
    void setName(const GString& name);

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

	/// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Name to the object
    GString m_name;

    /// @brief Naming mode
    NameMode m_nameMode;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
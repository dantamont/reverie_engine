/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_Nameable_H
#define GB_Nameable_H

// QT
//#include <QString>

// Internal
#include "../containers/GString.h"
#include "../encoding/GUUID.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object with a unique ID
class Identifiable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Identifiable(const Uuid& uuid) :
        m_uuid(uuid) {
    }
    Identifiable() :
        m_uuid(true) {
    }
    virtual ~Identifiable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property UUID
    /// @brief Uuid of this object
    inline const Uuid& getUuid() const { return m_uuid; }


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


    /// @brief Unique identifier for this object
    rev::Uuid m_uuid;

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
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
    Nameable():
        m_nameMode(NameMode::kCaseSensitive) {
    }
    virtual ~Nameable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Name
    inline const GString& getName() const {
        return m_name;
    }

    // Removed, since can accidentally be modified if case-insensitive
    //inline GString& getName() {
    //    return m_name;
    //}
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
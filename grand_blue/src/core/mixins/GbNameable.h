/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_Nameable_H
#define GB_Nameable_H

// QT
#include <QString>

// Internal

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
    Nameable(const QString& name, NameMode mode = kCaseSensitive);
    Nameable() {}
    virtual ~Nameable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property Name
    virtual const QString& getName() const;
    virtual void setName(const QString& name);

    /// @brief Name as a standard string
    inline const std::string& nameAsStdString() const { return m_name.toStdString(); }


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
    QString m_name;

    /// @brief Naming mode
    NameMode m_nameMode;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
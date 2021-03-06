/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_BYTE_ARRAY_H
#define GB_BYTE_ARRAY_H

// QT

// Internal
#include "GFlags.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class ByteArrayFlag {
    kTakeOwnership = 1 << 0 // If set, byte array takes ownership over data
};
typedef Flags<ByteArrayFlag> ByteArrayFlags;

/// @brief Byte array
/// @details Takes ownership of given data by default, and will delete
class ByteArray{
public:
	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    template<typename T>
    ByteArray(T* data, ByteArrayFlags flags = ByteArrayFlag::kTakeOwnership):
        m_dataFlags(flags),
        m_byteData(static_cast<char*>(data))
    {
    }

    ByteArray();
    ~ByteArray();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    char* data() { return m_byteData; }
    const char* data() const { return m_byteData; }

    /// #}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    template<typename T>
    T* as() {
        return dynamic_cast<T*>(m_byteData);
    }

    template<typename T>
    void setData(T* data) {
        m_bytedata = static_cast<char*>(data);
    }

	/// @}


protected:
    /// @brief The flags associated with the byte array
    ByteArrayFlags m_dataFlags;

    /// @brief The data encapsulated by the byte array
    char* m_byteData;

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
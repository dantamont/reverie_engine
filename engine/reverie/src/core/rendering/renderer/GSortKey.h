/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SORT_KEY
#define GB_SORT_KEY

// Standard

// QT
#include <QString>

// Internal

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class SortKey
/// @brief Class representing a configurable sort key for rendering
class SortKey {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Convert a floating point number to and from binary
    static unsigned long FloatToBinary(float number);
    static float BinaryToFloat(unsigned long binary);

    /// @brief Convert a floating point number to a binary that sorts in ascending order
    static unsigned long FloatToSortedBinary(float number);
    //static float SortedBinaryToFloat(unsigned long binary);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    SortKey();
    SortKey(unsigned long long key);
    ~SortKey();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    unsigned long long& key() { return m_key; }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator<(const SortKey& k1, const SortKey& k2) {
        return k1.m_key < k2.m_key;
    }

    operator QString() const {
        return bitStr(m_key);
    }

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set number of bits at nth bit to the given value
    /*!
      \param value The value to set
      \param n the bit at which to insert the LSB of the value
      \param numBits the number of bits to insert from the specified value
      \param rightShift the number of bits to right-shift the value before insertion
    */
    template<typename T>
    bool setBits(T value, size_t n, size_t numBits = sizeof(T) * 8, size_t rightShift = 0) {
        unsigned long long longValue = value;
        longValue = longValue >> rightShift;

        size_t numBitsType = sizeof(T) * 8;
        if (n + numBits > 64) {
#ifdef DEBUG_MODE
            throw("Error, cannot set this many bits");
#endif
            return false;
        }

        unsigned long long int mask = 0;
        size_t i;
        unsigned long long one = 1;
        for (i = 0; i < numBits; i++) {
            mask |= one << (i + n);
        }

        unsigned long long shiftedMaskedValue = (longValue << n) & mask;

#ifdef DEBUG_MODE
        if (shiftedMaskedValue != (longValue << n) && numBits >= numBitsType) {
            throw("Error, bits lost in conversion");
        }
#endif

        // Clear mask region from key
        m_key &= ~mask;

        // Set value in key
        m_key |= shiftedMaskedValue;

        return true;
    }

	/// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Return a binary number as a bitStr
    template<typename T>
    static QString bitStr(T val) {
        QString out;
        size_t numBits = sizeof(val) * 8;
        unsigned long long i;
        unsigned long long one = 1;
        for (i = one << (numBits - 1); i > 0; i = i / 2) {
            out += (val & i) ? "1" : "0";
        }

        return out;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The actual bit-field key
    unsigned long long m_key = 0;
    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
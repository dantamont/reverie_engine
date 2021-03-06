#ifndef GB_FLAGS_H
#define GB_FLAGS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
#include <QFlags>

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Allow enum to operate bitwise
#define MAKE_BITWISE(ENUMTYPE) inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b){ return static_cast<ENUMTYPE>(static_cast<int>(a) | static_cast<int>(b)); }

#define MAKE_FLAGS(ENUMTYPE, FLAGS_TYPE) typedef Flags<ENUMTYPE> FLAGS_TYPE;
#define MAKE_FLAGS_EXT(ENUMTYPE, FLAGS_TYPE, ITYPE) typedef Flags<ENUMTYPE, ITYPE> FLAGS_TYPE;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

template<typename FlagType, typename IType = uint32_t>
class Flags {
public:

    /// @brief Convert an integer to a QFlags object of the specified enum type
    template<typename IntType>
    static QFlags<FlagType> toQFlags(IntType integer) {
        QFlags<FlagType> flags;
        uint32_t unsignedInt = (uint32_t)integer;
        int count = 0;
        while (unsignedInt != 0) {
            // Get least-significant byte and set in QFlag
            bool lsb = unsignedInt & 1;
            uint32_t currVal = 1 << count;
            flags.setFlag(FlagType(currVal), lsb);

            // Shift over yet another time
            unsignedInt = unsignedInt >> 1;
            count++;
        }

        return flags;
    }


    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Flags() :
        m_flags(0)
    {
    }

    Flags(IType flags) :
        m_flags(flags)
    {
    }
    Flags(FlagType flags) :
        m_flags((IType)flags)
    {
    }
    ~Flags() {
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    IType& value() { return m_flags; }


    IType toInt() const { return m_flags; }

    inline bool testFlag(FlagType flag) const noexcept {
        // Check that flag is already set and is either nonzero or no bits are set
        return (m_flags & IType(flag)) == IType(flag) && (IType(flag) != 0 || m_flags == IType(flag));
    }
    inline Flags &setFlag(FlagType flag, bool on = true) noexcept
    {
        return on ? (*this |= flag) : (*this &= ~IType(flag));
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    inline Flags &operator&=(IType mask) noexcept {
        m_flags &= mask; 
        return *this; 
    }
    inline Flags &operator&=(FlagType mask) noexcept { 
        m_flags &= IType(mask);
        return *this;
    }
    inline Flags &operator|=(Flags other) noexcept { 
        m_flags |= other.m_flags; 
        return *this;
    }
    inline Flags &operator|=(FlagType other) noexcept { 
        m_flags |= IType(other);
        return *this; 
    }
    inline Flags &operator^=(Flags other) noexcept { 
        m_flags ^= other.m_flags; 
        return *this; 
    }
    inline Flags &operator^=(FlagType other) noexcept {
        m_flags ^= IType(other);
        return *this; 
    }
    inline operator IType() const noexcept { return m_flags; }
    inline Flags operator|(Flags other) const noexcept { return Flags(m_flags | other.m_flags); }
    inline Flags operator|(FlagType other) const noexcept { return Flags(m_flags | IType(other)); }
    inline Flags operator^(Flags other) const noexcept { return Flags(m_flags ^ other.m_flags); }
    inline Flags operator^(FlagType other) const noexcept { return Flags(m_flags ^ IType(other)); }
    inline Flags operator&(IType mask) const noexcept { return Flags(m_flags & mask); }
    inline Flags operator&(FlagType other) const noexcept { return Flags(m_flags & IType(other)); }
    inline Flags operator~() const noexcept { return Flags(~m_flags); }
    inline bool operator!() const noexcept { return !m_flags; }

    /// @}

private:

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief The encapsulate flags
    IType m_flags;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
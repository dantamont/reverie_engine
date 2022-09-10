#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Allow enum to operate bitwise
#define MAKE_BITWISE(ENUMTYPE) inline ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b){ return static_cast<ENUMTYPE>(static_cast<Uint32_t>(a) | static_cast<Uint32_t>(b)); }

#define MAKE_FLAGS(ENUMTYPE, FLAGS_TYPE) typedef Flags<ENUMTYPE> FLAGS_TYPE;
#define MAKE_FLAGS_EXT(ENUMTYPE, FLAGS_TYPE, ITYPE) typedef Flags<ENUMTYPE, ITYPE> FLAGS_TYPE;


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

template<typename FlagType, typename IType = Uint32_t>
class Flags {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    constexpr Flags() :
        m_flags(0)
    {
    }
    constexpr Flags(const Flags& other):
        m_flags(other.m_flags)
    {
    }
    constexpr Flags(Flags&& other):
        m_flags(other.m_flags)
    {
    }
    constexpr Flags(IType flags) :
        m_flags(flags)
    {
    }
    constexpr Flags(FlagType flags) :
        m_flags((IType)flags)
    {
    }
    ~Flags() {
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    constexpr IType& value() { return m_flags; }


    constexpr IType toInt() const { return m_flags; }

    constexpr inline bool testFlag(FlagType flag) const noexcept {
        // Check that flag is already set and is either nonzero or no bits are set
        return (m_flags & IType(flag)) == IType(flag) && (IType(flag) != 0 || m_flags == IType(flag));
    }
    inline Flags& setFlag(FlagType flag, bool on = true) noexcept
    {
        return on ? (*this |= flag) : (*this &= ~IType(flag));
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    constexpr inline Flags& operator=(IType other) noexcept {
        m_flags = other;
        return *this;
    }
    constexpr inline Flags& operator=(const Flags& other) noexcept {
        m_flags = other.m_flags;
        return *this;
    }
    constexpr inline Flags& operator=(Flags&& other) noexcept {
        m_flags = other.m_flags;
        return *this;
    }

    constexpr inline Flags &operator&=(IType mask) noexcept {
        m_flags &= mask; 
        return *this; 
    }
    constexpr inline Flags &operator&=(FlagType mask) noexcept {
        m_flags &= IType(mask);
        return *this;
    }
    constexpr inline Flags &operator|=(Flags other) noexcept {
        m_flags |= other.m_flags; 
        return *this;
    }
    constexpr inline Flags &operator|=(FlagType other) noexcept {
        m_flags |= IType(other);
        return *this; 
    }
    constexpr inline Flags &operator^=(Flags other) noexcept {
        m_flags ^= other.m_flags; 
        return *this; 
    }
    constexpr inline Flags &operator^=(FlagType other) noexcept {
        m_flags ^= IType(other);
        return *this; 
    }
    constexpr inline operator IType() const noexcept { return m_flags; }
    constexpr inline Flags operator|(Flags other) const noexcept { return Flags(m_flags | other.m_flags); }
    constexpr inline Flags operator|(FlagType other) const noexcept { return Flags(m_flags | IType(other)); }
    constexpr inline Flags operator^(Flags other) const noexcept { return Flags(m_flags ^ other.m_flags); }
    constexpr inline Flags operator^(FlagType other) const noexcept { return Flags(m_flags ^ IType(other)); }
    constexpr inline Flags operator&(IType mask) const noexcept { return Flags(m_flags & mask); }
    constexpr inline Flags operator&(FlagType other) const noexcept { return Flags(m_flags & IType(other)); }
    constexpr inline Flags operator~() const noexcept { return Flags(~m_flags); }
    constexpr inline bool operator!() const noexcept { return !m_flags; }

    /// @}

private:

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief The encapsulate flags
    IType m_flags;

    /// @}
};


} // End namespaces


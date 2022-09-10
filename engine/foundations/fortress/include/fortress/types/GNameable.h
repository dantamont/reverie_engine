#pragma once

// Internal
#include "fortress/string/GString.h"

namespace rev {

/// @class NameableInterface
/// @brief Interface class representing an object that can be named
/// @todo Make a templated class for name mode
class NameableInterface{
public:
    /// @name Static
    /// @{

    enum NameMode {
        kCaseSensitive,
        kCaseInsensitive
    };

    /// @}

	/// @name Constructors/Destructor
	/// @{
    explicit NameableInterface(const GString& name, NameMode mode = kCaseSensitive) :
        m_nameMode(mode)
    {
        setName(name);
    }
    NameableInterface():
        m_nameMode(NameMode::kCaseSensitive) {
    }

	/// @}

    /// @name Properties
    /// @{

    /// @property Name
    inline const GString& getName() const {
        return m_name;
    }

    void setName(const GString& name) {
        switch (m_nameMode) {
        case kCaseSensitive:
            m_name = name;
            break;
        case kCaseInsensitive:
            m_name = name.asLower();
            break;
        default:
            m_name = name;
        }
    }


    /// @}

protected:

    /// @brief Destructor made protected to prevent polymorphic deletion of non-virtual destructor
    ~NameableInterface() {}

    /// @name Private Members
    /// @{

    /// @brief Name to the object
    GString m_name;

    /// @brief Naming mode
    NameMode m_nameMode;

    /// @}

};


} // End namespaces

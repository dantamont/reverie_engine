/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_DICTIONARY_H
#define GB_DICTIONARY_H

// QT
#include <vector>
#include <QString>

// Internal
#include "GbGVariant.h"
#include "../encoding/GbUUID.h"


namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class Dictionary
/// @brief Struct for storing attributes of a resource
/// @details For passing into processResource function of LoadProcess
class Dictionary : public Serializable {
public:
    Dictionary() {}
    Dictionary(const QJsonValue& json);
    ~Dictionary() {}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{
    /// @brief Check whether dictionary has the given attribute
    bool hasAttribute(const QString& name) const {
        return m_attributes.find(name) != m_attributes.end();
    }

    /// @brief Check if empty or not
    bool isEmpty() const { return m_attributes.empty(); }

    /// @brief Return attribute
    const GVariant& at(const QString& name) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    
    GVariant& operator[](const QString& key);
    const GVariant& operator[](const QString& key) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    std::map<QString, GVariant> m_attributes;

    static GVariant s_invalidVariant;
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
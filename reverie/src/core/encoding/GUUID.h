#ifndef GB_UUID_H
#define GB_UUID_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External
#include <QUuid>
#include <QByteArray>
#include <QMetaType>

// Project
#include "../containers/GString.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** @class Uuid
    @brief  A subclass of a QUuid
    @details A Universally Unique Identifier (Uuid) is a standard way to uniquely identify entities
        in a distributed computing environment
*/
class Uuid: public QUuid {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Statiic
    /// @{
    
    /// @brief Generate a unique name
    static GString UniqueName(const GString& prefix);
    static GString UniqueName();
    static QString UniqueQName();

    static Uuid NullID();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    Uuid(const QUuid& uuid);
    Uuid(const GString& str);
    explicit Uuid(size_t id);
    explicit Uuid(const QString& str);
    explicit Uuid(bool generate = true);
    ~Uuid();
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    Uuid& operator=(const QUuid& uuid);

    /// @note Will only preserve ID for manually-specified Uuids that are smaller than sizeof(unsigned int)
    explicit operator size_t() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Obtain string representation of the UUID
    GString asString() const;
    QString asQString() const;

    /// @brief Generate a unique name from the UUID
    GString createUniqueName(const GString& prefix) const;
    GString createUniqueName() const;
    QString createUniqueQName() const;

    /// @}

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing


///@ brief Define std::hash overload for rev::Uuid
namespace std {
template <> struct std::hash<rev::Uuid>
{
    size_t operator()(const rev::Uuid& uuid) const noexcept
    {
        return uuid.data1 ^ uuid.data2 ^ (uuid.data3 << 16)
            ^ ((uuid.data4[0] << 24) | (uuid.data4[1] << 16) | (uuid.data4[2] << 8) | uuid.data4[3])
            ^ ((uuid.data4[4] << 24) | (uuid.data4[5] << 16) | (uuid.data4[6] << 8) | uuid.data4[7])
            ^ 7; // Seed, could be anything
    }
};

}

#endif 

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
#include "../containers/GbString.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

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
    explicit Uuid(const QString& str);
    explicit Uuid(bool generate = true);
    ~Uuid();
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{
    Uuid& operator=(const QUuid& uuid);
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
Q_DECLARE_METATYPE(Uuid)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

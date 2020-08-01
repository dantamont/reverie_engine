#ifndef GB_UUID_H
#define GB_UUID_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External
#include <QUuid>
#include <QByteArray>
#include <QString>
#include <QMetaType>

// Project

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
    static QString UniqueName(const QString& prefix);
    static QString UniqueName();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    Uuid(const QUuid& uuid);
    Uuid(const QString& str);
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
    QString asString() const;

    /// @brief Generate a unique name from the UUID
    QString createUniqueName(const QString& prefix = QString()) const;

    /// @}

private:
};
Q_DECLARE_METATYPE(Uuid)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

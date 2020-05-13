// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_G_GVariant_H
#define GB_G_GVariant_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <cstdlib>
#include <memory>
#include <vector>
#include <list>

// QT

// Internal 
#include "../mixins/GbLoadable.h"
#include "GbVariant.h"
#include "../geometry/GbMatrix.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief struct representing a GVariant for an openGLShader
struct GVariant :
    public Serializable,
    public Variant<bool, int, float, QString,
    Vector2f, Vector3f, Vector4f, Matrix4x4f,
    std::vector<float>>
{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Convert QVariantMap to map of GVariants
    static QVariantMap toQVariantMap(const std::map<QString, GVariant>& map);
    static std::map<QString, GVariant> toGVariantMap(const QVariantMap& map);

    /// @brief Convert Map of GVariants to QVariantMap

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    /// @}

    GVariant();
    GVariant(const QVariant& qv);
    GVariant(bool val);
    GVariant(int val);
    GVariant(float val);
    GVariant(const QString& val);
    GVariant(const Vector2f& val);
    GVariant(const Vector3f& val);
    GVariant(const Vector4f& val);
    GVariant(const Matrix4x4f& val);
    GVariant(const std::vector<float>& val);

    ~GVariant();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Convert to QVariant
    QVariant asQVariant() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Load from a QVariant
    void loadFromQVariant(const QVariant& qv);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @}

};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
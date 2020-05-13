// See:
// https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/

#ifndef GB_UNIFORM_H
#define GB_UNIFORM_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QObject>
#include <QString>

// Internal 
#include "../../containers/GbVariant.h"
#include "../../geometry/GbMatrix.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3g> Vec3List;
typedef std::vector<Vector4g> Vec4List;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief struct containing a uniform's name and type
struct UniformInfo{

    enum PrecisionQualifier {
        kNone=-1,
        kHigh,
        kMedium,
        kLow
    };

    UniformInfo();
    UniformInfo(const QString& name, const QString& type, bool isArray);
    UniformInfo(const QString& name, const QString& type, bool isArray, int id);

    QString m_name = "";
    QString m_typeStr = ""; // variable type in glsl, e.g. vec3, vec2, float
    int m_uniformType = -1;
    bool m_isArray = false; // whether the uniform is an array or not
    int m_uniformID = -1; // ID to be set when bound to a shader
    int m_arraySize = -1;
    PrecisionQualifier m_precision = kNone;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a uniform's name and type
struct ShaderStruct {
    QString m_name;
    std::vector<UniformInfo> m_fields;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct representing a uniform for an openGLShader
struct Uniform :
    public Serializable,
    public Variant<bool, int, real_g,
    Vector2g, Vector3g, Vector4g, 
    Matrix2x2g, Matrix3x3g, Matrix4x4g, 
    Vec3List,
    Vec4List,
    std::vector<Matrix4x4g>,
    std::vector<real_g>>
{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum UniformType {
        kNone=-1,
        kBool,
        kInt,
        kFloat,
        kDouble,
        kVec2,
        kVec3,
        kVec4,
        kMat2,
        kMat3,
        kMat4,
        kSamplerCube,
        kSampler2D
    };

    static std::unordered_map<QString, std::type_index> UNIFORM_TYPE_MAP;
    static std::unordered_map<QString, UniformType> UNIFORM_TYPE_STR_MAP;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    /// @}

    Uniform();
    Uniform(const QJsonValue& json);
    Uniform(const QString& name);
    template<typename T>
    Uniform(const QString& name, const T& val) :
        m_name(name)
    {
        set<T>(val);
    }
    template<>
    Uniform(const QString& name, const QVariant& qv);

    ~Uniform();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    inline const QString& getName() const { return m_name; }

    bool isPersistent() const { return m_persistent; }
    void setPersistence(bool p) { m_persistent = p; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Whether or not the uniform's type ID matches the given uniform info struct
    bool matchesInfo(const UniformInfo& typeInfo) const;

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
    /// @name Static
    /// @{
    friend class ShaderProgram;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Load from a QVariant
    void loadFromQVariant(const QVariant& qv);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief the name of the uniform in the GL shader
    QString m_name;

    /// @brief Whether or not to save this uniform value on shader save 
    bool m_persistent = true;

    /// @}

};
/////////////////////////////////////////////////////////////////////////////////////////////
template<>
inline Uniform::Uniform(const QString & name, const QVariant & qv):
    m_name(name)
{
    loadFromQVariant(qv);
}


    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
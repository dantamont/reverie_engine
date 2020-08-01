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
#include "../GbGLFunctions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3g> Vec3List;
typedef std::vector<Vector4g> Vec4List;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class ShaderInputType {
    kNone = -1,
    kBool = GL_BOOL,
    kInt = GL_INT,
    kFloat = GL_FLOAT,
    kDouble = GL_DOUBLE,
    kVec2 = GL_FLOAT_VEC2,
    kVec3 = GL_FLOAT_VEC3,
    kVec4 = GL_FLOAT_VEC4,
    kMat2 = GL_FLOAT_MAT2,
    kMat3 = GL_FLOAT_MAT3,
    kMat4 = GL_FLOAT_MAT4,
    kSamplerCube = GL_SAMPLER_CUBE,
    kSampler2D = GL_SAMPLER_2D
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a uniform's name and type
struct ShaderInputInfo{

    /// @brief Determines if the given value is a valid ShaderInputType
    static bool IsValidGLType(int typeInt);

    enum class PrecisionQualifier {
        kNone=-1,
        kHigh,
        kMedium,
        kLow
    };

    enum ShaderInputFlag {
        kIsArray = 1 << 0,
        kInBlockOrBuffer = 1 << 1
    };

    ShaderInputInfo();
    ShaderInputInfo(const QString& name, const ShaderInputType& type, bool isArray);
    ShaderInputInfo(const QString& name, const ShaderInputType& type, bool isArray, int id);

    bool isArray() const { return m_flags.testFlag(kIsArray); }
    bool inBlockOrBuffer() const { return m_flags.testFlag(kInBlockOrBuffer); }

    QString m_name = "";
    //QString m_typeStr = ""; // variable type in glsl, e.g. vec3, vec2, float
    ShaderInputType m_inputType = ShaderInputType::kNone; // Uniform::UniformType // Supports GL_UNIFORM, GL_PROGRAM_INPUT, GL_PROGRAM_OUTPUT, GL_TRANSFORM_FEEDBACK_VARYING, GL_BUFFER_VARIABLE
    QFlags<ShaderInputFlag> m_flags; // whether the uniform is an array or not
    int m_uniformID = -1; // ID to be set when bound to a shader
    int m_localIndex = -1; // Index of the uniform in the m_uniforms vector of corresponding ShaderProgram object
    int m_arraySize = -1;
    PrecisionQualifier m_precision = PrecisionQualifier::kNone;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct containing a uniform's name and type
struct ShaderStruct {
    QString m_name;
    std::vector<ShaderInputInfo> m_fields;
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

    static std::unordered_map<QString, std::type_index> UNIFORM_TYPE_MAP;
    static std::unordered_map<ShaderInputType, std::type_index> UNIFORM_GL_TYPE_MAP;
    static std::unordered_map<QString, ShaderInputType> UNIFORM_TYPE_STR_MAP;

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
    /// @name Operators
    /// @{

    /// @brief Conversion to QString
    operator QString() const;

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
    bool matchesInfo(const ShaderInputInfo& typeInfo) const;

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
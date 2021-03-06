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
#include "../../third_party/tsl/robin_map.h"
#include "../../containers/GVariant.h"
#include "../../containers/GStringView.h"
#include "../../geometry/GMatrix.h"
#include "../../containers/GContainerExtensions.h"
#include "../GGLFunctions.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderInputInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
// TypeDefs
/////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////

enum class ShaderVariableType {
    kCustomStruct = -2,
    kNone = -1,
    kBool = GL_BOOL,
    kInt = GL_INT,
    kFloat = GL_FLOAT,
    kDouble = GL_DOUBLE,
    kUVec2 = GL_UNSIGNED_INT_VEC2,
    kUVec3 = GL_UNSIGNED_INT_VEC3,
    kUVec4 = GL_UNSIGNED_INT_VEC4,
    kIVec2 = GL_INT_VEC2,
    kIVec3 = GL_INT_VEC3,
    kIVec4 = GL_INT_VEC4,
    kVec2 = GL_FLOAT_VEC2,
    kVec3 = GL_FLOAT_VEC3,
    kVec4 = GL_FLOAT_VEC4,
    kMat2 = GL_FLOAT_MAT2,
    kMat3 = GL_FLOAT_MAT3,
    kMat4 = GL_FLOAT_MAT4,
    kSamplerCube = GL_SAMPLER_CUBE,
    kSamplerCubeArray = GL_SAMPLER_CUBE_MAP_ARRAY,
    kSampler2D = GL_SAMPLER_2D,
    kSampler2DShadow = GL_SAMPLER_2D_SHADOW,
    kSampler2DArray = GL_SAMPLER_2D_ARRAY
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief struct representing a uniform for an openGLShader
struct Uniform :
    public Serializable,
    public Variant<bool, int, real_g,
    Vector2i, Vector3i, Vector4i,
    Vector2u, Vector3u, Vector4u,
    Vector2, Vector3, Vector4, 
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

    static tsl::robin_map<QString, std::type_index> s_uniformTypeMap;
    static tsl::robin_map<ShaderVariableType, std::type_index> s_uniformGLTypeMap;
    static tsl::robin_map<QString, ShaderVariableType> s_uniformTypeStrMap;
    static tsl::robin_map<ShaderVariableType, QString> s_uniformStrTypeMap;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    /// @}

    Uniform();
    Uniform(const Uniform& other);
    Uniform(const QJsonValue& json);
    explicit Uniform(const GStringView& name);
    template<typename T>
    Uniform(const GStringView& name, const T& val):
        m_name(name)
    {
        set<T>(val);
    }

    ~Uniform();
    /// @}


    //---------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    /// @brief Conversion to QString
    operator QString() const;

    Uniform& operator=(const Uniform& rhs);


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    inline const GStringView& getName() const { return m_name; }
    inline void setName(const GString& name) { m_name = name; }

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
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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
    void loadValue(const QJsonValue& json);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief the name of the uniform in the GL shader
    GStringView m_name;

    /// @brief Whether or not to save this uniform value on shader save 
    bool m_persistent = true;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
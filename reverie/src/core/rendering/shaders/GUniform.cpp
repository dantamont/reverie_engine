#include "GUniform.h"
#include "GShaderProgram.h"
// QT

// Internal

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Uniform
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform() :
    Variant()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const Uniform & other):
    Variant(other),
    m_name(other.m_name),
    m_persistent(other.m_persistent)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const QJsonValue & json)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const GStringView& name):
    m_name(name)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::~Uniform()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::operator QString() const
{
    QString string;
    if (is<int>()) {
        string = QString::number(get<int>());
    }
    else if (is<bool>()) {
        string = QString::number(get<bool>());
    }
    else if (is<real_g>()) {
        string = QString::number(get<real_g>());
    }
    else if (is<Vector2>()) {
        string = QString(get<Vector2>());
    }
    else if (is<Vector3>()) {
        string = QString(get<Vector3>());
    }
    else if (is<Vector4>()) {
        string = QString(get<Vector4>());
    }
    else if (is<Vector2i>()) {
        string = QString(get<Vector2i>());
    }
    else if (is<Vector3i>()) {
        string = QString(get<Vector3i>());
    }
    else if (is<Vector4i>()) {
        string = QString(get<Vector4i>());
    }
    else if (is<Vector2u>()) {
        string = QString(get<Vector2u>());
    }
    else if (is<Vector3u>()) {
        string = QString(get<Vector3u>());
    }
    else if (is<Vector4u>()) {
        string = QString(get<Vector4u>());
    }
    else if (is<Matrix2x2g>()) {
        string = QString(get<Matrix2x2g>());
    }
    else if (is<Matrix3x3g>()) {
        string = QString(get<Matrix3x3g>());
    }
    else if (is<Matrix4x4g>()) {
        string = QString(get<Matrix4x4g>());
    }
    else if (is<std::vector<Matrix4x4g>>()) {
        const std::vector<Matrix4x4>& mats = get<std::vector<Matrix4x4g>>();
        string += "{";
        for (const Matrix4x4& mat : mats) {
            string += QString(mat) + ", \n";
        }
        string += "}";
    }
    else if (is<std::vector<real_g>>()) {
        const std::vector<real_g>& reals = get<std::vector<real_g>>();
        string += "{";
        for (real_g num : reals) {
            string += QString::number(num) + ", ";
        }
        string += "}";
    }
    else if (is<Vec3List>()) {
        const Vec3List& vecs = get<Vec3List>();
        string += "{";
        for (const Vector3& vec : vecs) {
            string += QString(vec) + ", \n";
        }
        string += "}";
    }
    else if (is<Vec4List>()) {
        const Vec4List& vecs = get<Vec4List>();
        string += "{";
        for (const Vector4& vec : vecs) {
            string += QString(vec) + ", \n";
        }
        string += "}";
    }
    else {
        QString err = "Error, this uniform to QString conversion is not supported";
#ifdef DEBUG_MODE
        QString typeName = QString::fromStdString(Variant::typeInfo().name());
        err += ": " + typeName;
#endif
        throw(err);
    }

    return QString(m_name) + ": " + string;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform & Uniform::operator=(const Uniform & rhs)
{
    // TODO: insert return statement here
    Variant::operator=(rhs);
    m_name = rhs.m_name;
    m_persistent = rhs.m_persistent;

    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Uniform::matchesInfo(const ShaderInputInfo& typeInfo) const
{
    int inputType = (int)typeInfo.m_variableType;
    if (!ShaderInputInfo::IsValidGLType(inputType)) {
        return false;
    }

    //const std::type_index& type = s_uniformGLTypeMap.at(typeInfo.m_variableType);
    if (typeInfo.m_variableCType == Variant::typeInfo()) {
        return true;
    }
    else {
        if (!typeInfo.isArray()) {
            // If the uniform doesn't match and is not an array type
//                for (const auto& uniformPair : typeMap) {
//
//#ifdef DEBUG_MODE
//                    if (uniformPair.second == m_typeID) {
//                        qDebug() << "Type mismatch for expected type " << 
//                            QString::fromStdString(uniformPair.first)
//                            << " and given type " 
//                            << QString::fromStdString(typeInfo.m_type);
//                        break;
//                    }
//#endif
//                }
            return false;
        }
        else {
            // Return true for array types
            return true;
        }
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
QVariant Uniform::asQVariant() const
{
    QVariant qv;
    if (is<int>()) {
        qv.setValue(get<int>());
    }
    else if (is<bool>()) {
        qv.setValue(get<bool>());
    }
    else if (is<real_g>()) {
        qv.setValue(get<real_g>());
    }
    else if (is<Vector2>()) {
        qv.setValue(get<Vector2>().asJson());
    }
    else if (is<Vector3>()) {
        qv.setValue(get<Vector3>().asJson());
    }
    else if (is<Vector4>()) {
        qv.setValue(get<Vector4>().asJson());
    }
    //else if (is<Vector2i>()) {
    //    qv.setValue(get<Vector2i>().asJson());
    //}
    //else if (is<Vector3i>()) {
    //    qv.setValue(get<Vector3i>().asJson());
    //}
    //else if (is<Vector4i>()) {
    //    qv.setValue(get<Vector4i>().asJson());
    //}
    //else if (is<Vector2u>()) {
    //    qv.setValue(get<Vector2u>().asJson());
    //}
    //else if (is<Vector3u>()) {
    //    qv.setValue(get<Vector3u>().asJson());
    //}
    //else if (is<Vector4u>()) {
    //    qv.setValue(get<Vector4u>().asJson());
    //}
    else if (is<Matrix2x2g>()) {
        qv.setValue(get<Matrix2x2g>().asJson());
    }
    else if (is<Matrix3x3g>()) {
        qv.setValue(get<Matrix3x3g>().asJson());
    }
    else if (is<Matrix4x4g>()) {
        qv.setValue(get<Matrix4x4g>().asJson());
    }
    else if (is<std::vector<Matrix4x4g>>()) {
        const std::vector<Matrix4x4>& mat = get<std::vector<Matrix4x4g>>();
        qv.setValue(matrixVecAsJson(mat));
    }
    else if (is<std::vector<real_g>>()) {
        qv.setValue(vectorAsJson(get<std::vector<real_g>>()));
    }
    else if (is<Vec3List>()) {
        qv.setValue(vecOfVecAsJson(get<Vec3List>()));
    }
    else if (is<Vec4List>()) {
        qv.setValue(vecOfVecAsJson(get<Vec4List>()));
    }
    else {
        QString err = "Error, this uniform to QVariant conversion is not supported";
#ifdef DEBUG_MODE
        QString typeName = QString::fromStdString(Variant::typeInfo().name());
        err += ": " + typeName;
#endif
        throw(err);
    }

    return qv;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Uniform::asJson(const SerializationContext& context) const
{
    QVariantMap map;
    map.insert("name", m_name.c_str());
    map.insert("value", asQVariant());
    return QJsonObject::fromVariantMap(map);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Uniform::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    
    // Todo: Don't serialize uniforms, should always be a member or otherwise
    // SEE: https://stackoverflow.com/questions/4484982/how-to-convert-typename-t-to-string-in-c
    const QJsonObject& object = json.toObject();
    if (object.contains("name")) {
        // Parse json if name and value are separate key-value pairs
        // Note, the name is no longer set here, since a string-view requires a persistent string
        //m_name = object.value("name").toString();

        loadValue(object["value"]);
    }
    else if(object.size() == 1){
        // DEPRECATED
        for (const QString& key : object.keys()) {
            m_name = key;
            loadValue(object[key]);
        }
    }
    else {
#ifdef DEBUG_MODE
        // Incorrect JSON format
        throw("Error, uniform JSON format is not recognized");
#else
        logError("Error, uniform JSON format is not recognized");
#endif
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Uniform::loadValue(const QJsonValue& json)
{
    // FIXME: Do this more elegantly
    QJsonValue::Type type = json.type();
    if (type == QJsonValue::Bool) {
        set<bool>(json.toBool());
    }
    else if (type == QJsonValue::Double) {
        real_g val = json.toDouble();
        if (val == std::floor(val)) {
            set<int>((int) val);
        }
        else {
            set<real_g>(val);
        }
    }
    else if (json.isArray()){        
        // Determine type from form of JSON
        QJsonArray array = json.toArray();
        if (array.size() == 0) {
            throw("Error, empty array should not be stored in JSON");
        }
        else if (array.at(0).isString()) {
            // Is a matrix type
            int columnSize = array.at(1).toArray().size();
            if (columnSize == 4) {
                set<Matrix4x4g>(Matrix4x4g(json));
            }
            else if (columnSize == 3) {
                set<Matrix3x3g>(Matrix3x3g(json));
            }
            else if (columnSize == 2) {
                set<Matrix2x2g>(Matrix2x2g(json));
            }
            else {
                throw("Error, matrix of this size is not JSON serializable");
            }
        }
        else if(array.at(0).isArray()){
            if (array.at(0).toArray().at(0).isString()) {
                // Is a vector of Matrix type
                int columnSize = array.at(0).toArray().at(1).toArray().size();
                if (columnSize == 4) {
                    set<std::vector<Matrix4x4g>>(matrixVecFromJson(json));
                }
            }
            else {
                // Is a std::vec of Vector type
                if (array.at(0).toArray().size() == 3) {
                    set<Vec3List>(vecOfVecFromJson<real_g, 3>(json));
                }
                else if(array.at(0).toArray().size() == 4) {
                    set<Vec4List>(vecOfVecFromJson<real_g, 4>(json));
                }
                else {
                    throw("Error, std:vec of Vector of this size is not JSON serializable");
                }
            }
        }
        else {
            
            // Is a vector type
            if (array.size() > 4) {
                set<std::vector<real_g>>(vectorFromJson<real_g>(json));
            }
            else if (array.size() == 4) {
                set<Vector4>(Vector4(json));
            }
            else if (array.size() == 3) {
                set<Vector3>(Vector3(json));
            }
            else if (array.size() == 2) {
                set<Vector2>(Vector2(json));
            }
        }
    }
    else {
        throw("Error, unsupported JSON");
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<QString, std::type_index> Uniform::s_uniformTypeMap = {
    {"bool", typeid(bool)},
    {"int", typeid(int)},
    {"float", typeid(float)},
    {"double", typeid(double)},
    {"uvec2", typeid(Vector<unsigned int, 2>)},
    {"uvec3", typeid(Vector<unsigned int, 3>)},
    {"uvec4", typeid(Vector<unsigned int, 4>)},
    {"ivec2", typeid(Vector<int, 2>)},
    {"ivec3", typeid(Vector<int, 3>)},
    {"ivec4", typeid(Vector<int, 4>)},
    {"vec2", typeid(Vector2)},
    {"vec3", typeid(Vector3)},
    {"vec4", typeid(Vector4)},
    {"mat2", typeid(Matrix2x2g)},
    {"mat3", typeid(Matrix3x3g)},
    {"mat4", typeid(Matrix4x4g)},
    {"samplerCube", typeid(int)},
    {"samplerCubeArray", typeid(int)},
    {"sampler2D", typeid(int)},
    {"sampler2DShadow", typeid(int)},
    {"sampler2DArray", typeid(int)}
};
/////////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<ShaderVariableType, std::type_index> Uniform::s_uniformGLTypeMap = {
    {ShaderVariableType::kBool, typeid(bool)},
    {ShaderVariableType::kInt, typeid(int)},
    {ShaderVariableType::kFloat, typeid(float)},
    {ShaderVariableType::kDouble, typeid(double)},
    {ShaderVariableType::kUVec2, typeid(Vector<unsigned int, 2>)},
    {ShaderVariableType::kUVec3, typeid(Vector<unsigned int, 3>)},
    {ShaderVariableType::kUVec4, typeid(Vector<unsigned int, 4>)},
    {ShaderVariableType::kIVec2, typeid(Vector<int, 2>)},
    {ShaderVariableType::kIVec3, typeid(Vector<int, 3>)},
    {ShaderVariableType::kIVec4, typeid(Vector<int, 4>)},
    {ShaderVariableType::kVec2, typeid(Vector2)},
    {ShaderVariableType::kVec3, typeid(Vector3)},
    {ShaderVariableType::kVec4, typeid(Vector4)},
    {ShaderVariableType::kMat2, typeid(Matrix2x2g)},
    {ShaderVariableType::kMat3, typeid(Matrix3x3g)},
    {ShaderVariableType::kMat4, typeid(Matrix4x4g)},
    {ShaderVariableType::kSamplerCube, typeid(int)},
    {ShaderVariableType::kSamplerCubeArray, typeid(int)},
    {ShaderVariableType::kSampler2D, typeid(int)},
    {ShaderVariableType::kSampler2DShadow, typeid(int)},
    {ShaderVariableType::kSampler2DArray, typeid(int)}
};
///////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<QString, ShaderVariableType> Uniform::s_uniformTypeStrMap = {
    {"bool",   ShaderVariableType::kBool},
    {"int",    ShaderVariableType::kInt},
    {"float",  ShaderVariableType::kFloat},
    {"double", ShaderVariableType::kDouble},
    {"ivec2",   ShaderVariableType::kIVec2},
    {"ivec3",   ShaderVariableType::kIVec3},
    {"ivec4",   ShaderVariableType::kIVec4},
    {"uvec2",   ShaderVariableType::kUVec2},
    {"uvec3",   ShaderVariableType::kUVec3},
    {"uvec4",   ShaderVariableType::kUVec4},
    {"vec2",   ShaderVariableType::kVec2},
    {"vec3",   ShaderVariableType::kVec3},
    {"vec4",   ShaderVariableType::kVec4},
    {"mat2",   ShaderVariableType::kMat2},
    {"mat3",   ShaderVariableType::kMat3},
    {"mat4",   ShaderVariableType::kMat4},
    {"samplerCube", ShaderVariableType::kSamplerCube},
    {"samplerCubeArray", ShaderVariableType::kSamplerCubeArray},
    {"sampler2D", ShaderVariableType::kSampler2D},
    {"sampler2DShadow", ShaderVariableType::kSampler2DShadow},
    {"sampler2DArray", ShaderVariableType::kSampler2DArray}
};

///////////////////////////////////////////////////////////////////////////////////////////
tsl::robin_map<ShaderVariableType, QString> Uniform::s_uniformStrTypeMap = {
    {ShaderVariableType::kBool,   "bool"},
    {ShaderVariableType::kInt,    "int"},
    {ShaderVariableType::kFloat,  "float"},
    {ShaderVariableType::kDouble, "double"},
    {ShaderVariableType::kUVec2,   "uvec2"},
    {ShaderVariableType::kUVec3,   "uvec3"},
    {ShaderVariableType::kUVec4,   "uvec4"},
    {ShaderVariableType::kIVec2,   "ivec2"},
    {ShaderVariableType::kIVec3,   "ivec3"},
    {ShaderVariableType::kIVec4,   "ivec4"},
    {ShaderVariableType::kVec2,   "vec2"},
    {ShaderVariableType::kVec3,   "vec3"},
    {ShaderVariableType::kVec4,   "vec4"},
    {ShaderVariableType::kMat2,   "mat2"},
    {ShaderVariableType::kMat3,   "mat3"},
    {ShaderVariableType::kMat4,   "mat4"},
    {ShaderVariableType::kSamplerCube, "samplerCube"},
    {ShaderVariableType::kSamplerCubeArray, "samplerCubeArray"},
    {ShaderVariableType::kSampler2D,   "sampler2D"},
    {ShaderVariableType::kSampler2DShadow,   "sampler2DShadow"},
    {ShaderVariableType::kSampler2DArray,   "sampler2DArray"}
};

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
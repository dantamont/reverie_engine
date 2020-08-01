#include "GbUniform.h"

// QT

// Internal

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// ShaderInputInfo
/////////////////////////////////////////////////////////////////////////////////////////////
bool ShaderInputInfo::IsValidGLType(int typeInt)
{
    switch (ShaderInputType(typeInt)) {
    case ShaderInputType::kBool:
    case ShaderInputType::kInt:
    case ShaderInputType::kFloat:
    case ShaderInputType::kDouble:
    case ShaderInputType::kVec2:
    case ShaderInputType::kVec3:
    case ShaderInputType::kVec4:
    case ShaderInputType::kMat2:
    case ShaderInputType::kMat3:
    case ShaderInputType::kMat4:
    case ShaderInputType::kSamplerCube:
    case ShaderInputType::kSampler2D:
        return true;
    default:
        throw("GL type is not valid, need to account for this type");
        return false;
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
ShaderInputInfo::ShaderInputInfo(const QString & name, const ShaderInputType & type, bool isArray) :
    m_name(name),
    m_inputType(type)
{
    m_flags.setFlag(kIsArray, isArray);
}

ShaderInputInfo::ShaderInputInfo(const QString & name, const ShaderInputType & type, bool isArray, int id) :
    m_name(name),
    m_inputType(type),
    m_uniformID(id)
{
    m_flags.setFlag(kIsArray, isArray);
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Uniform
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform() :
    Variant(),
    m_name("")
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const QJsonValue & json)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const QString & name):
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
    else if (is<Vector2g>()) {
        string = QString(get<Vector2g>());
    }
    else if (is<Vector3g>()) {
        string = QString(get<Vector3g>());
    }
    else if (is<Vector4g>()) {
        string = QString(get<Vector4g>());
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
        const std::vector<Matrix4x4f>& mats = get<std::vector<Matrix4x4g>>();
        string += "{";
        for (const Matrix4x4f& mat : mats) {
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
        for (const Vector3g& vec : vecs) {
            string += QString(vec) + ", \n";
        }
        string += "}";
    }
    else if (is<Vec4List>()) {
        const Vec4List& vecs = get<Vec4List>();
        string += "{";
        for (const Vector4g& vec : vecs) {
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

    return m_name + ": " + string;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Uniform::matchesInfo(const ShaderInputInfo& typeInfo) const
{
    int inputType = (int)typeInfo.m_inputType;
    if (!ShaderInputInfo::IsValidGLType(inputType)) {
        return false;
    }

    const std::type_index& type = UNIFORM_GL_TYPE_MAP.at(typeInfo.m_inputType);
    if (type == Variant::typeInfo()) {
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
    else if (is<Vector2g>()) {
        qv.setValue(get<Vector2g>().asJson());
    }
    else if (is<Vector3g>()) {
        qv.setValue(get<Vector3g>().asJson());
    }
    else if (is<Vector4g>()) {
        qv.setValue(get<Vector4g>().asJson());
    }
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
        const std::vector<Matrix4x4f>& mat = get<std::vector<Matrix4x4g>>();
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
QJsonValue Uniform::asJson() const
{
    QVariantMap map;
    map.insert("name", m_name);
    map.insert("value", asQVariant());
    return QJsonObject::fromVariantMap(map);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Uniform::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    if (object.contains("name")) {
        // Parse json if name and value are separate key-value pairs
        m_name = object.value("name").toString();

        QVariantMap map = object.toVariantMap();
        loadFromQVariant(map["value"]);
    }
    else if(object.size() == 1){
        for (const QString& key : object.keys()) {
            m_name = key;
            QVariantMap map = object.toVariantMap();
            loadFromQVariant(map[key]);
        }
    }
    else {
#ifdef DEBUG_MODE
        // Incorrect JSON format
        throw("Error, uniform JSON format is not recognized");
#endif
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Uniform::loadFromQVariant(const QVariant & qv)
{
    int type = qv.type();
    if (type == QMetaType::Int) {
        set<int>(qv.value<int>());
    }
    else if (type == QMetaType::Bool) {
        set<bool>(qv.value<bool>());
    }
    else if (type == QMetaType::Float || type == QMetaType::Double) {
        set<real_g>(qv.value<real_g>());
    }
    else if (qv.canConvert<QJsonValue>()){
        QJsonValue json = qv.value<QJsonValue>();
        
        if (!json.isArray()) {
            throw("Error, JSON must be an array type");
        }

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
                set<Vector4g>(Vector4g(json));
            }
            else if (array.size() == 3) {
                set<Vector3g>(Vector3g(json));
            }
            else if (array.size() == 2) {
                set<Vector2g>(Vector2g(json));
            }
        }
    }
    else {
        throw("Error, this uniform to QVariant conversion is not supported");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<QString, std::type_index> Uniform::UNIFORM_TYPE_MAP = {
    {"bool", typeid(bool)},
    {"int", typeid(int)},
    {"float", typeid(float)},
    {"double", typeid(double)},
    {"vec2", typeid(Vector2g)},
    {"vec3", typeid(Vector3g)},
    {"vec4", typeid(Vector4g)},
    {"mat2", typeid(Matrix2x2g)},
    {"mat3", typeid(Matrix3x3g)},
    {"mat4", typeid(Matrix4x4g)},
    {"samplerCube", typeid(int)},
    {"sampler2D", typeid(int)}
};
/////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<ShaderInputType, std::type_index> Uniform::UNIFORM_GL_TYPE_MAP = {
    {ShaderInputType::kBool, typeid(bool)},
    {ShaderInputType::kInt, typeid(int)},
    {ShaderInputType::kFloat, typeid(float)},
    {ShaderInputType::kDouble, typeid(double)},
    {ShaderInputType::kVec2, typeid(Vector2g)},
    {ShaderInputType::kVec3, typeid(Vector3g)},
    {ShaderInputType::kVec4, typeid(Vector4g)},
    {ShaderInputType::kMat2, typeid(Matrix2x2g)},
    {ShaderInputType::kMat3, typeid(Matrix3x3g)},
    {ShaderInputType::kMat4, typeid(Matrix4x4g)},
    {ShaderInputType::kSamplerCube, typeid(int)},
    {ShaderInputType::kSampler2D, typeid(int)}
};
///////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<QString, ShaderInputType> Uniform::UNIFORM_TYPE_STR_MAP = {
    {"bool",   ShaderInputType::kBool},
    {"int",    ShaderInputType::kInt},
    {"float",  ShaderInputType::kFloat},
    {"double", ShaderInputType::kDouble},
    {"vec2",   ShaderInputType::kVec2},
    {"vec3",   ShaderInputType::kVec3},
    {"vec4",   ShaderInputType::kVec4},
    {"mat2",   ShaderInputType::kMat2},
    {"mat3",   ShaderInputType::kMat3},
    {"mat4",   ShaderInputType::kMat4},
    {"samplerCube", ShaderInputType::kSamplerCube},
    {"sampler2D", ShaderInputType::kSampler2D}
};

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
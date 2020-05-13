#include "GbUniform.h"

// QT

// Internal

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Uniform Info

UniformInfo::UniformInfo()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
UniformInfo::UniformInfo(const QString & name, const QString & type, bool isArray) :
    m_name(name),
    m_typeStr(type),
    m_uniformType(Uniform::UNIFORM_TYPE_STR_MAP[type]),
    m_isArray(isArray)
{

}

UniformInfo::UniformInfo(const QString & name, const QString & type, bool isArray, int id) :
    m_name(name),
    m_typeStr(type),
    m_isArray(isArray), 
    m_uniformType(Uniform::UNIFORM_TYPE_STR_MAP[type]),
    m_uniformID(id)
{

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
bool Uniform::matchesInfo(const UniformInfo& typeInfo) const
{
    auto iter = UNIFORM_TYPE_MAP.find(typeInfo.m_typeStr);
    if (iter != UNIFORM_TYPE_MAP.end()) {
        if (iter->second == Variant::typeInfo()) {
            return true;
        }
        else {
            if (!typeInfo.m_isArray) {
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
    else {
        return false;
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
std::unordered_map<QString, Uniform::UniformType> Uniform::UNIFORM_TYPE_STR_MAP = {
    {"bool",   kBool},
    {"int",    kInt},
    {"float",  kFloat},
    {"double", kDouble},
    {"vec2",   kVec2},
    {"vec3",   kVec3},
    {"vec4",   kVec4},
    {"mat2",   kMat2},
    {"mat3",   kMat3},
    {"mat4",   kMat4},
    {"samplerCube", kSamplerCube},
    {"sampler2D", kSampler2D}
};

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespacing
}
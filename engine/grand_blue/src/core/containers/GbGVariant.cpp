#include "GbGVariant.h"
#include "GbContainerExtensions.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
QVariantMap GVariant::toQVariantMap(const std::map<QString, GVariant>& gMap)
{
    QVariantMap map;
    for (const std::pair<QString, GVariant>& variantPair : gMap) {
        const QString& key = variantPair.first;
        const GVariant& variant = variantPair.second;
        map.insert(key, variant.asQVariant());
    }

    return map;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::map<QString, GVariant> GVariant::toGVariantMap(const QVariantMap & map)
{
    std::map<QString, GVariant> gMap;
    for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
        QString key = iter.key();
        QVariant qVariant = iter.value();
        Map::Emplace(gMap, key, qVariant);
    }

    return gMap;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant() :
    Variant()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const QVariant & qv):
    Variant()
{
    loadFromQVariant(qv);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(bool val) :
    Variant()
{
    set<bool>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(int val) :
    Variant()
{
    set<int>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(float val) :
    Variant()
{
    set<float>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const QString & val)
{
    set<QString>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const Vector2f & val) :
    Variant()
{
    set<Vector2f>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const Vector3f & val) :
    Variant()
{
    set<Vector3f>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const Vector4f & val) :
    Variant()
{
    set<Vector4f>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const Matrix4x4f & val) :
    Variant()
{
    set<Matrix4x4f>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::GVariant(const std::vector<float>& val)
{
    set<std::vector<float>>(val);
}
/////////////////////////////////////////////////////////////////////////////////////////////
GVariant::~GVariant()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QVariant GVariant::asQVariant() const
{
    QVariant qv;
    if (is<int>()) {
        qv.setValue(get<int>());
    }
    else if (is<bool>()) {
        qv.setValue(get<bool>());
    }
    else if (is<float>()) {
        qv.setValue(get<float>());
    }
    else if (is<QString>()) {
        qv.setValue(get<QString>());
    }
    else if (is<Vector2f>()) {
        qv.setValue(get<Vector2f>().asJson());
    }
    else if (is<Vector3f>()) {
        qv.setValue(get<Vector3f>().asJson());
    }
    else if (is<Vector4f>()) {
        qv.setValue(get<Vector4f>().asJson());
    }
    else if (is<Matrix4x4f>()) {
        qv.setValue(get<Matrix4x4f>().asJson());
    }
    else if (is<std::vector<float>>()) {
        qv.setValue(vectorAsJson(get<std::vector<float>>()));
    }
    else {
        throw("Error, this uniform to QVariant conversion is not supported");
    }

    return qv;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GVariant::loadFromQVariant(const QVariant & qv)
{
    int type = qv.type();
    if (type == QMetaType::Int) {
        set<int>(qv.value<int>());
    }
    else if (type == QMetaType::Bool) {
        set<bool>(qv.value<bool>());
    }
    else if (type == QMetaType::Float || type == QMetaType::Double) {
        real_g tmp = qv.value<real_g>();
        if (tmp >= std::numeric_limits<int>::min() && // value is not too small
            tmp <= std::numeric_limits<int>::max() && // value is not too big
            std::floor(tmp) == tmp // value does not have decimals, if it's not ok
            ) {
            set<int>(int(std::floor(tmp))); // let's be specific about rounding, if decimals are ok
        }
        else {
            set<real_g>(tmp);
        }
    }
    else if (type == QMetaType::QString) {
        set<QString>(qv.value<QString>());
    }
    else if (qv.canConvert<QJsonValue>()) {
        QJsonValue json = qv.value<QJsonValue>();
        if (!json.isArray()) {
            throw("Error, JSON must be an array type");
        }

        // Determine type from form of JSON
        QJsonArray array = json.toArray();
        if (array.size() == 0) {
            throw("Error, empty array should not be stored in JSON");
        }
        else if (array.at(0).isArray()) {
            // Is a matrix type
            int columnSize = array.at(0).toArray().size();
            if (columnSize == 4) {
                set<Matrix4x4f>(Matrix4x4f(json));
            }
            else if (columnSize == 3) {
                set<Matrix3x3f>(Matrix3x3f(json));
            }
            else {
                throw("Error, matrix of this size is not JSON serializable");
            }
        }
        else {
            // Is a vector type
            if (array.size() > 4) {
                set<std::vector<float>>(vectorFromJson<float>(json));
            }
            else if (array.size() == 4) {
                set<Vector4f>(Vector4f(json));
            }
            else if (array.size() == 3) {
                set<Vector3f>(Vector3f(json));
            }
            else if (array.size() == 2) {
                set<Vector2f>(Vector2f(json));
            }
        }
    }
    else {
        throw("Error, this uniform to QVariant conversion is not supported");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue GVariant::asJson() const
{
    throw("Error, not implemented");
    return QJsonValue();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GVariant::loadFromJson(const QJsonValue & json)
{
    throw("Error, not implemented");
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
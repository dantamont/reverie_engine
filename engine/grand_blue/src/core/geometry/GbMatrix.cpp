#include "GbMatrix.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue matrixVecAsJson(const std::vector<Matrix4x4>& vec)
{
    QJsonArray array;
    for (const auto& m : vec) {
        array.append(m.asJson());
    }
    return array;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Matrix4x4> matrixVecFromJson(const QJsonValue & json)
{
    std::vector<Matrix4x4> mats;
    const QJsonArray& array = json.toArray();
    for (int i = 0; i < array.size(); i++) {
        Matrix4x4 m;
        m.loadFromJson(array.at(i));
        Vec::EmplaceBack(mats, std::move(m));
    }
    return mats;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
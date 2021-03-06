#include "GJsonReader.h"

#include <QJsonDocument>

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
QJsonDocument JsonReader::ToJsonDocument(const QString & str)
{
    return QJsonDocument::fromJson(str.toUtf8());
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject JsonReader::ToJsonObject(const QString & str)
{
    return ToJsonDocument(str).object();
}
/////////////////////////////////////////////////////////////////////////////////////////////
JsonReader::JsonReader(const QString & filepath):
    FileReader(filepath)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
JsonReader::~JsonReader()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QVariantMap JsonReader::getContentsAsVariantMap()
{
    // Get variant map
    QVariantMap dataMap;
    if (FileExists()) {
        dataMap = getContentsAsJsonObject().toVariantMap();
    }

    return dataMap;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject JsonReader::getContentsAsJsonObject()
{
    QJsonObject object;
    if (FileExists()) {

        // Get document
        QJsonDocument doc = QJsonDocument::fromJson(getContentsAsByteArray());

        // Get object
         object = doc.object();
    }

    return object;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

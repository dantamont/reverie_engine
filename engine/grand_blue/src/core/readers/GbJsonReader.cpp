#include "GbJsonReader.h"

#include <QJsonDocument>

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
QString JsonReader::getJsonValueAsQString(const QJsonValue & json, bool verbose)
{
    QJsonDocument doc(json.toObject());
    QJsonDocument::JsonFormat format;
    if (verbose) {
        format = QJsonDocument::Indented;
    }
    else {
        format = QJsonDocument::Compact;
    }
    QString strJson(doc.toJson(format));
    return strJson;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonDocument JsonReader::getQStringAsJsonDocument(const QString & str)
{
    return QJsonDocument::fromJson(str.toUtf8());
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
    if (fileExists()) {
        dataMap = getContentsAsJsonObject().toVariantMap();
    }

    return dataMap;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject JsonReader::getContentsAsJsonObject()
{
    QJsonObject object;
    if (fileExists()) {

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

#include "GbFileReader.h"
#include <QFileInfo>
#include <QDir>
#include "../containers/GbContainerExtensions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
QRegExp FileReader::EMPTY_LINE("\\s+");
QRegExp FileReader::NEW_LINE("(\\r?\\n)+");
QRegExp FileReader::SPACE_OR_TAB("[\\s\\t]+");


/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::createHexName(const QString& str)
{
    return QString(QCryptographicHash::hash(QByteArray(str.toUtf8()), QCryptographicHash::Md5).toHex());
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::pathToName(const QString& path, bool extension, bool caseSensitive)
{
    // Convert filepath to a case-insensitive name
    QFile f(path);
    QFileInfo fileInfo(f.fileName());
    QString filename;
    if (caseSensitive) {
        filename = fileInfo.fileName();
    }
    else {
        filename = fileInfo.fileName().toLower();
    }
    if (!extension) {
        // Remove extension
        QStringList parts = filename.split(".");
        filename = "";
        for (int i = 0; i < parts.size() - 1; i++) {
            filename += parts[i];
        }
    }
    return filename;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::dirFromPath(const QString & path)
{
    QFileInfo info = QFileInfo(path);
    return info.absoluteDir().absolutePath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileReader::fileExists(const QString & filepath)
{
    return QFile::exists(filepath);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::getContentsAsString(const QString & filepath)
{
    // Read into QT file
    QFile readFile(filepath);

    // Verify that the file was successfully opened
    bool result = readFile.open(QIODevice::ReadOnly);
    if (!result) {
#ifdef DEBUG_MODE
        FileReader().logError("Unable to open file: " + filepath);
        //throw("Error, unable to open file");
#endif
        return "";
    }

    // Obtain file contents as a byte array
    QByteArray bstream = readFile.readAll();
    readFile.close();

    // Convert to a string and return
    QString content(bstream);
    return content;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> FileReader::splitString(const std::string & str, const char* delimiter)
{
    std::vector<std::string> results;
    std::string::const_iterator start = str.begin();
    std::string::const_iterator end = str.end();
    std::string::const_iterator next = std::find(start, end, *delimiter);
    while (next != end) {
        Vec::EmplaceBack(results, std::string(start, next));
        start = next + 1;
        next = std::find(start, end, *delimiter);
    }
    Vec::EmplaceBack(results, std::string(start, next));
    return results;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QString> FileReader::splitString(const QString & str, const char* delimiter)
{
    std::vector<std::string> results = splitString(str.toStdString(), delimiter);
    std::vector<QString> qResults;
    for (const auto& str : results) {
        Vec::EmplaceBack(qResults, QString::fromStdString(str));
    }
    return qResults;
}

/////////////////////////////////////////////////////////////////////////////////////////////
FileReader::FileReader() :
    m_filePath(""),
    m_fileExtension("")
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileReader::FileReader(const QString& filepath):
    m_filePath(filepath),
    m_fileExtension(m_filePath.split(".").back())
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
FileReader::~FileReader()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
QStringList FileReader::getFileLines()
{
    return getContentsAsString().split(NEW_LINE);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileReader::fileExists()
{
    return fileExists(m_filePath);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QByteArray FileReader::getContentsAsByteArray()
{
    // Read into QT file
    QFile readFile(m_filePath);

    // Verify that the file was successfully opened
    bool result = readFile.open(QIODevice::ReadOnly);
    if (!result) {
        logCritical("Unable to open file: " + m_filePath);
        throw("Error, unable to open file");
    }

    // Obtain file contents as a byte array
    QByteArray bstream = readFile.readAll();
    readFile.close();
    return bstream;
}

/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::getContentsAsString()
{
    return getContentsAsString(m_filePath);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

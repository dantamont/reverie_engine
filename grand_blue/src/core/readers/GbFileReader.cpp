#include "GbFileReader.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
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
QString FileReader::PathToName(const QString& path, bool extension, bool caseSensitive)
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
QString FileReader::DirFromPath(const QString & path)
{
    QFileInfo info = QFileInfo(path);
    return info.absoluteDir().absolutePath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString FileReader::FileExtension(const QString & filepath)
{
    return filepath.split(".").back();
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileReader::fileExists(const QString & directory, const QString & fileName, QString & outFilePath)
{
    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    bool found = false;
    QString currentFile;
    do {
        // Iterate through directories to find file
        if (it.fileName() == fileName) {
            found = true;
            outFilePath = it.filePath();
            break;
        }
        //Object().logInfo(it.next());
        it.next();
    } while (it.hasNext());

    // Check for last iterator
    if (it.fileName() == fileName) {
        found = true;
        outFilePath = it.filePath();
    }

    return found;
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

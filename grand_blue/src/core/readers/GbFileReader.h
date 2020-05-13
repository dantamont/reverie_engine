/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FILE_READER_H
#define GB_FILE_READER_H

// QT
#include <QDebug>
#include <QString>
#include <QFile>
#include <QCryptographicHash>

// Internal
#include "../GbObject.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief For loading in data from a file
/// @detailed Stores positional data of a model in a VAO
// TODO: parallelize 
// https://stackoverflow.com/questions/34572043/how-can-i-asynchronously-load-data-from-large-files-in-qt
class FileReader: public Gb::Object {
public:
    // Static ///////////////////////////////////////////////////////////////////////////////
    // Various white space regular expressions
    static QRegExp EMPTY_LINE;
    static QRegExp NEW_LINE;
    static QRegExp SPACE_OR_TAB;

    /// @brief Generates a unique md5 hex code for a given string
    static QString createHexName(const QString& str);

    /// @brief Return filename from a path
    static QString pathToName(const QString& path, bool extension=true, bool caseSensitive = false);
    static QString dirFromPath(const QString& path);

    /// @brief Return true if file exists, false otherwise
    static bool fileExists(const QString& filepath);

    /// @brief Get contents of a file as a string
    static QString getContentsAsString(const QString& filepath);

    /// @brief Alternative to string split than Qt, is slower
    static std::vector<std::string> splitString(const std::string& str, const char* delimiter);
    static std::vector<QString> splitString(const QString& str, const char* delimiter);

    // Constructor/Destructor ///////////////////////////////////////////////////////////////////////////////
    FileReader();
    FileReader(const QString& filepath);
    virtual ~FileReader();

    // Public methods ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return lines contained in the file
    QStringList getFileLines();

    /// @brief Return true if file held by reader exists
    bool fileExists();

    /// @brief Returns file contents
    QByteArray getContentsAsByteArray();
    QString getContentsAsString();

protected:

    QString m_filePath;
    QString m_fileExtension;
};

        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////     
} // End namespaces

#endif
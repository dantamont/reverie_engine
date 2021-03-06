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
#include "../GObject.h"
#include "../containers/GString.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////

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
class FileReader: public rev::Object {
public:
    // Static ///////////////////////////////////////////////////////////////////////////////
    // Various white space regular expressions
    static QRegExp EMPTY_LINE;
    static QRegExp NEW_LINE;
    static QRegExp SPACE_OR_TAB;

    /// @brief Generates a unique md5 hex code for a given string
    //static QString createHexName(const QString& str);

    /// @brief Return filename from a path
    static QString PathToName(const QString& path, bool extension=true, bool caseSensitive = false);
    static QString DirFromPath(const QString& path);

    /// @brief Return absolute filepath from relative filepath and root directory
    static QString AbsolutePath(const char* rootDir, const char* relPath);

    /// @brief Return relative filepath from one path to another
    static QString RelativePath(const char* rootDir, const char* absPath);

    /// @brief Replace the file exension with the specified one
    static QString ReplaceExtension(const QString& fileName, const char* ext);

    /// @brief Return file extension
    static QString FileExtension(const QString& filepath);

    /// @brief Search subdirectories and files in given directory recursively for a file
    static bool FileExists(const QString& directory, const QString& fileName, QString& outPath);

    /// @brief Return true if file exists, false otherwise
    static bool FileExists(const QString& filepath);

    /// @brief Get contents of a file as a string
    static QString getContentsAsString(const QString& filepath);

    // Constructor/Destructor ///////////////////////////////////////////////////////////////////////////////
    FileReader();
    FileReader(const QString& filepath);
    virtual ~FileReader();

    // Public methods ///////////////////////////////////////////////////////////////////////////////
    /// @brief Return lines contained in the file
    QStringList getFileLines();

    /// @brief Return true if file held by reader exists
    bool FileExists();

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
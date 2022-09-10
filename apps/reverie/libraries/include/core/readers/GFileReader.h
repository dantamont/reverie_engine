#pragma once

// QT
#include <QDebug>
#include <QString>
#include <QFile>

// Internal
#include "fortress/string/GString.h"

namespace rev {  

/// @brief For loading in data from a file
/// @detailed Stores positional data of a model in a VAO
/// @todo Deprecate, or rename to make clear that this is only to load Qt resource files via ":my/file/path" convention
class FileReader {
public:

    /// @brief Return content of a Qt resource file
    /// @todo Deprecate this and create your own resource system.
    /// @see https://doc.qt.io/archives/qt-4.8/resources.html#using-resources-in-the-application
    static QString GetResourceFileContents(const GString& filePath);

};

        
     
} /// End namespaces

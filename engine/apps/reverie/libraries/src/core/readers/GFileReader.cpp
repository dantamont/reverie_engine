#include "core/readers/GFileReader.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include "geppetto/qt/layer/types/GQtConverter.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"
#include "logging/GLogger.h"

namespace rev {


QString FileReader::GetResourceFileContents(const GString& filepath)
{
    // Read into QT file
    QFile readFile(filepath.c_str());

    // Verify that the file was successfully opened
    bool result = readFile.open(QIODevice::ReadOnly);
    if (!result) {
#ifdef DEBUG_MODE
        Logger::LogError("Unable to open file: " + filepath);
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


// End namespaces
}

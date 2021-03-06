#include "GFileManager.h"

#include <filesystem>

#include "../containers/GStringView.h"
#include "../GCoreEngine.h"
#include "../readers/GFileReader.h"
#include "../scene/GScenario.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// FileManager
/////////////////////////////////////////////////////////////////////////////////////////////
void FileManager::SetCurrentDir(const char * dir)
{
    // FIXME: This crashes
    //std::error_code err;
    //std::filesystem::path newPath(dir);
    //std::filesystem::current_path(newPath, err);
    //if (err) {
    //    const char* errName = err.category().name();
    //    throw(errName);
    //}
    QDir::setCurrent(dir);
}
/////////////////////////////////////////////////////////////////////////////////////////////
const GString&  FileManager::GetRootPath()
{
    return s_rootPath;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const GString & FileManager::GetApplicationPath()
{
    return s_exePath;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const GString & FileManager::GetINIPath()
{
    return s_iniPath;
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileManager::FileManager(CoreEngine* engine, int argc, char *argv[]):
    Manager(engine, "FileManager")
{
    initialize(argc, argv);
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileManager::~FileManager()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString FileManager::searchPathString() const
{
    GString pathStr = "";
    size_t count = 0;
    size_t len = m_searchPaths.size();
    for (const std::filesystem::path& path : m_searchPaths) {
        pathStr += path.string().c_str();
        count++;
        if (count < len) {
            pathStr += "; ";
        }
    }
    return pathStr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
GString FileManager::getScenarioDirPath() const
{
    return GString(FileReader::DirFromPath(m_engine->scenario()->getPath().c_str()));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileManager::searchFor(const char * fileName, QString & outFilePath)
{
    bool exists = false;
    GString scenarioDirPath = getScenarioDirPath();
    for (const std::filesystem::path& path : m_searchPaths) {
        // Get path as absolute path
        QString absPath = FileReader::AbsolutePath(scenarioDirPath.c_str(), path.string().c_str());

        exists = FileReader::FileExists(absPath, fileName, outFilePath);
        if (exists) {
            break;
        }
    }

    return exists;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FileManager::addSearchPath(const char * path)
{
    // Split given path based on semi-colon delimiting
    GString pathStr(path);
    std::vector<GString> paths;
    pathStr.split(";", paths);

    // Iterate over all given search paths
    for (const GString& p : paths) {
        m_searchPaths.emplace_back(p.c_str());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FileManager::clear(bool addRootDir)
{
    m_searchPaths.clear();

    if (addRootDir) {
        // Default to just root directory as search path
        m_searchPaths.emplace_back(GetRootPath().c_str());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue FileManager::asJson(const SerializationContext & context) const
{
    QJsonObject object;
    QJsonArray searchPaths;
    for (const std::filesystem::path& path : m_searchPaths) {
        searchPaths.append(path.string().c_str());
    }
    object.insert("searchPaths", searchPaths);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FileManager::loadFromJson(const QJsonValue & json, const SerializationContext & context)
{
    m_searchPaths.clear();
    QJsonObject object = json.toObject();
    QJsonArray searchPaths = object["searchPaths"].toArray();
    for (const QJsonValueRef& json : searchPaths) {
        // deprecated
        if (json.isObject()) {
            m_searchPaths.emplace_back(json.toObject()["path"].toString().toStdString().c_str());
        }
        else {
            m_searchPaths.emplace_back(json.toString().toStdString().c_str());
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
const std::filesystem::path FileManager::CurrentDirectory()
{
    return std::filesystem::current_path();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void FileManager::initialize(int argc, char* argv[]) {
    /// Set current directory to the root directory of the application
    //std::string spath = GetRootPath().toStdString();
    //const char* rootPath = spath.c_str();

    std::vector<GString> args;
    for (int i = 0; i < argc; i++) {
        char* arg = argv[i];
        args.push_back(arg);
        Logger::LogInfo(argv[i]);
    }

    // Set executable path
    std::filesystem::path exePath(argv[0]);
    s_exeName = exePath.filename().string().c_str();
    exePath.remove_filename();
    s_exePath = exePath.string().c_str();

    if (argc < 2) {
        // If unspecified, default to .exe path for root path
        s_rootPath = QDir::cleanPath(QString(s_exePath)); // Go up two folders
    }
    else {
        // Otherwise, the root/working directory is specified as a command line argument
        s_rootPath = argv[1];
    }

    SetCurrentDir(s_rootPath.c_str());

    // Set INI path
    s_iniPath = s_rootPath + "/reverie.ini";
}

/////////////////////////////////////////////////////////////////////////////////////////////
GString FileManager::s_rootPath;

GString FileManager::s_iniPath;

GString FileManager::s_exePath;
GString FileManager::s_exeName;

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
#include "core/resource/GFileManager.h"

#include <filesystem>

#include "fortress/string/GStringView.h"
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"
#include "core/GCoreEngine.h"
#include "core/readers/GFileReader.h"
#include "core/scene/GScenario.h"

#include "logging/GLogger.h"

namespace rev {

// FileManager

void FileManager::SetCurrentDirToRoot()
{
    GDir::SetCurrentWorkingDir(s_rootPath.c_str());
}

const GString&  FileManager::GetRootPath()
{
    return s_rootPath;
}

const GString& FileManager::GetResourcePath()
{
    return s_resourcePath;
}

const GString & FileManager::GetApplicationPath()
{
    return s_exePath;
}

const GString & FileManager::GetIniPath()
{
    return s_iniPath;
}

FileManager::FileManager(CoreEngine* engine):
    Manager(engine, "FileManager")
{
}

FileManager::~FileManager()
{
}

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

GString FileManager::getScenarioDirPath() const
{
    GFile scenarioFile(m_engine->scenario()->getPath());
    return scenarioFile.getAbsoluteDirectory();
}

bool FileManager::searchFor(const char * fileName, GString & outFilePath)
{
    bool exists = false;
    GString scenarioDirPath = getScenarioDirPath();
    for (const std::filesystem::path& path : m_searchPaths) {
        // Get search directory path as absolute path
        GString absSearchDirPath = GPath::AbsolutePath(scenarioDirPath.c_str(), path.string().c_str());
        GDir searchDir(absSearchDirPath);
        exists = searchDir.containsFile(fileName, true, outFilePath);
        if (exists) {
            break;
        }
    }

    return exists;
}

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

void FileManager::clear(bool addRootDir)
{
    m_searchPaths.clear();

    if (addRootDir) {
        // Default to just root directory as search path
        m_searchPaths.emplace_back(GetRootPath().c_str());
    }
}

void to_json(json& orJson, const FileManager& korObject)
{
    json searchPaths = json::array();
    for (const std::filesystem::path& path : korObject.m_searchPaths) {
        searchPaths.push_back(path.string().c_str());
    }
    orJson["searchPaths"] = searchPaths;
}

void from_json(const json& korJson, FileManager& orObject)
{
    orObject.m_searchPaths.clear();
    
    const json& searchPaths = korJson["searchPaths"];
    for (const json& json : searchPaths) {
        // deprecated
        if (json.is_object()) {
            orObject.m_searchPaths.emplace_back(json["path"].get_ref<const std::string&>().c_str());
        }
        else {
            orObject.m_searchPaths.emplace_back(json.get_ref<const std::string&>().c_str());
        }
    }
}

void FileManager::SetApplicationPaths(int argc, char* argv[]) {
    /// Set current directory to the root directory of the application
    //std::string spath = GetRootPath().toStdString();
    //const char* rootPath = spath.c_str();

    std::vector<GString> args;
    for (int i = 0; i < argc; i++) {
        char* arg = argv[i];
        args.push_back(arg);
        Logger::LogDebug(argv[i]);
    }
    assert(args.size() > 1 && "Must specify a path to the initialization JSON file");

    // Set executable path
    // First argument is always .exe path
    std::filesystem::path exePath(argv[0]);
    s_exeName = exePath.filename().string().c_str();
    exePath.remove_filename();
    s_exePath = exePath.string().c_str();

    // Set INI path
    s_resourcePath = GString(args[1]) + "/resources/";
    s_iniPath = GString(args[1]) + "/reverie.json";

    // Set root path from settings
    GApplicationSettings& settings = GApplicationSettings::Instance();
    s_rootPath = settings.getWorkingDirectory();
    

    SetCurrentDirToRoot();

}


GString FileManager::s_rootPath;

GString FileManager::s_iniPath;
GString FileManager::s_resourcePath;
GString FileManager::s_exePath;
GString FileManager::s_exeName;


} // End namespaces
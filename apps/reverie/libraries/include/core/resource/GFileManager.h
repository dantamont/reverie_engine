#pragma once

// Qt
#include <filesystem>

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/string/GString.h"
#include "core/GManager.h"

namespace rev {

class CoreEngine;
class GStringView;

/// @class File Manager
// TODO: Add scenario subfolder with name of scenario, add resource subfolder to that, and subfolders for each type
// TODO: Maybe only do this for final build, TBD
class FileManager: public Manager {
public:
    /// @name Static
    /// @{

    /// @brief Initialize application paths such as the .exe directory and settings file location
    static void SetApplicationPaths(int argc, char* argv[]);

    /// @brief Set the current working directory to the application root (the exe folder)
    static void SetCurrentDirToRoot();

    /// @brief Get path to base Reverie directory
    static const GString& GetRootPath();

    /// @brief Get path to the folder where resources are stored
    static const GString& GetResourcePath();

    /// @brief Get path to the application exe
    static const GString& GetApplicationPath();

    /// @brief Get path to INI file 
    static const GString& GetIniPath();


    /// @}

	/// @name Constructors/Destructor
	/// @{
    FileManager(CoreEngine* engine);
	~FileManager();
	/// @}

    /// @name Properties
    /// @{

    /// @brief The search paths used to located resources
    const std::vector<std::filesystem::path>& searchPaths() const { return m_searchPaths; }
    GString searchPathString() const;

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Get path to the scenario location
    GString getScenarioDirPath() const;

    /// @brief Search for the given file in all search directories
    bool searchFor(const char* fileName, GString& outFilePath);

    /// @brief Add a search path, may be multiple paths delimited by semi-colons
    void addSearchPath(const char* path);

    /// @brief For clearing file manager on scenario switch
    void clear(bool addRootDir = true);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const FileManager& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, FileManager& orObject);

    /// @}

protected:
    /// @name Protected Members
    /// @{

    /// @brief the search paths used to locate scenario files
    /// @note Relative paths are relative to the root directory
    std::vector<std::filesystem::path> m_searchPaths;

    //

    /// @brief The root/working directory
    static GString s_rootPath;

    /// @brief The path to look for the INI file
    static GString s_iniPath;

    static GString s_resourcePath; ///< The path to use to look for resources

    /// @brief the directory of the application exe
    static GString s_exePath;
    static GString s_exeName;

    /// @}
};

} /// End namespaces

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FILE_MANAGER_H
#define GB_FILE_MANAGER_H

// Qt
#include <filesystem>

// Internal
#include "../mixins/GLoadable.h"
#include "../containers/GString.h"
#include "../GManager.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class GStringView;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class File Manager
// TODO: Add scenario subfolder with name of scenario, add resource subfolder to that, and subfolders for each type
// TODO: Maybe only do this for final build, TBD
class FileManager: public Manager, public Serializable {
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Get the current working directory
    static const std::filesystem::path CurrentDirectory();

    /// @brief Set the current working directory
    static void SetCurrentDir(const char* dir);

    /// @brief Get path to base Reverie directory
    static const GString& GetRootPath();

    /// @brief Get path to the application exe
    static const GString& GetApplicationPath();

    /// @brief Get path to INI file 
    static const GString& GetINIPath();


    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    FileManager(CoreEngine* engine, int argc, char *argv[]);
	~FileManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The search paths used to located resources
    const std::vector<std::filesystem::path>& searchPaths() const { return m_searchPaths; }
    GString searchPathString() const;

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Get path to the scenario location
    GString getScenarioDirPath() const;

    /// @brief Search for the given file in all search directories
    bool searchFor(const char* fileName, QString& outFilePath);

    /// @brief Add a search path, may be multiple paths delimited by semi-colons
    void addSearchPath(const char* path);

    /// @brief For clearing file manager on scenario switch
    void clear(bool addRootDir = true);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "FileManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::FileManager"; }
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty());

    /// @}

signals:


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void initialize(int argc, char* argv[]);

    /// @}

    //--------------------------------------------------------------------------------------------
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

    /// @brief the directory of the application exe
    static GString s_exePath;
    static GString s_exeName;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
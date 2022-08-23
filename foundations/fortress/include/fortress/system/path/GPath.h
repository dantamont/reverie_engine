#pragma once

// std
#include <filesystem>
#include <regex>

// Public
#include "fortress/types/GString.h"

namespace rev {

class GDir;
class GFile;
class GString;

/// @class GPath
/// @brief Class for path I/O related helper functions
class GPath {
public:

    /// @name Static
    /// @{

    /// @brief Return the system's preferred seperator
    static char Separator();

    /// @brief Whether or not the given path exists
    /// @details Applies to both files and directories
    static bool Exists(const char* path);

    /// @return if the path is absolute
    static bool IsAbsolute(const char* path);

    /// @return if the path is relative
    /// @note The path "/" is absolute on a POSIX OS, but is relative on Windows. 
    static bool IsRelative(const char* path);

    /// @brief Returns the canonical version of the path
    /// @details The canonical version of a path removes all symbolic links and "..", "." 
    /// @note This requires the path to exist
    static GString CanonicalPath(const char* path);

    /// @brief Return the normalized version of the path
    /// @details Unlike CanonicalPath, this does not require a path to exist
    static GString NormalizedPath(const char* path);

    /// @brief Return absolute path relative to the current working directory
    static GString AbsolutePath(const char* relPath);

    /// @brief Return absolute path from relative path, assuming the given root (working) directory
    static GString AbsolutePath(const char* rootDir, const char* relPath);

    /// @brief Return relative path from one path to another
    static GString RelativePath(const char* rootDir, const char* absPath);

    /// @}

private:

    friend class GDir;
    friend class GFile;

    static std::regex s_emptyLineRegex; ///< Regex representing an empty line
    static std::regex s_newLineRegex; ///< Regex representing a new line
    static std::regex s_spaceOrTabRegex; ///< Regex representing a space or a tab

};


} // End rev namespace

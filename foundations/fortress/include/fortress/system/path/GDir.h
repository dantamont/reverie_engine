#pragma once

// std
#include <filesystem>
#include <regex>

// Public
#include "fortress/string/GStringView.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

/// @class GDir
/// @brief Class representing a directory path
/// @note Does not take ownership over given path. Keeps things performant
class GDir {
public:

    /// @details The current working directory is the directory to which all relative paths are in reference to
    /// @note Since the current working directory as used by std::filesystem is a dangerous global variable, which
    /// can change from other threads or applications, this is used instead
    static const GString& CurrentWorkingDir();

    /// @copydoc GDir::GetCurrentDirectory()
    static void SetCurrentWorkingDir(const GStringView& dir);

    GDir() = default;
    GDir(const GStringView& dirPath);
    ~GDir() = default;

    /// @brief Return the path of the directory
    const GStringView& path() const { return m_path; }

    /// @brief Return true if the path actually represents a directory
    bool isDirectory() const;

    /// @brief Return whether or not the directory exists
    bool exists() const;

    /// @brief Return the absolute path of the given path relative to this directory
    /// @details Does not clean up any "." or "..", so returned path is not canonical
    GString absolutePath(const char* relPath) const;

    /// @brief Returns the given path relative to this directory
    GString relativePath(const char* absPath) const;

    /// @brief Create the directory if it doesn't exist
    /// @return True if created
    bool create() const;

    /// @brief Remove the directory. Will only work if empty
    /// @return true if removed
    bool remove() const;

    /// @brief Remove the directory and all contents recursively
    /// @return The number of removed files and directories
    Uint64_t removeAll() const;

    /// @brief Search subdirectories and files in this directory for a file
    /// @param[in] recursive If true, use a recursive search
    bool containsFile(const GString& fileName, bool recursive, GString& outPath) const;

    /// @brief Search subdirectories and files in this directory for files satisfying the given search function
    /// @details Is recursive
    /// @param[in] searchFunction The function to use to compare file names. Returns true on a match
    std::vector<GString> getFiles(std::function<bool(const GString&)> searchFunction) const;

private:

    /// @brief Non-recursively search subdirectories and files in this directory for a file
    bool containsFile(const GString& fileName, GString& outPath) const;

    /// @brief Recursively search subdirectories and files in this directory for a file
    bool containsFileRecursive(const GString& fileName, GString& outPath) const;

    GStringView m_path{}; ///< The directory path

    static GString s_currentDirectoryPath; ///< The cached current working directory, in case the actual one is changed
};


} // End rev namespace

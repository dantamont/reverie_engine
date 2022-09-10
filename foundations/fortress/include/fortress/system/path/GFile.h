#pragma once

// std
#include <filesystem>
#include <regex>

// Public
#include "fortress/string/GStringView.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {

/// @class GFile
/// @brief Class for File I/O related helper functions
/// @note Does not take ownership of its path. This keeps things performant
class GFile {
public:

    enum OpenModeFlag {
        kRead = std::ios::in, ///< Open for read (input)
        kWrite = std::ios::out, ///< Open for write (output)
        kBinary = std::ios::binary, ///< Open in binary mode (which suppresses automatic conversions like "\r\n" to "\n"
        kClear = std::ios::trunc, ///< Truncates (discards) file content before write
        kAppend = std::ios::app, ///< Seek to the end of the file before write, i.e., write appends to file, and cannot overwrite previous contents
        kAtEnd = std::ios::ate, ///< Set stream to the end of file when opened, but can move around (seek)
    };
    MAKE_FLAGS(OpenModeFlag, OpenModeFlags)

    GFile() = default;
    GFile(const GStringView& filePath);
    ~GFile() = default;

    /// @brief Return the path of the file
    const GStringView& path() const { return m_path; }

    /// @brief Get the file extension
    /// @param[in] includePeriod if true, include period in retrieved extension
    GString extension(bool includePeriod = false) const;

    /// @brief Get the path of the file with the extension replaced
    GString replaceExtension(const char* ext) const;

    /// @brief Get the name of the file
    /// @param[in] extension if true, return with extension
    /// @param[in] caseSensitive if false, make name lowercase
    GString getFileName(bool keepExtension = true, bool caseSensitive = false) const;

    /// @brief Get the size of the file (in bytes)
    Uint64_t getFileSizeBytes() const;

    /// @brief Return true if the path actually represents a file
    bool isFile() const;

    /// @brief Return whether or not the file exists
    bool exists() const;

    /// @brief Get the parent directory of the file
    GString getDirectory() const;

    /// @brief Get the absolute parent directory of the file
    GString getAbsoluteDirectory() const;

    /// @brief Create the file if it does not exist
    /// @note This will not create directories unless specified
    /// @return Whether or not the file was created
    bool create(bool createDirectories = false);

    /// @brief Delete the file
    /// @return Whether or not the file was deleted
    bool remove();

    /// @brief Get contents of a file as a string
    /// @note tellg may not work on all platforms
    /// @note Does not recognize carriage returns, resulting in nonsense data at the end of strings
    /// @see https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c
    /// @see https://stackoverflow.com/questions/524591/performance-of-creating-a-c-stdstring-from-an-input-iterator/524843#524843
    GString read() const;

    /// @brief Get the lines from a file
    std::vector<GString> readLines() const;

    /// @brief Write to the file
    /// @todo Untested.
    void writeLines(std::vector<GString>& lines);

private:

    GStringView m_path; ///< The path to the file
};


} // End rev namespace

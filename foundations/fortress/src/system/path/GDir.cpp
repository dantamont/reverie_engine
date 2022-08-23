#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"
#include "fortress/types/GStringView.h"
#include "fortress/types/GString.h"

namespace rev {

const GString& GDir::CurrentWorkingDir()
{
    std::error_code err;
    GString freshCurrentPath = std::filesystem::current_path(err).generic_string();
    assert(!err && err.message().c_str());
    assert(freshCurrentPath == s_currentDirectoryPath);
    return s_currentDirectoryPath;
}

void GDir::SetCurrentWorkingDir(const GStringView& dir)
{
    std::error_code err;
    std::filesystem::current_path(dir.c_str(), err);
    s_currentDirectoryPath = dir;

    if (err) {
        std::cerr << "Failed to set current directory" << err.message() << '\n';
    }
    assert(!err);
}

GDir::GDir(const GStringView& dirPath):
    m_path(dirPath)
{
}

bool GDir::isDirectory() const
{
    return std::filesystem::is_directory(m_path.c_str());
}

bool GDir::exists() const
{
    return GPath::Exists(m_path);
}

GString GDir::absolutePath(const char* relPath) const
{
#ifdef DEBUG_MODE
    assert(GPath::IsRelative(relPath) && "Error, absolute path given as argument");
#endif
    std::filesystem::path outPath = std::filesystem::path(m_path.c_str()) / relPath;
    GString absPathStr = outPath.generic_string();
    return absPathStr;
}

GString GDir::relativePath(const char* absPath) const
{
    return std::filesystem::relative(absPath, std::filesystem::path(m_path.c_str())).generic_string();
}

bool GDir::create() const
{
    return std::filesystem::create_directories(m_path.c_str());
}

bool GDir::remove() const
{
    return std::filesystem::remove(m_path.c_str());
}

Uint64_t GDir::removeAll() const
{
    return std::filesystem::remove_all(m_path.c_str());
}

bool GDir::containsFile(const GString& fileName, bool recursive, GString& outFilePath) const
{
    if (recursive) {
        return containsFileRecursive(fileName, outFilePath);
    }
    else {
        return containsFile(fileName, outFilePath);
    }
}

bool GDir::containsFile(const GString& fileName, GString& outFilePath) const
{
    bool found = false;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_path.c_str())) {
        if (!entry.is_regular_file()) {
            continue;
        }

        GString entryFileName = entry.path().filename().generic_string();

        // Iterate through directories to find file
        if (fileName == entryFileName) {
            found = true;
            outFilePath = entry.path().generic_string();
            break;
        }
    }
    return found;
}

bool GDir::containsFileRecursive(const GString& fileName, GString& outFilePath) const
{
    bool found = false;
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_path.c_str())) {
        if (!entry.is_regular_file()) {
            continue;
        }

        GString entryFileName = entry.path().filename().generic_string();

        // Iterate through directories to find file
        if (fileName == entryFileName) {
            found = true;
            outFilePath = entry.path().generic_string();
            break;
        }
    }
    return found;
}

GString GDir::s_currentDirectoryPath = std::filesystem::current_path().generic_string();

} // End namespaces

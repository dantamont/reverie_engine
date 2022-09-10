#include "fortress/system/path/GPath.h"
#include "fortress/system/path/GDir.h"
#include "fortress/string/GStringView.h"

namespace rev {

char GPath::Separator()
{
    return std::filesystem::path::preferred_separator;
}

bool GPath::Exists(const char* path)
{
    return std::filesystem::exists(std::filesystem::path(path));
}

bool GPath::IsAbsolute(const char* path)
{
    std::filesystem::path filePath(path);
    return filePath.is_absolute();
}

bool GPath::IsRelative(const char* path)
{
    std::filesystem::path filePath(path);
    return filePath.is_relative();
}

GString GPath::CanonicalPath(const char* path)
{
    std::error_code ec;
    std::string canonicalPath = std::filesystem::canonical(path, ec).generic_string();
#ifdef DEBUG_MODE
    if (ec) {
        GString err = ec.message().c_str();
        std::cerr << err;
    }
    assert(!ec);
#endif
    return canonicalPath;
}

GString GPath::NormalizedPath(const char* path)
{
    std::filesystem::path absPath(GPath::AbsolutePath(path).c_str());
    std::filesystem::path::iterator it = absPath.begin();
    std::filesystem::path result = *it++;

    // Get canonical version of the existing part
    for (; exists(result / *it) && it != absPath.end(); ++it) {
        result /= *it;
    }
    result = std::filesystem::canonical(result);

    // For the rest remove ".." and "." in a path with no symlinks
    for (; it != absPath.end(); ++it) {
        // Just move back on ../
        if (*it == "..") {
            result = result.parent_path();
        }
        // Ignore "."
        else if (*it != ".") {
            // Just cat other path entries
            result /= *it;
        }
    }

    // Make sure resulting separators are consistent
    return result.generic_string();
}

GString GPath::AbsolutePath(const char* relPath)
{
    return std::filesystem::absolute(std::filesystem::path(relPath)).generic_string();
}

GString GPath::AbsolutePath(const char* root, const char* relPath)
{
    GDir rootDir(root);
    GString absPath = rootDir.absolutePath(relPath);
    absPath = CanonicalPath(absPath);
    return absPath;
}

GString GPath::RelativePath(const char* root, const char* absPath)
{
    GDir rootDir(root);
    GString relPath = rootDir.relativePath(absPath);
    return relPath;
}

std::regex GPath::s_emptyLineRegex("\\s+");
std::regex GPath::s_newLineRegex("(\\r?\\n)+");
std::regex GPath::s_spaceOrTabRegex("[\\s\\t]+");

} // End namespaces
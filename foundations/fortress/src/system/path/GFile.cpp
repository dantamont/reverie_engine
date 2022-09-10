#include "fortress/system/path/GPath.h"
#include "fortress/string/GStringView.h"
#include "fortress/string/GString.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GDir.h"
#include "fortress/streams/GFileStream.h"

#include <iostream>>
#include <fstream>

namespace rev {

GFile::GFile(const GStringView& filePath) :
    m_path(filePath)
{
}

GString GFile::extension(bool includePeriod) const
{
    GString ext = std::filesystem::path(m_path.c_str()).extension().generic_string();
    if (!includePeriod) {
        ext = ext.subStr(1);
    }
    return ext;
}

GString GFile::replaceExtension(const char* ext) const
{
    return std::filesystem::path(m_path.c_str()).replace_extension(ext).generic_string();
}

GString GFile::getFileName(bool keepExtension, bool caseSensitive) const
{
    // Convert filepath to a case-insensitive name
    GString filename = std::filesystem::path(m_path.c_str()).filename().generic_string();
    if (!keepExtension) {
        filename.replace(extension(true).c_str(), "");
    }

    if (!caseSensitive) {
        filename.toLower();
    }
    return filename;
}

Uint64_t GFile::getFileSizeBytes() const
{
    std::error_code err;
    Uint64_t size = std::filesystem::file_size(m_path.c_str(), err);
    if (err) {
        std::cerr << err.message() << '\n';
    }
    assert(!err && "Failed to retrieve file size");
    return size;
}

bool GFile::isFile() const
{
    return std::filesystem::is_regular_file(m_path.c_str());
}

bool GFile::exists() const
{
    return GPath::Exists(m_path);
}

GString GFile::getDirectory() const
{
    return std::filesystem::path(m_path.c_str()).parent_path().generic_string();
}

GString GFile::getAbsoluteDirectory() const
{
    return GPath::AbsolutePath(getDirectory());
}

bool GFile::create(bool createDirectories)
{
    if (GPath::Exists(m_path)) {
        return false;
    }

    // Create directory for the file if it does not exist
    GFile myFile(m_path);
    GString myDirStr = myFile.getDirectory();
    GDir myDir(myDirStr);
    if (!myDir.exists()) {
        if (createDirectories) {
            myDir.create();
        }
        else {
            return false;
        }
    }

    // Create the actual file
    std::ofstream outFile(m_path);
    outFile.close();

    return true;
}

bool GFile::remove()
{
    return std::filesystem::remove(m_path.c_str());
}


GString GFile::read() const
{
    // Create file stream, opening at the end
    OpenModeFlags flags = OpenModeFlag::kAtEnd;
    std::ifstream ifs(m_path, flags);

    // Get position at the end of the stream for file size
    /// @note Could (maybe) also use std::filesystem::file_size, but might also not be portable
    std::ifstream::pos_type fileSize = ifs.tellg();
    assert(fileSize >= 0, "Failed to read file");

    // Return to the beginning of the stream
    ifs.seekg(0, std::ios::beg);

    GString outStr((char*)nullptr, fileSize);
    ifs.read(outStr.begin(), fileSize);

    return outStr;
}


std::vector<GString> GFile::readLines() const
{
    std::vector<GString> lines;
    std::ifstream file(m_path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }

    return lines;
}

void GFile::writeLines(std::vector<GString>& lines)
{
    FileStream myStream(m_path);
    myStream.open(FileAccessMode::kWrite | FileAccessMode::kAppend);
    const char newLine = '\n';
    for(const GString& line: lines) {
        myStream.write(line.c_str(), line.length());
        myStream.write(&newLine, 1);
    }
}


} // End namespaces
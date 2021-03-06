/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_FILE_STREAM_H
#define GB_FILE_STREAM_H

// standard
#include <array>

// Internal
#include "../containers/GString.h"
#include "../containers/GFlags.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned long long uint64;

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
enum class FileAccessMode {
    kRead = std::ios_base::in,
    kWrite = std::ios_base::out,
    kBinary = std::ios_base::binary,
    kAtEnd = std::ios_base::ate, // The output (write) position starts at the end of the file, but output operations can happen in other places
    kAppend = std::ios_base::app, // ALL output (write) operations happen at the end of the file, appending to existing contents
    kTruncate = std::ios_base::trunc // Discard any contents in the file that existed before opened
};
MAKE_BITWISE(FileAccessMode);
MAKE_FLAGS(FileAccessMode, FileAccessModes);

/// @brief Encapsulates a file stream
class FileStream{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructors
    /// @{

    FileStream(const GString& filePath);
    ~FileStream();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Open the file represented by the stream
    /// @param[in] mode to open filepath with. Maps to "wb" for write binary, "rb" for read binary
    inline bool open(FileAccessModes mode) {
        GString modeStr = getAccessModeStr(mode);
        m_fileStream = fopen(m_filePath.c_str(), modeStr.c_str());
        if (!m_fileStream) {
            throw("Error, failed to open file " + m_filePath);
        }
        else {
            // Opened filestream, so position to end of file if AtEnd flag set
            if (mode.testFlag(FileAccessMode::kAtEnd)) {
                int failed = fseek(m_fileStream, 0, SEEK_END); // Returns zero on success
                if (failed) {
                    // Close if failed to find end of file
                    close();
                }
            }
        }
        return m_fileStream;
    }

    /// @brief close the file represented by the stream
    /// @return Whether or not the file stream was successfully closed
    inline bool close() {
        if (!m_fileStream) {
            throw("Error, file stream not opened");
        }

        int closed = fclose(m_fileStream);
        bool closedStream = (closed == 0);
        if (closedStream) {
            m_fileStream = nullptr;
        }

        return closedStream;
    }

    /// @brief Binary write to file
    /// @return Whether the operation was a success or not
    template<typename T>
    inline bool write(const std::vector<T>& vec) const {
        uint64 size = (uint64)vec.size();
        return write(vec.data(), size);
    }

    template<typename T>
    inline bool write(const T* data, const uint64& count) const{
        // Write count to file
        size_t result = lendian_write(&count, sizeof(uint64), 1, m_fileStream);
        if (result != 1) {
            throw("Error, failed to write count to stream");
            return false;
        }

        // Write actual data contents to file
        result = lendian_write(data, sizeof(T), count, m_fileStream);
        bool success = result == (size_t)count;
        if (!success) {
            throw("Error, failed to write data to stream");
        }

        return success;
    }

    /// @brief Read from file stream
    template<typename T>
    inline bool read(std::vector<T>& outVec) const {
        // Load in count from file        
        uint64 count;
        readCount(count);
        outVec.resize(count);

        // Read data from file
        return readData(outVec.data(), count);
    }

    /// @brief Read from file stream
    /// @details Assumes that data is prepended by a count
    template<typename T>
    inline bool read(T* outData, uint64& outCount) const {
        // Read count from file
        readCount(outCount);

        // Read data from file
        return readData(outData, outCount);
    }


    /// @brief Reads a llong from the file stream, assuming it represents a count
    inline bool readCount(uint64& outCount) const {
        // Load in count from file
        size_t check = fread(&outCount, sizeof(uint64), 1, m_fileStream);
        if (check != 1) {
            bool err = hasError();
            bool eof = reachedEOF();
            Q_UNUSED(eof);
            throw("Error, failed to read from file");
            return err;
        }
        return true;
    }

    /// @brief Reads a set of data from the file stream
    template<typename T>
    inline bool readData(T* outData, uint64 count) const {
        // Read data from file
        size_t check = fread(outData, sizeof(T), (size_t)count, m_fileStream);
        if (check != (size_t)count) {
            throw("Error, failed to load in data from file");
            return false;
        }
        return true;
    }

    /// @brief Checks whether or not the stream has an error
    bool hasError() const {
        if (!m_fileStream) {
            return false;
        }

        if (ferror(m_fileStream)) {
            return true;
        }
        else {
            return false;
        }
    }

    /// @brief Checks whether or not the stream has reached end of file
    bool reachedEOF() const {
        if (!m_fileStream) {
            return false;
        }

        if (feof(m_fileStream)) {
            return true;
        }
        else {
            return false;
        }
    }

    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Replicate fstream behavior
    // See: https://en.cppreference.com/w/cpp/io/basic_filebuf/open
    // See: https://comp.lang.cpp.moderated.narkive.com/vJEyIdHF/equivalent-of-c-the-iostream-for-fopen-filename-a
    GString getAccessModeStr(FileAccessModes  modes) {
        GString modeStr;
        if (modes.testFlag(FileAccessMode::kRead) && modes.testFlag(FileAccessMode::kWrite)) {
            if (modes.testFlag(FileAccessMode::kTruncate)) {
                modeStr = "w+";
            }
            else if (modes.testFlag(FileAccessMode::kAppend)) {
                modeStr = "a+";
            }
            else {
                modeStr = "r+";
            }
        }
        else if (modes.testFlag(FileAccessMode::kRead)) {
            modeStr = "r";
            if (modes.testFlag(FileAccessMode::kAppend)) {
                modeStr = "a+";
            }
        }
        else if (modes.testFlag(FileAccessMode::kWrite)) {
            modeStr = "w";
            if (modes.testFlag(FileAccessMode::kAppend)) {
                modeStr = "a";
            }
            // Don't do anything for truncate
        }
        else if ((size_t)modes == (size_t)FileAccessMode::kAppend) {
            modeStr = "a";
        }
        else {
            throw("Error, must specify read or write flag");
        }

        // Append binary flag
        if (modes.testFlag(FileAccessMode::kBinary)) {
            modeStr += "b";
        }
        
        return modeStr;
    }

    /// @brief Write to file, independent of system endianness
    /// @details Stores data in little endian format
    // TODO: Perform write as many times as needed for nmemb > size_t::max_size
    static uint64 lendian_write(const void* ptr, size_t size, const uint64& count, FILE* stream);

    /// @brief Swaps between endianness to perform fwrite
    // See: https://stackoverflow.com/questions/40193322/how-to-fwrite-and-fread-endianness-independant-integers-such-that-i-can-fwrite
    // TODO: Perform write as many times as needed for nmemb > size_t::max_size
    static uint64 endian_swap_fwrite(const void* ptr, size_t size, const uint64& count, FILE* stream);


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the internal C-style file stream
    FILE* m_fileStream = nullptr;

    /// @brief Filepath for the stream
    GString m_filePath;

    /// @}
};

        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////     
} // End namespaces

#endif
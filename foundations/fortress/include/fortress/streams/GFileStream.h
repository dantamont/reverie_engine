#pragma once

// standard
#include <array>
#include <stdio.h>

// Internal
#include "fortress/GGlobal.h"
#include "fortress/encoding/binary/GEndianConverter.h"
#include "fortress/system/GSystemPlatform.h"
#include "fortress/string/GString.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

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
class FileStream {
public:
    /// @name Static
    /// @{

    /// @}

    /// @name Constructors/Destructors
    /// @{

    FileStream(const GString& filePath);
    ~FileStream();

    /// @}

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
        Uint64_t size = (Uint64_t)vec.size();
        return write(vec.data(), size);
    }

    template<typename T>
    inline bool write(const T* data, const Uint64_t& count, bool prependCount = true) const {
        // Write count to file
        size_t result;
        if (prependCount) {
            result = lendian_write(&count, 1, m_fileStream);
            if (result != 1) {
                throw("Error, failed to write count to stream");
                return false;
            }
        }

        // Write actual data contents to file
        result = lendian_write(data, count, m_fileStream);
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
        Uint64_t count;
        readCount(count);
        outVec.resize(count);

        // Read data from file
        return readData(outVec.data(), count);
    }

    /// @brief Read from file stream
    /// @details Assumes that data is prepended by a count
    template<typename T>
    inline bool read(T* outData, Uint64_t& outCount) const {
        // Read count from file
        readCount(outCount);

        // Read data from file
        return readData(outData, outCount);
    }


    /// @brief Reads a llong from the file stream, assuming it represents a count
    inline bool readCount(Uint64_t& outCount) const {
        // Load in count from file
        size_t check = fread(&outCount, sizeof(Uint64_t), 1, m_fileStream);
        if (check != 1) {
            bool err = hasError();
            bool eof = reachedEOF();
            G_UNUSED(eof);
            throw("Error, failed to read from file");
            return err;
        }
        return true;
    }

    /// @brief Reads a set of data from the file stream
    template<typename T>
    inline bool readData(T* outData, Uint64_t count) const {
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
    /// @param[in] ptr     pointer to the array of elements to be written
    /// @param[in] count   number of elements, each with a size of sizeof(T) bytes
    /// @param[in] stream  pointer to a FILE object that specifies an output stream
    /// @todo Perform write as many times as needed for nmemb > size_t::max_size
    template<typename T>
    static Uint64_t lendian_write(const T* ptr, const Uint64_t& count, FILE* stream)
    {
        static const bool isLittleEndian = SystemMonitor::GetEndianness() == SystemMonitor::SystemEndianness::kLittle;
        constexpr Uint64_t sizeInBytesOfType = sizeof(T);
        if (isLittleEndian)
        {
            // Little endian machine, use fwrite directly

            return fwrite(ptr, sizeInBytesOfType, count, stream);
        }
        else
        {
            // Big endian machine, pre-process first
            return endian_swap_fwrite(ptr, count, stream);
        }
    }

    /// @brief Swaps between endianness to perform fwrite
    /// @note This will not work on non-trivial types, as the word-length for integers
    /// can't be automatically deduced. Those must be serialized manually.
    /// @see https://stackoverflow.com/questions/40193322/how-to-fwrite-and-fread-endianness-independant-integers-such-that-i-can-fwrite
    /// @todo Perform write as many times as needed for nmemb > size_t::max_size
    template<typename T>
    static Uint64_t endian_swap_fwrite(const T* ptr, const Uint64_t& count, FILE* stream)
    {
        Uint8_t* bufferDst;
        if constexpr (!std::is_integral_v<T>) {
            const Uint8_t* bufferSrc = reinterpret_cast<const Uint8_t*>(ptr);
            bufferDst = EndianConverter::SwapEndianness(bufferSrc, count);
        }
        else {
            bufferDst = EndianConverter::SwapEndianness(ptr, count);
        }

        // Perform fwrite
        constexpr Uint64_t sizeInBytesOfType = sizeof(T);
        Uint64_t result;

        // Need to convert to a byte array if type is not integral.
        result = fwrite(bufferDst, sizeInBytesOfType, count, stream);

        // Delete intermediate buffer
        delete[] bufferDst;
        return result;
    }


    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Pointer to the internal C-style file stream
    FILE* m_fileStream = nullptr;

    /// @brief Filepath for the stream
    GString m_filePath;

    /// @}
};


} // End namespaces

#ifndef GB_BUFFER_QUEUE_H
#define GB_BUFFER_QUEUE_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Internal 
#include "GbGLBuffer.h"
#include <mutex>
//#include "GbShaderStorageBuffer.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class ShaderStorageBuffer;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct BufferCommand
/// @brief Contains data to update a buffer
struct BufferCommand {

    BufferCommand(const void* data, unsigned long offset, unsigned long sizeInBytes);
    ~BufferCommand();

    /// @brief The data to use to update the buffer
    const void* m_data = nullptr;

    /// @brief The offset in the buffer to insert the data into (in bytes)
    unsigned long m_offset = 1;

    /// @brief The length of the data to insert (in bytes)
    unsigned long m_sizeInBytes = 0;
};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class BufferQueue
/// @brief Class representing a set of GL buffers to be updated
/// @details Used to implement double or triple buffering
// Triple Buffering:
// Three buffers, A, B, C
// Display A, while drawing to B
// Swap, to display B, now drawing into C since cannot draw into A until swap is done
// Display C (swapping B into C), while drawing into A, since it is free now
// Swap A and C to...
// Display A, starting the process over
template<typename BufferType, size_t N>
class BufferQueue{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    /// @}

    BufferQueue():
        m_initialized(false)
    {
    }

    /// @brief Initialize queue however one would a buffer
    /// @note Variadic template is used to avoid having to modify this if buffer initialization changes
    template<typename ...BufferArgs>
    BufferQueue(BufferArgs&&... args) {
        static_assert(N == 2 || N == 3, "Error, buffer queue only supports two or three buffers");

        for (size_t i = 0; i < N; i++) {
            m_buffers[i] = BufferType(std::forward<BufferArgs>(args)...);
        }

        // Initialize local data
        m_data = new char[m_buffers[0].byteSize()];
        m_initialized = true;
    }

    ~BufferQueue() {
        delete[] m_data;
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    //std::array<BufferType, N>& buffers() { return m_buffers; }
    BufferType& writeBuffer() { return m_buffers[m_writeBuffer]; }
    BufferType& readBuffer() { return m_buffers[m_readBuffer]; }

    template<typename T>
    T* data() { 
        return (T*)m_data;
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Clear the contained buffers
    void clear() {
        std::unique_lock lock(m_mutex);
        if (m_initialized) {
            // Clear OpenGL buffers
            for (BufferType& buffer : m_buffers) {
                buffer.clear();
            }

            // Clear local buffer
            memset(m_data, 0, m_buffers[0].byteSize());
        }
    }

    /// @brief Set bind point of each of the buffers
    void setBindPoints(size_t bindPoint) {
        for (BufferType& buffer : m_buffers) {
            buffer.setBindPoint(bindPoint);
        }
    }

    void setNames(const GString& name) {
        for (BufferType& buffer : m_buffers) {
            buffer.setName(name);
        }
    }

    /// @brief Add a buffer command to the queue
    void queueUpdate(const void* data, unsigned long offset, unsigned long sizeInBytes) {
        std::unique_lock lock(m_mutex);
        Vec::EmplaceBack(m_incomingCommands, data, offset, sizeInBytes);
    }

    /// @brief Add a buffer command to the queue, assuming a specific data type
    /// @param[in] countOffset the offset at which to insert data into the buffer, in units of sizeof(T)
    template<typename T>
    void queueUpdate(const T& data, unsigned long countOffset) {
        std::unique_lock lock(m_mutex);
        Vec::EmplaceBack(m_incomingCommands, &data, countOffset * sizeof(T), sizeof(T));
    }

    /// @brief Swap buffers for triple buffering
    // Display A, while drawing into B.In my case, "drawing" means writing updated data into the buffer.
    // Swap, to display B, now writing into C since cannot write into A until swap is done
    // Display C(swapping B into C), while writing into A, since it is free now.
    // Swap A and C to Display A, bring us back to the start of the process
    void swapBuffers() {
        std::unique_lock lock(m_mutex);
        if constexpr (N == 2) {
            m_writeBuffer = m_readBuffer;
            m_readBuffer = 1 - m_readBuffer;
        }
        else {
            size_t bufferSize = m_buffers[0].byteSize();
            if (m_readBuffer == 0) {
                if (m_writeBuffer != 1) {
                    throw("Error afoot");
                }
                // Perform actual data copy into other buffers
                //m_buffers[1].copyInto(m_buffers[2]);
                m_buffers[2].subData(m_data, 0, bufferSize);

                m_readBuffer = 1; // Swap to read from previous write buffer
                m_writeBuffer = 2; // Make previously available buffer into write buffer, since 0 is swapping
            }
            else if (m_readBuffer == 1) {
                if (m_writeBuffer != 2) {
                    throw("Error afoot");
                }
                // Perform actual data copy
                //m_buffers[2].copyInto(m_buffers[0]);
                m_buffers[0].subData(m_data, 0, bufferSize);

                m_readBuffer = 2; // Swap to read from previous write buffer
                m_writeBuffer = 0; // Make previously available buffer into write buffer, since 0 is swapping
            }
            else if (m_readBuffer == 2) {
                if (m_writeBuffer != 0) {
                    throw("Error afoot");
                }
                // Perform actual data copy
                //m_buffers[0].copyInto(m_buffers[1]);
                m_buffers[1].subData(m_data, 0, bufferSize);

                m_readBuffer = 0; // Swap to read from previous write buffer                
                m_writeBuffer = 1; // Make previously available buffer into write buffer, since 0 is swapping
            }
            else {
                throw("Unreachable");
            }
        }
    }

    /// @brief Flush all pending updates into the buffer, returning true if there were updates
    bool flushBuffer() {
        m_mutex.lock();
        bool updated = m_incomingCommands.size();
        if (!updated) {
            m_mutex.unlock();
            return updated;
        }

        m_incomingCommands.swap(m_commands);
        m_incomingCommands.clear();
        m_mutex.unlock();

        // Map buffer
        //BufferType& buffer = m_buffers[m_writeBuffer];
        //char* bufferData = (char*)buffer.map(GL::RangeBufferAccessFlag::kWrite);

        // Update local buffer contents
        for (const BufferCommand& command : m_commands) {
            //memcpy(bufferData + command.m_offset, command.m_data, command.m_sizeInBytes);
            //buffer.subData(command.m_data, command.m_offset, command.m_sizeInBytes);
            memcpy(m_data + command.m_offset, command.m_data, command.m_sizeInBytes);
        }

        // Copy contents into write buffer
        if (m_commands.size()) {
            writeBuffer().subData(m_data, 0, writeBuffer().byteSize());
        }

        // Clear commands
        m_commands.clear();

        // Unmap buffer
        //buffer.unmap();

        return updated;
    }

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    bool m_initialized = false;

    /// @brief the buffer currently being written into
    size_t m_readBuffer = 0;
    size_t m_writeBuffer = 1;

    std::mutex m_mutex;

    /// @brief Local cache of buffer data to copy into write buffer
    char* m_data = nullptr;

    /// @brief Received data
    std::vector<BufferCommand> m_incomingCommands;

    /// @brief Data used to update the buffer
    std::vector<BufferCommand> m_commands;

    /// @brief The buffers used for updating the queue
    std::array<BufferType, N> m_buffers;
    
    /// @}

};
typedef BufferQueue<ShaderStorageBuffer, 3> ShaderStorageBufferQueue;


    /////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
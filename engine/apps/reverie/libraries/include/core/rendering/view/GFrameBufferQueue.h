#pragma once

// Internal 
#include "GFrameBuffer.h"

namespace rev {

/// @class FrameBufferQueue
/// @brief Class representing a set of FBOs to be updated
/// @details Used to implement double or triple buffering
// Triple Buffering:
// Three buffers, A, B, C
// Display A, while drawing to B
// Swap, to display B, now drawing into C since cannot draw into A until swap is done
// Display C (swapping B into C), while drawing into A, since it is free now
// Swap A and C to...
// Display A, starting the process over
template<size_t N>
class FrameBufferQueue{
public:
    /// @name Constructors/Destructor
    /// @{

    FrameBufferQueue() = default;
    FrameBufferQueue(const FrameBufferQueue& other) = delete;

    /// @brief Initialize queue however one would a buffer
    /// @note Variadic template is used to avoid having to modify this if buffer initialization changes
    template<typename ...BufferArgs>
    FrameBufferQueue(BufferArgs&&... args):
        m_frameBuffers([&] {
            if constexpr (N == 2) {
                // N == 2
                return std::array<std::unique_ptr<FrameBuffer>, N>{ { std::make_unique<FrameBuffer>(std::forward<BufferArgs>(args)...), std::make_unique<FrameBuffer>(std::forward<BufferArgs>(args)...) } };
            }
            else {
                // N == 3
                return std::array<std::unique_ptr<FrameBuffer>, N>{ { std::make_unique<FrameBuffer>(std::forward<BufferArgs>(args)...), std::make_unique<FrameBuffer>(std::forward<BufferArgs>(args)...), std::make_unique<FrameBuffer>(std::forward<BufferArgs>(args)...) } };
            }
        }()
        )// Immediate evaluation of lambda, lambdas are constexpr by default as of N4487 and P0170 of open-standard https://stackoverflow.com/questions/41011900/equivalent-ternary-operator-for-constexpr-if
    {
        static_assert(N == 2 || N == 3, "Error, framebuffer queue only supports two or three buffers");
    }

    ~FrameBufferQueue() {
    }

    /// @}

    /// @name Properties
    /// @{

    FrameBuffer& writeBuffer() { return *m_frameBuffers[m_writeBuffer]; }
    const FrameBuffer& writeBuffer() const { return *m_frameBuffers[m_writeBuffer]; }

    FrameBuffer& readBuffer() { return *m_frameBuffers[m_readBuffer]; }
    const FrameBuffer& readBuffer() const { return *m_frameBuffers[m_readBuffer]; }

    std::array<std::unique_ptr<FrameBuffer>, N>& frameBuffers() { return m_frameBuffers; }


    /// @}

    /// @name Operators
    /// @{

    FrameBufferQueue& operator=(const FrameBufferQueue& other) = delete;

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Swap buffers for triple buffering
    // Display A, while drawing into B.In my case, "drawing" means writing updated data into the buffer.
    // Swap, to display B, now writing into C since cannot write into A until swap is done
    // Display C(swapping B into C), while writing into A, since it is free now.
    // Swap A and C to Display A, bring us back to the start of the process
    void swapBuffers() {
        //std::unique_lock lock(m_mutex);
        if constexpr (N == 2) {
            m_writeBuffer = m_readBuffer;
            m_readBuffer = 1 - m_readBuffer;
        }
        else {
            size_t bufferSize = m_frameBuffers[0].byteSize();
            if (m_readBuffer == 0) {
#ifdef DEBUG_MODE
                if (m_writeBuffer != 1) {
                    Logger::Throw("Error afoot");
                }
#endif
                m_readBuffer = 1; // Swap to read from previous write buffer
                m_writeBuffer = 2; // Make previously available buffer into write buffer, since 0 is swapping
            }
            else if (m_readBuffer == 1) {
#ifdef DEBUG_MODE
                if (m_writeBuffer != 2) {
                    Logger::Throw("Error afoot");
                }
#endif
                m_readBuffer = 2; // Swap to read from previous write buffer
                m_writeBuffer = 0; // Make previously available buffer into write buffer, since 0 is swapping
            }
            else if (m_readBuffer == 2) {
#ifdef DEBUG_MODE
                if (m_writeBuffer != 0) {
                    Logger::Throw("Error afoot");
                }
#endif
                m_readBuffer = 0; // Swap to read from previous write buffer                
                m_writeBuffer = 1; // Make previously available buffer into write buffer, since 0 is swapping
            }
        }
    }

    /// @}


protected:
    /// @name Protected members
    /// @{

    //bool m_initialized = false;

    /// @brief the buffer currently being written into
    size_t m_readBuffer = 0;
    size_t m_writeBuffer = 1;

    //std::mutex m_mutex;

    /// @brief The framebuffers being updated and read from
    std::array<std::unique_ptr<FrameBuffer>, N> m_frameBuffers;
    
    /// @}

};

typedef FrameBufferQueue<2> PingPongFrameBuffers;

} // End namespaces

#include "core/rendering/buffers/GBufferQueue.h"

// QT

// Internal
#include "core/rendering/renderer/GRenderContext.h"
#include "fortress/system/memory/GPointerTypes.h"
#include <QOpenGLFunctions_4_4_Core> 

namespace rev {

BufferCommand::BufferCommand(const void * data, unsigned long offset, unsigned long sizeInBytes):
    m_data(data),
    m_offset(offset),
    m_sizeInBytes(sizeInBytes)
{
}

BufferCommand::~BufferCommand()
{
}

// End namespacing
}
#include "fortress/process/GProcess.h"
#include "fortress/process/GProcessQueue.h"

namespace rev {

Process::Process(ProcessQueue* queue) :
    ProcessInterface(queue)
{
    m_sortingLayer = queue->sortingLayers().getLayer(SortingLayer::s_defaultSortingLayer).id();
}

Process::Process(Uint32_t sortLayerId, ProcessQueue* queue) :
    ProcessInterface(queue),
    m_sortingLayer(sortLayerId)
{
}

Process::~Process()
{
}

ProcessQueue* Process::queue() const
{
    return static_cast<ProcessQueue*>(m_processQueue);
}

} // End namespaces
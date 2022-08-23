#pragma once

#include <limits>

// Internal
#include "fortress/containers/GSortingLayer.h"
#include "fortress/process/GProcessInterface.h"
#include "fortress/process/GProcessQueueInterface.h"
#include "fortress/types/GSizedTypes.h"

namespace rev {

class Process;
class ProcessQueue;

/// @class Process
/// @brief Represents a thread-friendly process
class Process: public ProcessInterface {
public:
    /// @name Static
    /// @{
    /// @}

	/// @name Constructors/Destructor
	/// @{
    Process(ProcessQueue* queue);
    Process(Uint32_t sortLayerId, ProcessQueue* queue);
	virtual ~Process();
	/// @}

    /// @name Properties
    /// @{

    /// @brief The order of the process
    Uint32_t sortingLayerId() const { return m_sortingLayer; }
    void setSortingLayer(const SortingLayer& layer) { m_sortingLayer = layer.id(); }

    /// @}

protected:
    friend struct CompareBySortingLayer;
    friend class ProcessQueue;

    ProcessQueue* queue() const;

    /// @name Protected Members
    /// @{

    Uint32_t m_sortingLayer = std::numeric_limits<Uint32_t>::max(); ///< Sorting layer ID of the process

    /// @}
};

} // End rev namespaces
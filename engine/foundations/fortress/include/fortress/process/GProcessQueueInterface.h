#pragma once

namespace rev {

/// @brief ProcessQueueInterface class
class ProcessQueueInterface {
protected:

    /// @name Constructor
    /// @{
    /// @brief Make constructor/destructor protected so a child class cannot be deleted as the base interface type
    ProcessQueueInterface() {}
    ~ProcessQueueInterface() {}
    /// @}

public:

	/// @name Public Methods
	/// @{

    /// @brief To be called on destruction, clears all processes
    virtual void clearProcesses() = 0;

    /// @brief Attach a process to the process manager
    virtual WeakProcessInterfacePtr attachProcess(StrongProcessInterfacePtr process, bool initialize = false) = 0;

	/// @}

protected:
    /// @name Protected Methods
    /// @{
    /// @}

    /// @name Protected Members
    /// @{
    /// @}
};


} // End namespaces

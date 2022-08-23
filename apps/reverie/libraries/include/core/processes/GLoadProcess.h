#pragma once

// Internal
#include "fortress/process/GThreadedProcess.h"
#include "core/resource/GResource.h"

namespace rev {

class CoreEngine;
class ProcessManager;
class ImageResource;

/// @class LoadProcess
/// @brief Represents a thread process to load a resource
class LoadProcess: public ThreadedProcess {
public:
    /// @name Static
    /// @{

    /// @brief The number of load processes currently active
    static Uint32_t Count() { return s_loadProcessCount.load(); }

    /// @}

	/// @name Constructors/Destructor
	/// @{
    LoadProcess(const std::shared_ptr<ResourceHandle>& resourceHandle);
	~LoadProcess();
	/// @}

	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onLateUpdate(double deltaSec) override {}
    virtual void onPostUpdate(double deltaSec) override {}
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Load the requested resource
    /// @details Returns success status
    std::unique_ptr<Resource> loadTexture();
    std::unique_ptr<Resource> loadModel();
    std::unique_ptr<Resource> loadMesh();
    std::unique_ptr<Resource> loadCubeTexture();
    std::unique_ptr<Resource> loadMaterial();
    std::unique_ptr<Resource> loadAnimation();
    std::unique_ptr<Resource> loadShaderProgram();
    std::unique_ptr<Resource> loadPythonScript();
    std::unique_ptr<Resource> loadAudio();

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief The handle for the resource to be loaded by this process
    std::shared_ptr<ResourceHandle> m_resourceHandle;

    static std::atomic<Uint32_t> s_loadProcessCount; ///< The number of running or failed loadprocesses

    /// @}
};


} // End namespaces

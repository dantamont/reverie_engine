/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LOAD_PROCESS_H
#define GB_LOAD_PROCESS_H

// Qt

// Internal
#include "GThreadedProcess.h"
#include "../resource/GResource.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class ProcessManager;

class Image;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class LoadProcess
/// @brief Represents a thread process to load a resource
class LoadProcess: public ThreadedProcess {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    LoadProcess(CoreEngine* engine, const std::shared_ptr<ResourceHandle>& resourceHandle);
	~LoadProcess();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Functions to be overriden by process subclasses as needed
    virtual void onInit() override;
    virtual void onSuccess() override;
    virtual void onFail() override;
    virtual void onAbort() override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "LoadProcess"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::LoadProcess"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Load the requested resource
    /// @details Returns success status
    std::unique_ptr<Resource> loadImage();
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

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The handle for the resource to be loaded by this process
    std::shared_ptr<ResourceHandle> m_resourceHandle;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
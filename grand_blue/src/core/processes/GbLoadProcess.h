/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LOAD_PROCESS_H
#define GB_LOAD_PROCESS_H

// Qt

// Internal
#include "GbThreadedProcess.h"
#include "../resource/GbResource.h"

namespace Gb {
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
    virtual const char* namespaceName() const { return "Gb::LoadProcess"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Load the requested resource
    /// @details Returns success status
    std::shared_ptr<Image> loadImage();
    std::shared_ptr<Resource> loadTexture();
    std::shared_ptr<Resource> loadModel();
    std::shared_ptr<Resource> loadMesh();
    std::shared_ptr<Resource> loadCubeTexture();
    std::shared_ptr<Resource> loadMaterial();
    std::shared_ptr<Resource> loadAnimation();
    std::shared_ptr<Resource> loadShaderProgram();
    std::shared_ptr<Resource> loadPythonScript();
    std::shared_ptr<Resource> loadAudio();

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
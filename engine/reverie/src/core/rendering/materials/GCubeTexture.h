/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CUBE_TEXTURE_H
#define GB_CUBE_TEXTURE_H

// QT
#include <QOpenGLTexture>
#include <QString>

// Internal
#include "./GTexture.h"
#include "../../containers/GColor.h"
#include "../models/GModel.h"
#include "../GGLFunctions.h"
#include "../shaders/GUniform.h"
#include "../../resource/GResource.h"
#include "../../geometry/GTransform.h"
#include "../../containers/GContainerExtensions.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CameraComponent;
class CoreEngine;
class ShaderProgram;
class Material;
class CubeMap;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct CubePaths
/// @brief Struct representing filepaths to the cubemap textures
struct CubePaths {
    CubePaths();
    CubePaths(
        const QString& directoryPath,
        const QString& right,
        const QString& left,
        const QString& top,
        const QString& bottom,
        const QString& front,
        const QString& back);
    ~CubePaths();

    QString path(const CubeMapFace& face) const;

    /// @brief Directory path for cubemap files
    QString m_directoryPath;

    /// @brief Names of each file at directory path, indexed by face
    std::array<GString, 6> m_fileNames;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CubeTexture
/// @brief Class representing texture for cubemap faces
class CubeTexture: public Texture{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static CubePaths loadPathsFromCubemapFile(const QString& filepath);
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const QString& filepath);

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CubeTexture(const CubePaths& imagePaths);
    CubeTexture(const QString& cubemapFilePath);

    /// @param[in] width the width of each side of the cube map
    /// @param[in] height the height of each side of the cube map
    CubeTexture(size_t width, size_t height, size_t depth = 1, bool isArray = false, TextureFormat internalFormat = TextureFormat::kRGBA8);
    ~CubeTexture();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{ 

    /// @brief Get the type of resource stored by this handle
    virtual ResourceType getResourceType() const override {
        return ResourceType::kCubeTexture;
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{ 
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{

    /// @property className
    /// @brief The name of this class
    virtual const char* className() const { return "CubeTexture"; }

    /// @property namespaceName
    /// @brief The full namespace for this class
    virtual const char* namespaceName() const { return "rev::GL::CubeTexture"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{ 
    
    friend class CubeMapComponent;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{ 

    /// @brief Load images from files
    void loadImages();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{ 

    /// @brief Paths to texture images
    CubePaths m_imagePaths;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
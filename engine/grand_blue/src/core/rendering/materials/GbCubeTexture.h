/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_CUBE_TEXTURE_H
#define GB_CUBE_TEXTURE_H

// QT
#include <QOpenGLTexture>
#include <QString>

// Internal
#include "../../containers/GbColor.h"
#include "../models/GbModel.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "../../resource/GbResource.h"
#include "../../geometry/GbTransform.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

class Image;
class CameraComponent;
class CoreEngine;
class ShaderProgram;
class Material;
class CubeMap;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @enum Face
/// @brief Faces of the cubemap
enum Face : int {
    kRight = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    kLeft,
    kTop,
    kBottom,
    kBack, // Cubemap uses Renderman convention, which is left-handed, so +z is back face
    kFront // and -z is front-face
};

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

    QString path(const Face& face) const;

    /// @brief Directory path for cubemap files
    QString m_directoryPath;

    /// @brief Names of each file at directory path
    std::unordered_map<Face, QString> m_fileNames;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @class CubeTexture
/// @brief Class representing texture for cubemap faces
class CubeTexture: public Resource{
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static CubePaths loadCubemapFile(const QString& filepath);
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine, const QString& filepath);

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CubeTexture(const CubePaths& imagePaths, unsigned int texUnit = 0);
    CubeTexture(const QString& cubemapFilePath, unsigned int texUnit = 0);
    ~CubeTexture();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{ 

    /// @brief Bind this texture
    inline void bind() {
        m_gl->glActiveTexture(GL_TEXTURE0 + m_textureUnit); // Activate the texture unit of this texture
        m_gl->glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID); // bind this texture
    }

    /// @brief unbind this texture
    inline void release() {
        m_gl->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    /// @brief Delete this texture from GL
    inline void remove() {
        glDeleteTextures(1, &m_textureID);
    }

    /// @brief Initialize the texture in OpenGl
    void initializeTexture();

    /// @brief Perform on removal of this texture resource
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{
    /** @property className
        @brief The name of this class
    */
    virtual const char* className() const { return "CubeTexture"; }

    /** @property namespaceName
        @brief The full namespace for this class
    */
    virtual const char* namespaceName() const { return "Gb::GL::CubeTexture"; }

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

    /// @brief Initialize the cube texture
    void initializeTexture(const CubePaths& imagePaths);

    /// @brief Initialize texture settings
    void initializeSettings();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{ 

    /// @brief Pointer to OpenGLFunctions 
    GL::OpenGLFunctions* m_gl;

    /// @brief Texture unit of the cubemap
    unsigned int m_textureUnit;

    /// @brief Texture ID of the cubemap
    unsigned int m_textureID;

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
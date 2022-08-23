#pragma once

// QT
#include <QOpenGLTexture>

// Internal
#include "./GTexture.h"
#include "core/rendering/models/GModel.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/shaders/GUniform.h"
#include "core/resource/GResource.h"

#include "fortress/containers/GColor.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/image/GImage.h"

namespace rev {

class CameraComponent;
class CoreEngine;
class ShaderProgram;
class Material;
class CubeMap;


/// @struct CubePaths
/// @brief Struct representing filepaths to the cubemap textures
struct CubePaths {
    CubePaths();
    CubePaths(
        const GString& directoryPath,
        const GString& right,
        const GString& left,
        const GString& top,
        const GString& bottom,
        const GString& front,
        const GString& back);
    ~CubePaths();

    GString path(const CubeMapFace& face) const;

    /// @brief Directory path for cubemap files
    GString m_directoryPath;

    /// @brief Names of each file at directory path, indexed by face
    std::array<GString, 6> m_fileNames;
};


/// @class CubeTexture
/// @brief Class representing texture for cubemap faces
class CubeTexture: public Texture{
public:
    /// @name Static
    /// @{

    static CubePaths loadPathsFromCubemapFile(const GString& filepath);
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const GString& filepath);

    /// @}

    /// @name Constructors/Destructor
    /// @{

    CubeTexture(const CubePaths& imagePaths);
    CubeTexture(const GString& cubemapFilePath);

    /// @param[in] width the width of each side of the cube map
    /// @param[in] height the height of each side of the cube map
    CubeTexture(uint32_t width, uint32_t height, uint32_t depth = 1, bool isArray = false, TextureFormat internalFormat = TextureFormat::kRGBA8);
    ~CubeTexture();

    /// @}

    /// @name Properties
    /// @{ 

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eCubeTexture;
    }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CubeTexture& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CubeTexture& orObject);


    /// @}

protected:
    /// @name Friends
    /// @{ 
    
    friend class CubeMapComponent;

    /// @}

    /// @name Private Methods
    /// @{ 

    /// @brief Load images from files
    void loadImages();

    /// @}

    /// @name Private Members
    /// @{ 

    /// @brief Paths to texture images
    CubePaths m_imagePaths;

    /// @}

};


} // End namespaces

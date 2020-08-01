/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_TEXTURE_H
#define GB_TEXTURE_H

// QT
#include <QOpenGLTexture>
#include <QString>

// Internal
#include "../../resource/GbResource.h"
#include "../GbGLFunctions.h"
#include "../shaders/GbUniform.h"
#include "../../resource/GbImage.h"
#include "../../containers/GbContainerExtensions.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Texture;
class ShaderProgram;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Type of model that texture belongs to
enum TextureModelType {
    kTextureModelType_None,  // default
    kTextureModelType_Sphere,
    kTextureModelType_Cube_Top,
    kTextureModelType_Cube_Bottom,
    kTextureModelType_Cube_Front,
    kTextureModelType_Cube_Back,
    kTextureModelType_Cube_Left,
    kTextureModelType_Cube_Right
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct TextureProperties
struct TextureProperties {
    // See: http://paulbourke.net/dataformats/mtl/
    TextureModelType m_textureModelType = Gb::kTextureModelType_None;      // -type (default kTextureModelType_None)

    //Specifies the sharpness of the reflections from the local reflection
    //    map.If a material does not have a local reflection map defined in its
    //    material definition, sharpness will apply to the global reflection map
    //    defined in PreView.
    real_g m_sharpness = 1.0;         // -boost (default 1.0?)

    //-mm base gain
    //    The - mm option modifies the range over which scalar or color texture
    //    values may vary.This has an effect only during rendering and does not
    //    change the file.
    real_g m_brightness = static_cast<real_g>(0.0);        // base_value in -mm option (default 0)
    real_g m_contrast = static_cast<real_g>(1.0);          // gain_value in -mm option (default 1)

    // The - o option offsets the position of the texture map on the surface by
    // shifting the position of the map origin.The default is 0, 0, 0.
    Vector3g m_originOffset;  // -o u [v [w]] (default 0 0 0)

    // The - s option scales the size of the texture pattern on the textured
    // surface by expanding or shrinking the pattern.The default is 1, 1, 1.
    Vector3g m_scale = { static_cast<real_g>(1.0), static_cast<real_g>(1.0), static_cast<real_g>(1.0) }; 
                       // -s u [v [w]] (default 1 1 1)

    // The - t option turns on turbulence for textures.  Adding turbulence to a
    // texture along a specified direction adds variance to the original image
    // and allows a simple image to be repeated over a larger area without
    // noticeable tiling effects.
    Vector3g m_turbulence;     // -t u [v [w]] (default 0 0 0) u is hor direction of texture turbulence, v is vert, w is depth
    // int   texture_resolution; // -texres resolution (default = ?) TODO

    //-clamp on | off
    //    The - clamp option turns clamping on or off.When clamping is on,
    //    textures are restricted to 0 - 1 in the uvw range.The default is off.
    bool m_clamp = false;    // -clamp (default false)

    //-imfchan r | g | b | m | l | z
    //    The - imfchan option specifies the channel used to create a scalar or
    //    bump texture. Scalar textures are applied to :
    //    transparency
    //    specular exponent
    //    decal
    //    displacement

    //    The channel choices are :

    //    r specifies the red channel.
    //    g specifies the green channel.
    //    b specifies the blue channel.
    //    m specifies the matte channel.
    //    l specifies the luminance channel.
    //    z specifies the z - depth channel.

    //    The default for bump and scalar textures is "l" (luminance), unless you
    //    are building a decal.In that case, the default is "m" (matte).
    char m_imfChannel = 'm';  // -imfchan (the default for bump is 'l' and for decal is 'm')

    //-blenu on | off
    //    The - blendu option turns texture blending in the horizontal direction
    //    (u direction) on or off.The default is on.
    bool m_blendu = true;   // -blendu (default on)

    //-blenv on | off
    //    The - blendv option turns texture blending in the vertical direction(v
    //        direction) on or off.The default is on.
    bool m_blendv = true;   // -blendv (default on)

    //-bm mult
    //    The - bm option specifies a bump multiplier.You can use it only with
    //    the "bump" statement.Values stored with the texture or procedural
    //    texture file are multiplied by this value before they are applied to the
    //    surface.

    //    "mult" is the value for the bump multiplier.It can be positive or
    //    negative.Extreme bump multipliers may cause odd visual results because
    //    only the surface normal is perturbed and the surface position does not
    //    change.For best results, use values between 0 and 1.
    real_g m_bumpMultiplier = static_cast<real_g>(1.0);  // -bm (for bump maps only, default 1.0)

    // extension
    std::string m_colorSpace;  // Explicitly specify color space of stored texel
                             // value. Usually `sRGB` or `linear` (default empty).
};

/////////////////////////////////////////////////////////////////////////////////////////////
struct TextureData {

    TextureData(bool isBump = false);
    ~TextureData();

    void initialize(bool isBump);

    /// @brief Name of texture file
    std::string m_textureFileName = "";

    /// @brief The directory of the texture file
    std::string m_textureFileDir = "";

    /// @brief Properties of the texture
    TextureProperties m_properties;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Subclass of QOpenGLTexture
/// @detailed Adds some convenience functions and presets
/// @note GL texture coordinates wrap around, e.g. 3.1 == 0.1, 1.2334 == 0.2334 etc.
// TODO: Make construction private and relegate to resource cache
class Texture: public Resource {

public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

	/// @brief Type of texture
	enum TextureType {
		kDiffuse,
		kNormalMap,
        kAmbient,
        kSpecular,
        kSpecularHighlight,
        kBump,
        kDisplacement,
        kOpacity,
        kReflection,
        kLightMap,
        kAlbedo_PBR,
        kBump_PBR,
        kAmbientOcclusion_PBR,
        kRoughness_PBR,
        kMetallic_PBR,
        kShininess_PBR,
        kEmissive_PBR,
        kMAX_TEXTURE_TYPE
	};

    /// @brief get the uniform name corresponding to the texture type
    static const QString& getUniformName(TextureType type) { return TYPE_TO_UNIFORM_MAP.at(type); }
    
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine,
        const QString& texturePath,
        TextureType type);
    static std::shared_ptr<ResourceHandle> createHandle(
        const QString& texturePath,
        TextureType type,
        ResourceHandle& material);
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* core,
        const Image& image,
        TextureType type = kDiffuse,
        QOpenGLTexture::Filter minFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::Filter maxFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::WrapMode wrapMode = QOpenGLTexture::Repeat);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Destructor
    /// @{

    ~Texture();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    int height() const { return m_texture.height(); }
    int width() const { return m_texture.width(); }

    /// @brief Return GL ID of the texture
    inline GLuint getID() { return m_texture.textureId(); }

    TextureType getType() const { return m_type; }
    void setType(const TextureType& type) { m_type = type; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief bind/release the texture
    void bind(int texUnit = -1);
    void release() { m_texture.release(); }

    /// @brief Perform on removal of this texture resource
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// Get the image stored in GL
    void getGLImage(Image& outImage);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{
    /// @property className
    virtual const char* className() const { return "Texture"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::GL::Texture"; }

    /// @}

protected:

    //---------------------------------------------------------------------------------------
    /// @name Constructors
    /// @{
    Texture(const QString& filePath, TextureType type = kDiffuse);
    Texture(const Image& image,
        TextureType type = kDiffuse,
        QOpenGLTexture::Filter minFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::Filter maxFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::WrapMode wrapMode = QOpenGLTexture::Repeat);
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @brief enable Material to access members of Texture
    friend class Material;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Determine and memory cost of the texture 
    void setCost();
    
    /// @brief Destroy GL texture
    void destroy() {
        if (m_texture.isCreated()) {
            m_texture.destroy();
        }
    }

    /// @brief Initialize the texture
    void initialize(const QString& filePath);


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The image used for construction of this texture
    Image m_image;

    /// @brief Minification filter
    QOpenGLTexture::Filter m_minFilter = QOpenGLTexture::LinearMipMapLinear;

    /// @brief Maxification filter
    /// @details Magnification has no miip-mapping, only supports nearest and linear
    QOpenGLTexture::Filter m_magFilter = QOpenGLTexture::Linear;

    /// @brief Wrap mode
    QOpenGLTexture::WrapMode m_wrapMode = QOpenGLTexture::Repeat;

    /// @The QOpenGLTexture encapsulated by this texture
    QOpenGLTexture m_texture;
	TextureType m_type;

    /// @brief Map of diffuse texture types and corresponding uniform name
    static const std::unordered_map<TextureType, QString> TYPE_TO_UNIFORM_MAP;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
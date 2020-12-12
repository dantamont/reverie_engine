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
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_PATH_SIZE 128 //260

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Texture;
class ShaderProgram;
class FrameBuffer;
class Color;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Type of model that texture belongs to
enum TextureModelType: int {
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
enum class TextureTargetType {
    k1D = GL_TEXTURE_1D,
    k1DArray = GL_TEXTURE_1D_ARRAY,
    k2D = GL_TEXTURE_2D,
    k2DArray = GL_TEXTURE_2D_ARRAY,
    k3D = GL_TEXTURE_3D,
    kCubeMap = GL_TEXTURE_CUBE_MAP,
    kCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
    k2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
    k2DMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    kRectangle = GL_TEXTURE_RECTANGLE,
    kBuffer = GL_TEXTURE_BUFFER
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum TextureBindType {
    k1D = GL_TEXTURE_BINDING_1D,
    k1DArray = GL_TEXTURE_BINDING_1D_ARRAY,
    k2D = GL_TEXTURE_BINDING_2D,
    k2DArray = GL_TEXTURE_BINDING_2D_ARRAY,
    k3D = GL_TEXTURE_BINDING_3D,
    kCubeMap = GL_TEXTURE_BINDING_CUBE_MAP,
    kCubeMapArray = GL_TEXTURE_BINDING_CUBE_MAP_ARRAY,
    k2DMultisample = GL_TEXTURE_BINDING_2D_MULTISAMPLE,
    k2DMultisampleArray = GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY,
    kRectangle = GL_TEXTURE_BINDING_RECTANGLE,
    kBuffer = GL_TEXTURE_BINDING_BUFFER
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class TextureFilter {
    kUnset,
    kNearest = GL_NEAREST,
    kLinear = GL_LINEAR,
    kNearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
    kNearestMipMapLinear = GL_NEAREST_MIPMAP_LINEAR,
    kLinearMipMapNearest = GL_LINEAR_MIPMAP_NEAREST,
    kLinearMipMapLinear = GL_LINEAR_MIPMAP_LINEAR
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class TextureWrapMode {
    kRepeat = GL_REPEAT,
    kMirroredRepeat = GL_MIRRORED_REPEAT,
    kClampToEdge = GL_CLAMP_TO_EDGE,
    kClampToBorder = GL_CLAMP_TO_BORDER
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class TextureCoordinateDirection {
    kS = GL_TEXTURE_WRAP_S,
    kT = GL_TEXTURE_WRAP_T,
    kR = GL_TEXTURE_WRAP_R
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class TextureFormat {
    kNone = GL_NONE,

    // Unsigned normalized formats
    kR8 = GL_R8,
    kRG8 = GL_RG8,
    kRGB8 = GL_RGB8,
    kRGBA8 = GL_RGBA8,

    kR16 = GL_R16,
    kRG16 = GL_RG16,
    kRGB16 = GL_RGB16,
    kRGBA16 = GL_RGBA16,

    // Signed normalized formats
    kR8_SNorm = GL_R8_SNORM,
    kRG8_SNorm = GL_RG8_SNORM,
    kRGB8_SNorm = GL_RGB8_SNORM,
    kRGBA8_SNorm = GL_RGBA8_SNORM,

    kR16_SNorm = GL_R16_SNORM,
    kRG16_SNorm = GL_RG16_SNORM,
    kRGB16_SNorm = GL_RGB16_SNORM,
    kRGBA16_SNorm = GL_RGBA16_SNORM,

    // Unsigned integer formats
    kR8U = GL_R8UI,
    kRG8U = GL_RG8UI,
    kRGB8U = GL_RGB8UI,
    kRGBA8U = GL_RGBA8UI,

    kR16U = GL_R16UI,
    kRG16U = GL_RG16UI,
    kRGB16U = GL_RGB16UI,
    kRGBA16U = GL_RGBA16UI,

    kR32U = GL_R32UI,
    kRG32U = GL_RG32UI,
    kRGB32U = GL_RGB32UI,
    kRGBA32U = GL_RGBA32UI,

    // Signed integer formats
    kR8I = GL_R8I,
    kRG8I = GL_RG8I,
    kRGB8I = GL_RGB8I,
    kRGBA8I = GL_RGBA8I,

    kR16I = GL_R16I,
    kRG16I = GL_RG16I,
    kRGB16I = GL_RGB16I,
    kRGBA16I = GL_RGBA16I,

    kR32I = GL_R32I,
    kRG32I = GL_RG32I,
    kRGB32I = GL_RGB32I,
    kRGBA32I = GL_RGBA32I,

    // Floating point formats
    kR16F = GL_R16F,
    kRG16F = GL_RG16F,
    kRGB16F = GL_RGB16F, // Good for HDR framebuffer
    kRGBA16F = GL_RGBA16F,

    kR32F = GL_R32F,
    kRG32F = GL_RG32F,
    kRGB32F = GL_RGB32F, // Double!
    kRGBA32F = GL_RGBA32F,

    // Packed formats
    kRGB9E5 = GL_RGB9_E5,
    kRG11B10F = GL_R11F_G11F_B10F,
    kRG3B2 = GL_R3_G3_B2,
    kR5G6B5 = GL_RGB565,
    kRGB5A1 = GL_RGB5_A1,
    kRGBA4 = GL_RGBA4,
    kRGB10A2 = GL_RGB10_A2UI,

    // Depth formats
    kDepth16 = GL_DEPTH_COMPONENT16,
    kDepth24 = GL_DEPTH_COMPONENT24,
    kDepth24Stencil8 = GL_DEPTH24_STENCIL8,
    kDepth32 = GL_DEPTH_COMPONENT32,
    kDepth32F = GL_DEPTH_COMPONENT32F,
    kDepth32FStencil8X24 = GL_DEPTH32F_STENCIL8,
    kStencil8 = GL_STENCIL_INDEX8,

    // Compressed formats
    kRGB_DXT1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
    kRGBA_DXT1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,    
    kRGBA_DXT3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,        
    kRGBA_DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,        
    kR_ATI1N_UNorm = GL_COMPRESSED_RED_RGTC1,        
    kR_ATI1N_SNorm = GL_COMPRESSED_SIGNED_RED_RGTC1,    
    kRG_ATI2N_UNorm = GL_COMPRESSED_RG_RGTC2,    
    kRG_ATI2N_SNorm = GL_COMPRESSED_SIGNED_RG_RGTC2,    
    kRGB_BP_UNSIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,    
    kRGB_BP_SIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,    
    kRGB_BP_UNorm = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,    
    kR11_EAC_UNorm = GL_COMPRESSED_R11_EAC,    
    kR11_EAC_SNorm = GL_COMPRESSED_SIGNED_R11_EAC,    
    kRG11_EAC_UNorm = GL_COMPRESSED_RG11_EAC,    
    kRG11_EAC_SNorm = GL_COMPRESSED_SIGNED_RG11_EAC,    
    kRGB8_ETC2 = GL_COMPRESSED_RGB8_ETC2,    
    kSRGB8_ETC2 = GL_COMPRESSED_SRGB8_ETC2,    
    kRGB8_PunchThrough_Alpha1_ETC2 = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 
    kSRGB8_PunchThrough_Alpha1_ETC2 = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 
    kRGBA8_ETC2_EAC = GL_COMPRESSED_RGBA8_ETC2_EAC,    
    kSRGB8_Alpha8_ETC2_EAC = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,    
#if defined(QT_OPENGL_ES_3)
        kRGB8_ETC1 = GL_ETC1_RGB8_OES,
#elif defined(QT_OPENGL_ES_2)
        kRGB8_ETC1 = GL_ETC1_RGB8_OES,
#endif
    kRGBA_ASTC_4x4 = GL_COMPRESSED_RGBA_ASTC_4x4_KHR,    
    kRGBA_ASTC_5x4 = GL_COMPRESSED_RGBA_ASTC_5x4_KHR,    
    kRGBA_ASTC_5x5 = GL_COMPRESSED_RGBA_ASTC_5x5_KHR,    
    kRGBA_ASTC_6x5 = GL_COMPRESSED_RGBA_ASTC_6x5_KHR,    
    kRGBA_ASTC_6x6 = GL_COMPRESSED_RGBA_ASTC_6x6_KHR,    
    kRGBA_ASTC_8x5 = GL_COMPRESSED_RGBA_ASTC_8x5_KHR,    
    kRGBA_ASTC_8x6 = GL_COMPRESSED_RGBA_ASTC_8x6_KHR,    
    kRGBA_ASTC_8x8 = GL_COMPRESSED_RGBA_ASTC_8x8_KHR,    
    kRGBA_ASTC_10x5 = GL_COMPRESSED_RGBA_ASTC_10x5_KHR,    
    kRGBA_ASTC_10x6 = GL_COMPRESSED_RGBA_ASTC_10x6_KHR,    
    kRGBA_ASTC_10x8 = GL_COMPRESSED_RGBA_ASTC_10x8_KHR,    
    kRGBA_ASTC_10x10 = GL_COMPRESSED_RGBA_ASTC_10x10_KHR,    
    kRGBA_ASTC_12x10 = GL_COMPRESSED_RGBA_ASTC_12x10_KHR,    
    kRGBA_ASTC_12x12 = GL_COMPRESSED_RGBA_ASTC_12x12_KHR,    
    kSRGB8_Alpha8_ASTC_4x4 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,    
    kSRGB8_Alpha8_ASTC_5x4 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,    
    kSRGB8_Alpha8_ASTC_5x5 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,    
    kSRGB8_Alpha8_ASTC_6x5 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,    
    kSRGB8_Alpha8_ASTC_6x6 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,    
    kSRGB8_Alpha8_ASTC_8x5 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,    
    kSRGB8_Alpha8_ASTC_8x6 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,    
    kSRGB8_Alpha8_ASTC_8x8 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,    
    kSRGB8_Alpha8_ASTC_10x5 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,    
    kSRGB8_Alpha8_ASTC_10x6 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,    
    kSRGB8_Alpha8_ASTC_10x8 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,    
    kSRGB8_Alpha8_ASTC_10x10 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,   
    kSRGB8_Alpha8_ASTC_12x10 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,   
    kSRGB8_Alpha8_ASTC_12x12 = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,   

    // sRGB formats
    kSRGB8 = GL_SRGB8,    
    kSRGB8_Alpha8 = GL_SRGB8_ALPHA8,    
    kSRGB_DXT1 = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,    
    kSRGB_Alpha_DXT1 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,    
    kSRGB_Alpha_DXT3 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,    
    kSRGB_Alpha_DXT5 = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,    
    kSRGB_BP_UNorm = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,    

    // ES 2 formats
    kDepthFormat = GL_DEPTH_COMPONENT,    
    kAlphaFormat = GL_ALPHA,    
    kRGBFormat = GL_RGB,    
    kRGBAFormat = GL_RGBA,    
    kLuminanceFormat = GL_LUMINANCE,    
    kLuminanceAlphaFormat = GL_LUMINANCE_ALPHA

};

/////////////////////////////////////////////////////////////////////////////////////////////

enum class CubeMapFace {
    kRight = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    kLeft = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    kTop = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    kBottom = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    kBack = GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // Cubemap uses Renderman convention, which is left-handed, so +z is back face
    kFront = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z // and -z is front-face
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class SwizzleComponent {
    kRed = GL_TEXTURE_SWIZZLE_R,  
    kGreen = GL_TEXTURE_SWIZZLE_G,  
    kBlue = GL_TEXTURE_SWIZZLE_B,  
    kAlpha = GL_TEXTURE_SWIZZLE_A   
};
/////////////////////////////////////////////////////////////////////////////////////////////
enum class SwizzleValue {
    kRedValue = GL_RED, 
    kGreenValue = GL_GREEN,
    kBlueValue = GL_BLUE, 
    kAlphaValue = GL_ALPHA,
    kZeroValue = GL_ZERO, 
    kOneValue = GL_ONE 
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class PixelFormat {
    kNone = 0,   
    kRed = GL_RED,    
    kRG = GL_RG,    
    kRGB = GL_RGB,    
    kBGR = GL_BGR,    
    kRGBA = GL_RGBA,    
    kBGRA = GL_BGRA,    
    kRed_Integer = GL_RED_INTEGER,    
    kRG_Integer = GL_RG_INTEGER,    
    kRGB_Integer = GL_RGB_INTEGER,    
    kBGR_Integer = GL_BGR_INTEGER,    
    kRGBA_Integer = GL_RGBA_INTEGER,    
    kBGRA_Integer = GL_BGRA_INTEGER,    
    kStencil = GL_STENCIL_INDEX,    
    kDepth = GL_DEPTH_COMPONENT,    
    kDepthStencil = GL_DEPTH_STENCIL,    
    kAlpha = GL_ALPHA,    
    kLuminance = GL_LUMINANCE,    
    kLuminanceAlpha = GL_LUMINANCE_ALPHA     
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class PixelType {
    kNoPixelType = GL_NONE,         
    kByte8 = GL_BYTE,    
    kUByte8 = GL_UNSIGNED_BYTE,    
    kInt16 = GL_SHORT,    
    kUInt16 = GL_UNSIGNED_SHORT,    
    kInt32 = GL_INT,    
    kUInt32 = GL_UNSIGNED_INT,    
    kFloat16 = GL_HALF_FLOAT,    
#if defined(QT_OPENGL_ES_3)
    kFloat16OES = GL_HALF_FLOAT_OES,    
#elif defined(QT_OPENGL_ES_2)
    kFloat16OES = GL_HALF_FLOAT_OES,    
#endif
    kFloat32 = GL_FLOAT,    
    kUInt32_RGB9_E5 = GL_UNSIGNED_INT_5_9_9_9_REV,    
    kUInt32_RG11B10F = GL_UNSIGNED_INT_10F_11F_11F_REV,    
    kUInt8_RG3B2 = GL_UNSIGNED_BYTE_3_3_2,    
    kUInt8_RG3B2_Rev = GL_UNSIGNED_BYTE_2_3_3_REV,    
    kUInt16_RGB5A1 = GL_UNSIGNED_SHORT_5_5_5_1,    
    kUInt16_RGB5A1_Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,    
    kUInt16_R5G6B5 = GL_UNSIGNED_SHORT_5_6_5,    
    kUInt16_R5G6B5_Rev = GL_UNSIGNED_SHORT_5_6_5_REV,    
    kUInt16_RGBA4 = GL_UNSIGNED_SHORT_4_4_4_4,    
    kUInt16_RGBA4_Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,    
    kUInt32_RGBA8 = GL_UNSIGNED_INT_8_8_8_8,    
    kUInt32_RGBA8_Rev = GL_UNSIGNED_INT_8_8_8_8_REV,    
    kUInt32_RGB10A2 = GL_UNSIGNED_INT_10_10_10_2,    
    kUInt32_RGB10A2_Rev = GL_UNSIGNED_INT_2_10_10_10_REV,    
    kUInt32_D24S8 = GL_UNSIGNED_INT_24_8,    
    kFloat32_D32_UInt32_S8_X24 = GL_FLOAT_32_UNSIGNED_INT_24_8_REV 
};

/////////////////////////////////////////////////////////////////////////////////////////////
enum class DepthStencilMode {
    kDepthComponent = GL_DEPTH_COMPONENT,
    kStencilComponent = GL_STENCIL_INDEX
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
    Vector3 m_originOffset;  // -o u [v [w]] (default 0 0 0)

    // The - s option scales the size of the texture pattern on the textured
    // surface by expanding or shrinking the pattern.The default is 1, 1, 1.
    Vector3 m_scale = { static_cast<real_g>(1.0), static_cast<real_g>(1.0), static_cast<real_g>(1.0) }; 
                       // -s u [v [w]] (default 1 1 1)

    // The - t option turns on turbulence for textures.  Adding turbulence to a
    // texture along a specified direction adds variance to the original image
    // and allows a simple image to be repeated over a larger area without
    // noticeable tiling effects.
    Vector3 m_turbulence;     // -t u [v [w]] (default 0 0 0) u is hor direction of texture turbulence, v is vert, w is depth
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
    //GString m_colorSpace;  // Explicitly specify color space of stored texel
                             // value. Usually `sRGB` or `linear` (default empty).
};

/////////////////////////////////////////////////////////////////////////////////////////////
struct TextureData {

    TextureData(bool isBump = false);
    ~TextureData();

    void initialize(bool isBump);

    GStringView fileName() const {
        return GStringView(m_textureFileName.data());
    }

    /// @brief Name of texture file, is optionally the fully specified path
    /// @details Is stored on stack to make binary serialization less painful
    std::array<char, MAX_PATH_SIZE> m_textureFileName;

    /// @brief Properties of the texture
    TextureProperties m_properties;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Type of texture
enum class TextureUsageType {
    kNone = -1,
    kDiffuse = 0,
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

/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OpenGL Texture Options
struct TextureOptions {

    /// @brief Number of samples for MSAA
    size_t m_numSamples = 16;

    /// @brief Whether or not the image will use identical sample locations and same number of samples
    /// for all texels in the image
    bool m_fixedSampleLocations = true;

    /// @brief Minification filter
    TextureFilter m_minFilter = TextureFilter::kLinearMipMapLinear;

    /// @brief Maxification filter
    /// @details Magnification has no miip-mapping, only supports nearest and linear
    TextureFilter m_magFilter = TextureFilter::kLinear;

    /// @brief Wrap mode
    TextureWrapMode m_wrapMode = TextureWrapMode::kRepeat;

    /// @brief The internal storage format
    TextureFormat m_internalFormat = TextureFormat::kRGBA8;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OpenGL texture
/// @detailed Adds some convenience functions and presets
/// @note GL texture coordinates wrap around, e.g. 3.1 == 0.1, 1.2334 == 0.2334 etc.
// TODO: Make construction private and relegate to resource cache
class Texture: public Resource {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum TextureFlag {
        kInitialized = 1 << 0, // Whether or not the texture is initialize in OpenGL
        kGenerateMipMaps = 1 << 1 // Whether or not to generate mip maps
    };
    typedef QFlags<TextureFlag> TextureFlags;

    /// @brief Return largest allowed texture size that is a power of two (nextPower, nearestPower)
    static size_t MaxDivisibleTextureSize();
    static size_t MaxTextureSize();
    static size_t MaxNumTextureLayers();
    static size_t TextureBoundToActiveUnit();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief get the uniform name corresponding to the texture type
    static const GString& GetUniformName(TextureUsageType type) { return s_typeToUniformVec.at((int)type); }
    
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine,
        const QString& texturePath,
        TextureUsageType type);
    static std::shared_ptr<ResourceHandle> createHandle(
        const QString& texturePath,
        TextureUsageType type,
        ResourceHandle& material);
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* core,
        const Image& image,
        TextureUsageType type = TextureUsageType::kDiffuse,
        TextureFilter minFilter = TextureFilter::kLinearMipMapLinear,
        TextureFilter maxFilter = TextureFilter::kLinearMipMapLinear,
        TextureWrapMode wrapMode = TextureWrapMode::kRepeat);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    Texture(size_t width, size_t height, 
        TextureTargetType targetType = TextureTargetType::k2D,
        TextureUsageType type = TextureUsageType::kNone,
        TextureFilter minFilter = TextureFilter::kLinearMipMapLinear,
        TextureFilter magFilter = TextureFilter::kLinearMipMapLinear,
        TextureWrapMode wrapMode = TextureWrapMode::kRepeat,
        TextureFormat format = TextureFormat::kRGBA8,
        size_t depth = 1);

    virtual ~Texture();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Get the type of resource stored by this handle
    virtual Resource::ResourceType getResourceType() const override {
        return Resource::kTexture;
    }

    //bool isBound() const { return m_isBound; }

    /// @brief Images cached prior to copying into OpenGL
    std::vector<Image>& images() { return m_images; }

    /// @brief Set to generate mip maps
    void generateMipMaps(bool gen) {
        m_textureFlags.setFlag(kGenerateMipMaps, gen);
    }

    /// @brief Whether or not the texture is created in OpenGL
    bool isCreated() const { return m_textureFlags.testFlag(kInitialized); }
    bool hasMipMaps() const { return m_textureFlags.testFlag(kGenerateMipMaps); }

    int height() const { return m_height; }
    int width() const { return m_width; }
    int depth() const { return m_depth; }

    /// @brief Return GL ID of the texture
    inline GLuint getID() { return m_textureID; }

    TextureUsageType getUsageType() const { return m_usageType; }
    void setUsageType(const TextureUsageType& type) { m_usageType = type; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Get data from OpenGL texture
    void getData(void* outData, PixelFormat format, PixelType pixelType);

    /// @brief Set data in the texture
    void setData(const void* data, PixelFormat format, PixelType pixelType);

    /// @brief Set border color of the texture
    void setBorderColor(const Color& color);

    /// @brief Attach the texture to the specified framebuffer
    /// @param[in] attachment The attachment point, e.g. GL_COLOR_ATTACHMENT0, GL_DEPTH_STENCIL_ATTACHMENT
    /// @param[in] depthLayer For texture arrays, the z-level of the array to attach to the framebuffer
    void attach(FrameBuffer& fbo, int attachment, size_t depthLayer = 0);

    /// @brief Set read mode to depth or stencil
    void setReadMode(DepthStencilMode mode);

    void setNumSamples(size_t numSamples) {
        m_textureOptions.m_numSamples = numSamples;
    }

    void setFixedSampleLocations(bool fixed) {
        m_textureOptions.m_fixedSampleLocations = fixed;
    }

    /// @brief bind/release the texture
    void bind(int texUnit = -1);
    void release();

    /// @brief Perform on removal of this texture resource
    void onRemoval(ResourceCache* cache = nullptr) override;

    /// @brief What action to perform post-construction of the resource
    /// @details For performing any actions that need to be done on the main thread
    virtual void postConstruction() override;

    /// Get the image stored in GL
    //void getGLImage(Image& outImage);

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
    Texture(const QString& filePath, TextureUsageType type = TextureUsageType::kDiffuse);
    
    Texture(const Image& image,
        TextureUsageType type = TextureUsageType::kDiffuse,
        TextureFilter minFilter = TextureFilter::kLinear,
        TextureFilter maxFilter = TextureFilter::kLinear,
        TextureWrapMode wrapMode = TextureWrapMode::kRepeat,
        TextureTargetType targetType = TextureTargetType::k2D); 

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

    /// @brief Use cached images to set data for texture in OpenGL, then clear them
    /// @param[in] depthLevel zoffset to set texture data, only used for texture arrays
    void flushData();

    /// @brief Generate mip maps for the texture
    void generateMipMaps();

    /// @brief Set texture parameters
    void setTextureParameters();

    /// @brief Allocate memory for texture data
    void allocateMemory();

    /// @brief Determine and memory cost of the texture 
    void setCost();
    
    /// @brief Destroy GL texture
    void destroy();

    /// @brief Get max number of mip-map levels
    size_t maxMipMapLevel();

    /// @brief Initialize the texture's image (NOT IN GL)
    void initialize(const Image& img, bool mirror);
    void initialize(const QString& filePath);

    /// @brief Initialize the texture in GL
    void initializeGL();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    // TODO: Use context to create texture, so that context can have list of textures bound by unit
    //bool m_isBound = false;

    /// @brief The images used for construction of this texture
    /// @note A QImage is like a shared pointer, in the sense that QImages referencing
    /// the same underlying image (i.e., at the same filepath) will point to the same
    /// place in memory, and clearing one will not clear the rest.
    std::vector<Image> m_images;

    /// @brief Texture dimensions
    size_t m_height;
    size_t m_width;
    size_t m_depth;

    TextureFlags m_textureFlags;

    TextureOptions m_textureOptions;

    /// @brief The OpenGL texture ID
    /// @details ID is negative 1 if not yet created in OpenGL
    GLuint m_textureID;

    TextureTargetType m_targetType;
	TextureUsageType m_usageType;

    /// @brief Map of diffuse texture types and corresponding uniform name
    static const std::vector<GString> s_typeToUniformVec;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
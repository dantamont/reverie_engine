/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MATERIAL_H
#define GB_MATERIAL_H

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
class Material;
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
        kEmissive_PBR
	};

    /// @brief get the uniform name corresponding to the texture type
    static const QString& getUniformName(TextureType type) { return TYPE_TO_UNIFORM_MAP.at(type); }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Texture(const QString& filePath, TextureType type = kDiffuse);
    Texture(const Image& image,
        TextureType type = kDiffuse, 
        QOpenGLTexture::Filter minFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::Filter maxFilter = QOpenGLTexture::LinearMipMapLinear,
        QOpenGLTexture::WrapMode wrapMode = QOpenGLTexture::Repeat);
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

    ///// @brief Obtain the offset from the contained textures in texture coordinates
    //Vector2f getTextureOffset(size_t index) {
    //    return Vector2f(getTextureXOffset(index), getTextureYOffset(index));
    //}


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

    ///// @brief Obtain the x offset used from contained textures (in texture coordinates)
    ///// [in] The index of the texture in the atlas (indexing is from top left to right, then down)
    //double getTextureXOffset(size_t index) {
    //    size_t column = index % m_numRows;
    //    return double(column) / double(m_numRows);
    //}

    ///// @brief Obtain the y offset used from contained textures (in texture coordinates)
    ///// [in] The index of the texture in the atlas (indexing is from top left to right, then down)
    //double getTextureYOffset(size_t index) {
    //    size_t row = index / m_numRows; // don't need floor, since these are ints
    //    return double(row) / double(m_numRows);
    //}

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

    ///// @brief Number of rows (for if this is a texture atlas)
    //unsigned int m_numRows;

    /// @The QOpenGLTexture encapsulated by this texture
    QOpenGLTexture m_texture;
	TextureType m_type;

    /// @brief Map of diffuse texture types and corresponding uniform name
    static const std::unordered_map<TextureType, QString> TYPE_TO_UNIFORM_MAP;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MaterialProperties
/// @brief Structure containing material properties
struct MaterialProperties: Serializable {

    MaterialProperties();
    MaterialProperties(const QJsonValue& json);
    ~MaterialProperties();

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    Vector3g m_ambient; // Base color of texture
    Vector3g m_diffuse; // Diffuse color of texture
    Vector3g m_specularity; // (reflectivity) Specular reflection, effects color intensity of specular reflection
    Vector3g m_transmittance; // Transmission filter of the object, 0 1 0 would allow all green through, no red or blue
    Vector3g m_emission; // same as luminosity, self-illumination
    real_g m_shininess = 1.0; // Same as glossiness, shine dampening, exponent that approximates surface glossiness for specular reflection
    real_g m_ior = 1.0;       // index of refraction, also known as optical density, from 0.001 to 10, 0.001 does not bend light, glass = 1.5, want >1
    real_g m_dissolve = 1.0;  // (opacity) how much material dissolves into background, 1 == opaque; 0 == fully transparent
                      // Does not depend on material thickness, unlike real transparent material

    // illumination model (see http://www.fileformat.info/format/material/)
    //illum illum_#

    //    The "illum" statement specifies the illumination model to use in the material.Illumination models are mathematical equations that represent various material lighting and shading effects.

    //    "illum_#"can be a number from 0 to 10. The illumination models are summarized below; for complete descriptions see "Illumination models" on page 5 - 30.

    //    Illumination Properties that are turned on in the
    //    model Property Editor

    //    0 Color on and Ambient off
    //    1 Color on and Ambient on
    //    2 Highlight on
    //    3 Reflection on and Ray trace on
    //    4 Transparency: Glass on
    //    Reflection : Ray trace on
    //    5 Reflection : Fresnel on and Ray trace on
    //    6 Transparency : Refraction on
    //    Reflection : Fresnel off and Ray trace on
    //    7 Transparency : Refraction on
    //    Reflection : Fresnel on and Ray trace on
    //    8 Reflection on and Ray trace off
    //    9 Transparency : Glass on
    //    Reflection : Ray trace off
    //    10 Casts shadows onto invisible surfaces
    int m_illum = 0;

    // PBR Extension
    // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
    real_g m_roughness = 0.0;           // [0, 1] default 0
    real_g m_metallic = 0.0;            // [0, 1] default 0
    real_g m_sheen = 0.0;               // [0, 1] default 0
    real_g m_clearcoatThickness = 0.0;  // [0, 1] default 0
    real_g m_clearcoatRoughness = 0.0;  // [0, 1] default 0
    real_g m_anisotropy = 0.0;          // aniso. [0, 1] default 0
    real_g m_anisotropyRotation = 0.0;  // anisor. [0, 1] default 0

    // Custom parameters
    std::unordered_map<std::string, std::string> m_customParameters;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MaterialData
/// @brief Structure for parsing in data from a file
/// @note For structure alignment, see:
/// http://www.catb.org/esr/structure-packing/
struct MaterialData {
    MaterialData();
    ~MaterialData();

    void initialize();

    /// @brief Return texture of the given type
    TextureData& textureData(Texture::TextureType type);

    std::string m_name;

    // Path for material file
    /// @note May not represent an actual path, but the directory is needed for loading textures
    std::string m_path;

    // ID
    int m_id;

    // Struct for material properties
    MaterialProperties m_properties;

    // Texture options
    TextureData m_ambientTexture;           // map_Ka
    TextureData m_diffuseTexture;           // map_Kd
    TextureData m_specularTexture;          // map_Ks (specular reflectivity using rgb)
    TextureData m_specularHighlightTexture; // map_Ns (exponent, the specular highlight, large makes concentrated)
    TextureData m_bumpTexture = true;       // map_bump, map_Bump, bump
    TextureData m_displacementTexture;      // disp
    TextureData m_alphaTexture;             // map_d
    TextureData m_reflectionTexture;        // refl
    TextureData m_lightmapTexture;
    TextureData m_opacityTexture;

    // PBR extension
    // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
    TextureData m_albedoTexture; // same as diffuse, or base color, but with shadows removed (may be more realistic)
    TextureData m_ambientOcclusionTexture;        // ambient occlusion handles shadows
    TextureData m_pbrBumpMapping;
    TextureData m_roughnessTexture;        // map_Pr (or gloss, are simply inverses)
    TextureData m_metallicTexture;         // map_Pm
    TextureData m_sheenTexture;            // map_Ps
    TextureData m_emissiveTexture;         // map_Ke
    TextureData m_normalTexture;           // norm. For normal mapping.

    TextureData m_unknown;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Material
/// @brief Encapsulation of texture data to contain material info
/// @detailed Describes texture and related metadata for rendering
///     The material may serve as an atlas (a compilation of many textures in one image)
// TODO: Make construction private and relegate to resource cache
class Material: public Gb::Object, public Loadable{
     
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Material(CoreEngine*, const QJsonValue& json);
    Material(CoreEngine*, const QString& name);
    Material(CoreEngine*, MaterialData& data);
    Material(CoreEngine*, MaterialData&& data);
    Material(CoreEngine*, const QString& name, const std::shared_ptr<Gb::ResourceHandle>& diffuse);
    ~Material();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
	/// @property isBound
	/// @brief whether the material's textures are bound or not
	bool isBound() { return m_isBound; }
	void setIsBound(bool isBound) { m_isBound = isBound; }

    /// @property Shininess
    void setShininess(real_g shininess) { m_properties.m_shininess = shininess; }

    /// @property Specularity
    void setSpecularity(const Vector3g& s) { m_properties.m_specularity = s; }

    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @property Set the name of the material and change location in resource map
    void rename(const QString& name);

    /// @brief Whether or not the material is still in the resourceCache
    bool inResourceCache() const;

    /// @brief Search directory for the material and its textures
    QString getSearchDirectory() const;

	/// @brief Adds a diffuse texture
	void setDiffuseTexture(std::shared_ptr<Gb::ResourceHandle> diffuse);

	/// @brief Binds all the textures associated with this material
	void bind(const std::shared_ptr<ShaderProgram>& shaderProgram);

	/// @brief Unbinds all the textures associated with this material
	void release();

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{
    /// @property className
    virtual const char* className() const { return "Material"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::GL::Material"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    friend bool operator==(const Material& m1, const Material& m2);

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Initialize texture in resource cache
    void initializeTexture(int textureType, MaterialData & data);

    /// @brief Return shared_ptr to specified texture type
    std::shared_ptr<ResourceHandle> getTexture(Texture::TextureType);

    /// @brief Set the texture to the correct type
    void setTexture(std::shared_ptr<ResourceHandle> resourceHandle, Texture::TextureType type);

    /// @brief Set uniforms in the given shader program
    void setShaderUniformsAndBindTextures(const std::shared_ptr<ShaderProgram>& shaderProgram);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    bool m_isBound;

    /// @brief Properties of the material
    MaterialProperties m_properties;

    /// @brief the textures used by this material
    std::unordered_map<Texture::TextureType, std::shared_ptr<Gb::ResourceHandle>> m_textures;
    std::unordered_map<Texture::TextureType, TextureProperties> m_textureProperties;

    /// @brief core engine
    CoreEngine* m_engine;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
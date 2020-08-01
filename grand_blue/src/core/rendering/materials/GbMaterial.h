/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MATERIAL_H
#define GB_MATERIAL_H

// QT

// Internal
#include "GbTexture.h"

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Material;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
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
class Material: public Resource, public Serializable{
     
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Create a handle to a material resource
    static std::shared_ptr<ResourceHandle> createHandle(CoreEngine* engine, const QString& mtlName);

    static void clearCount();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ~Material();
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    size_t getSortID() const;

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

    /// @brief Set material data
    void setData(MaterialData&& data);

    /// @brief Whether or not the material is still in the resourceCache
    bool inResourceCache() const;

    /// @brief Search directory for the material and its textures
    QString getSearchDirectory() const;

	/// @brief Adds a diffuse texture
	void setDiffuseTexture(std::shared_ptr<Gb::ResourceHandle> diffuse);

	/// @brief Binds all the textures associated with this material
	void bind(ShaderProgram& shaderProgram);

	/// @brief Unbinds all the textures associated with this material
	void release();

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override;

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

    //friend bool operator==(const Material& m1, const Material& m2);

    /// @}

protected:
    friend class Model;
    friend class LoadProcess;

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Material(CoreEngine*);
    Material(CoreEngine*, const QString& name);
    //Material(CoreEngine*, MaterialData&& data);
    //Material(CoreEngine*, const QString& name, const std::shared_ptr<Gb::ResourceHandle>& diffuse);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Whether the material has a texture of the specified type
    bool hasType(Texture::TextureType type);

    /// @brief Initialize texture in resource cache
    void initializeTexture(int textureType, MaterialData & data);

    /// @brief Return shared_ptr to specified texture type
    std::shared_ptr<ResourceHandle> getTexture(Texture::TextureType);

    /// @brief Set the texture to the correct type
    void setTexture(std::shared_ptr<ResourceHandle> resourceHandle, Texture::TextureType type);

    /// @brief Set uniforms in the given shader program
    void setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    bool m_isBound;

    size_t m_sortID;

    /// @brief Properties of the material
    MaterialProperties m_properties;

    /// @brief the textures used by this material
    //std::unordered_map<Texture::TextureType, std::shared_ptr<Gb::ResourceHandle>> m_textures;
    std::unordered_map<Texture::TextureType, TextureProperties> m_textureProperties;

    /// @brief core engine
    CoreEngine* m_engine;

    static size_t s_materialCount;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MATERIAL_H
#define GB_MATERIAL_H

// QT

// Internal
#include "GTexture.h"
#include "../../containers/GString.h"

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class Material;
class RenderContext;
template<typename ...Args> class Protocol;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MaterialProperties
/// @brief Structure containing material properties
struct MaterialProperties {

    MaterialProperties();
    ~MaterialProperties();

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @}

    Vector3 m_ambient; // Base color of texture
    Vector3 m_diffuse; // Diffuse color of texture
    Vector3 m_specularity; // (reflectivity) Specular reflection, effects color intensity of specular reflection
    Vector3 m_transmittance; // Transmission filter of the object, 0 1 0 would allow all green through, no red or blue
    Vector3 m_emission; // same as luminosity, self-illumination
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
    uint32_t m_illum = 0;

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
    // Only used in tiny_obj loader right now, deprecated
    //std::unordered_map<std::string, std::string> m_customParameters;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MaterialTextureDataa
/// @brief Represents all texture data associated with a material
struct MaterialTextureData {
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

// Typedef protocol
typedef Protocol<char*, char*, int, MaterialProperties, MaterialTextureData> MaterialDataProtocol;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct MaterialData
/// @brief Structure for parsing in data from a file
/// @note For structure alignment, see:
/// http://www.catb.org/esr/structure-packing/
struct MaterialData {
    MaterialData();
    ~MaterialData();

    /// @brief Create protocol representing this material data
    /// @note The material data does not assume ownership of the protocol, which must be manually deallocated
    MaterialDataProtocol* createProtocol();
    void createProtocol(MaterialDataProtocol& outProtocol);

    void initialize();

    /// @brief Return texture of the given type
    const TextureData& textureData(TextureUsageType type) const;

    /// @name Members
    /// @{
    GString m_name = "";

    // Path for material file
    /// @note May not represent an actual path, but the directory is needed for loading textures
    GString m_path = "";

    // ID
    int32_t m_id;

    // Struct for material properties
    MaterialProperties m_properties;
    
    /// @brief Properties of the textures used by this material
    MaterialTextureData m_textureData;

    /// @}
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
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const GString& mtlName);

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

    /// @brief Get the type of resource stored by this handle
    virtual ResourceType getResourceType() const override {
        return ResourceType::kMaterial;
    }

    uint32_t getSortID() const;

	/// @property isBound
	/// @brief whether the material's textures are bound or not
    bool isBound(RenderContext& context) const;

    /// @property Shininess
    void setShininess(real_g shininess) { m_data.m_properties.m_shininess = shininess; }

    /// @property Specularity
    void setSpecularity(const Vector3& s) { m_data.m_properties.m_specularity = s; }

    /// @}

	//---------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    const MaterialData& data() const { return m_data; }
    MaterialData& data() { return m_data; }

    /// @brief Set material data
    void setData(const MaterialData& data);

    /// @brief Search directory for the material and its textures
    QString getSearchDirectory() const;

	/// @brief Adds a diffuse texture
	void setDiffuseTexture(std::shared_ptr<rev::ResourceHandle> diffuse);

	/// @brief Binds all the textures associated with this material
	void bind(ShaderProgram& shaderProgram, RenderContext& context);

	/// @brief Unbinds all the textures associated with this material
	void release(RenderContext& context);

    /// @brief What action to perform on removal of the resource
    virtual void onRemoval(ResourceCache* cache = nullptr) override;

	/// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{
    /// @property className
    virtual const char* className() const { return "Material"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::GL::Material"; }
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
    Material();
    //Material(const QString& name);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Whether the material has a texture of the specified type
    bool hasType(TextureUsageType type);

    /// @brief Initialize texture in resource cache
    void initializeTexture(int textureType, const MaterialData & data);

    /// @brief Return shared_ptr to specified texture type
    std::shared_ptr<ResourceHandle> getTexture(TextureUsageType);

    /// @brief Set the texture to the correct type
    void setTexture(std::shared_ptr<ResourceHandle> resourceHandle, TextureUsageType type);

    /// @brief Set uniforms in the given shader program
    void setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram, RenderContext& context);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    uint32_t m_sortIndex;

    /// @brief Properties of the material
    //MaterialProperties m_properties;
    MaterialData m_data;

    static uint32_t s_materialCount;

    /// @}
};
        
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
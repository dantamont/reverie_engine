#pragma once

// Internal
#include "GTexture.h"
#include <fortress/numeric/GSizedTypes.h>
#include "fortress/image/GTexturePacker.h"
#include "fortress/string/GString.h"
#include "core/rendering/lighting/GLightSettings.h" // For shadow map texture count

namespace rev {  

class Material;
class RenderContext;
template<typename ...Args> class SerializationProtocol;

/// @struct Material PBR properties
struct PBRProperties {
    // PBR Extension
    // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
    Real_t m_roughness = 0.0;           // [0, 1] default 0
    Real_t m_metallic = 0.0;            // [0, 1] default 0
    Real_t m_sheen = 0.0;               // [0, 1] default 0
    Real_t m_clearcoatThickness = 0.0;  // [0, 1] default 0
    Real_t m_clearcoatRoughness = 0.0;  // [0, 1] default 0
    Real_t m_anisotropy = 0.0;          // aniso. [0, 1] default 0
    Real_t m_anisotropyRotation = 0.0;  // anisor. [0, 1] default 0
};

/// @struct MaterialProperties
/// @brief Structure containing material properties
struct MaterialProperties {

    MaterialProperties();
    ~MaterialProperties();

    /// @name Friend Functions
    /// @{

    /// @}

    Vector3 m_ambient; // Base color of texture
    Vector3 m_diffuse; // Diffuse color of texture
    Vector3 m_specularity; // (reflectivity) Specular reflection, effects color intensity of specular reflection
    Vector3 m_transmittance; // Transmission filter of the object, 0 1 0 would allow all green through, no red or blue
    Vector3 m_emission; // same as luminosity, self-illumination
    Real_t m_shininess = 1.0; // Same as glossiness, shine dampening, exponent that approximates surface glossiness for specular reflection
    Real_t m_ior = 1.0;       // index of refraction, also known as optical density, from 0.001 to 10, 0.001 does not bend light, glass = 1.5, want >1
    Real_t m_dissolve = 1.0;  // (opacity) how much material dissolves into background, 1 == opaque; 0 == fully transparent
                      // Does not depend on material thickness, unlike real transparent material

    // TODO: Create a flag for something similar in material, control:
    // 1) Shadow casting
    // 2) Transparency
    // 3) Visibility
    // // NOTE, unused. Illumination model (see http://www.fileformat.info/format/material/)
    uint32_t m_illum = 0;

    /// @brief Optional PBR parameters for the material
    PBRProperties m_pbr;
};


/// @struct MaterialTextureData
/// @brief Represents all texture data associated with a material
struct MaterialTextureData {
    /// @brief Texture data, indexed by texture type
    std::vector<TextureData> m_data;
};

// Typedef protocol
typedef SerializationProtocol<char*, char*, int, MaterialProperties, std::vector<TextureData>> MaterialProtocol;

/// @struct MaterialData
/// @brief Structure for parsing in data from a file
/// @note For structure alignment, see:
/// http://www.catb.org/esr/structure-packing/
struct MaterialData {
    MaterialData();
    ~MaterialData();

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

    /// @brief Metadata if the material is a sprite sheet
    SpriteSheetInfo m_spriteSheetInfo;

    /// @}
};


// Material
/// @brief Encapsulation of texture data to contain material info
/// @detailed Describes texture and related metadata for rendering
///     The material may serve as an atlas (a compilation of many textures in one image)
class Material: public Resource{
public:
    /// @name Static
    /// @{

    /// @brief Create a handle to a material resource
    static std::shared_ptr<ResourceHandle> CreateHandle(CoreEngine* engine, const GString& mtlName);

    static void clearCount();

    /// @}

    /// @name Constructors/Destructor
    /// @{
    ~Material();
    /// @}

    /// @name Public Methods
    /// @{

    const SpriteSheetInfo& spriteInfo() const {
        return m_spriteSheetInfo;
    }
    void setSpriteInfo(const SpriteSheetInfo& info) {
        m_spriteSheetInfo = info;
    }

    /// @brief Get the type of resource stored by this handle
    virtual GResourceType getResourceType() const override {
        return EResourceType::eMaterial;
    }

    uint32_t getSortID() const;

    void initializeUniformValues(UniformContainer& uc);

	/// @property isBound
	/// @brief whether the material's textures are bound or not
    bool isBound(RenderContext& context) const;

    /// @brief Create protocol representing this material
    /// @param[in] outData The material data generated along with the protocol. Must persist until protocol use is complete
    std::unique_ptr<MaterialProtocol> createProtocol(MaterialData& outData);

    const MaterialProperties& properties() const { return m_properties; }

    /// @brief Generate material data from this material
    MaterialData getData() const;

    /// @brief Set material data
    /// @param[in] serialLoad If true, load textures on this thread
    void setData(const MaterialData& data, bool serialLoad = false);

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

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Material& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Material& orObject);


    /// @}

protected:
    friend class Model;
    friend class LoadProcess;

    /// @name Constructors/Destructor
    /// @{
    Material();
    //Material(const QString& name);

    /// @}

    /// @name Protected methods
    /// @{

    /// @brief Whether the material has a texture of the specified type
    bool hasType(TextureUsageType type);

    /// @brief Initialize texture in resource cache
    void initializeTexture(TextureUsageType textureType, const MaterialData & data, bool serialLoad);

    /// @brief Resolve the path of a texture from the filename
    /// @return the path to the texture
    GString resolveTexturePath(const GStringView& textureFilename);

    /// @brief Generate texture data for the material
    void getTextureData(MaterialTextureData& outData) const;

    /// @brief Return shared_ptr to specified texture type
    std::shared_ptr<ResourceHandle> getTexture(TextureUsageType);

    /// @brief Set the texture to the correct type
    void setTexture(std::shared_ptr<ResourceHandle> resourceHandle, TextureUsageType type);

    /// @brief Set uniforms in the given shader program
    void setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram, RenderContext& context);

    /// @}

    /// @name Protected members
    /// @{

    struct MaterialUniforms {
        UniformData m_blankTextureUnit; ///< Unit of the blank texture
        UniformData m_specularity;
        UniformData m_shininess;
        std::array<UniformData, (Uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE> m_textureUnits; ///< Each type might not be used
    };
    MaterialUniforms m_materialUniforms; ///< The uniforms used by the material

    Uint32_t m_sortIndex;
    MaterialProperties m_properties; ///< Properties of the material
    SpriteSheetInfo m_spriteSheetInfo; ///< Metadata if the material is a sprite sheet

    static Uint32_t s_materialCount;
    static constexpr Uint32_t s_reservedTextureCount = NUM_SHADOW_MAP_TEXTURES + 1;

    /// @}
};
        

} // End namespaces

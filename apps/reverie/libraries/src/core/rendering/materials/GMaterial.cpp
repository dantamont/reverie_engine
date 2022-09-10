#include "core/rendering/materials/GMaterial.h"

// Qt
#include <QDebug>

// internal
#include "fortress/encoding/binary/GSerializationProtocol.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "fortress/process/GProcess.h"
#include "fortress/json/GJson.h"
#include "core/rendering/GGLFunctions.h"
//#include "QGLFramebufferObject"
#include "core/rendering/lighting/GShadowMap.h"
#include "core/rendering/lighting/GLightSettings.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/resource/GFileManager.h"

namespace rev {



// MaterialProperties

MaterialProperties::MaterialProperties()
{
}

MaterialProperties::~MaterialProperties()
{
}

//void to_json(json& orJson, const MaterialProperties& korObject)
//{
//    json object = json::object();
//    orJson["shininess"] = korObject.m_shininess;
//    orJson["specularity"] = korObject.m_specularity.asJson();
//
//    return object;
//}

//void from_json(const json& korJson, MaterialProperties& orObject)
//{
//    const QJsonObject& object = json.toObject();
//    m_shininess = korJson.at("shininess").toDouble();
//    m_specularity = Vector3g(korJson.at("specularity"));
//}






// MaterialData

MaterialData::MaterialData() {
    initialize();
}

MaterialData::~MaterialData()
{
}

void MaterialData::initialize()
{
    m_name = "";
    m_properties = MaterialProperties();
}

const TextureData & MaterialData::textureData(TextureUsageType type) const
{
    // Add texture data to material data
    if (type == TextureUsageType::kNone || type >= TextureUsageType::kMAX_TEXTURE_TYPE) {
        Logger::Throw("Error, texture type unrecognized");
    }

    if ((size_t)type > m_textureData.m_data.size()) {
        Logger::Throw("Error, texture type not found");
    }

    return m_textureData.m_data[(size_t)type];
}





// Material

std::shared_ptr<ResourceHandle> Material::CreateHandle(CoreEngine * engine, const GString & mtlName)
{
    // Create handle for material
    auto handle = ResourceHandle::Create(engine, (GResourceType)EResourceType::eMaterial);
    //handle->setResourceType(EResourceType::eMaterial);
    handle->setRuntimeGenerated(true); // material is generated in engine
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_unique<Material>();
    mtl->initializeUniformValues(engine->openGlRenderer()->renderContext().uniformContainer());
    handle->setResource(std::move(mtl), false);

    return handle;
}

void Material::clearCount()
{
    s_materialCount = 0;
}

Material::Material():
    m_sortIndex(s_materialCount++)
{
}


//Material::Material(const QString & name) :
//    Resource(name),
//    m_sortIndex(s_materialCount++)
//{
//}

Material::~Material()
{
}

uint32_t Material::getSortID() const
{
    return m_sortIndex;
}

void Material::initializeUniformValues(UniformContainer& uc)
{
    m_materialUniforms.m_blankTextureUnit.setValue((Int32_t)s_reservedTextureCount, uc);
}

bool Material::isBound(RenderContext & context) const
{
    return context.boundMaterial() == m_sortIndex;
}

std::unique_ptr<MaterialProtocol> Material::createProtocol(MaterialData& outData)
{
    // Create material data
    outData = getData();

    // Create protocol with members
    // Note, pointers must be passed as a SerializationProtocolField, since they require a count
    GStringProtocolField nameField(outData.m_name.createProtocolField());
    GStringProtocolField pathField(outData.m_path.createProtocolField());
    auto protocol = std::make_unique<MaterialProtocol>(
        nameField,
        pathField,
        outData.m_id,
        outData.m_properties,
        outData.m_textureData.m_data);

    return protocol;
}

MaterialData Material::getData() const
{
    MaterialData data;
    data.m_name = m_handle->getName();
    data.m_path = m_handle->getPath();
    data.m_id = m_sortIndex;
    data.m_properties = m_properties;
    
    // Populate texture data
    getTextureData(data.m_textureData);

    return data;
}

void Material::setData(const MaterialData & data, bool serialLoad)
{
    // Initialize properties
    m_properties = data.m_properties;

    // Initialize textures
    for (const TextureData& td : data.m_textureData.m_data) {
        TextureUsageType texType = td.m_properties.m_usageType;
        if (texType != TextureUsageType::kNone) {
            initializeTexture(texType, data, serialLoad);
        }
    }

    // TODO: Load options
}

QString Material::getSearchDirectory() const
{
    GFile file(m_handle->getPath());
    return file.getAbsoluteDirectory();
}

void Material::setDiffuseTexture(std::shared_ptr<rev::ResourceHandle> diffuse)
{
	setTexture(diffuse, TextureUsageType::kDiffuse);
}


void Material::bind(ShaderProgram& shaderProgram, RenderContext& context)
{
    // TODO: Set bound material in context so that this check is more performant
    if (context.boundMaterial() != m_sortIndex) {
        // Set uniforms and bind textures
        setShaderUniformsAndBindTextures(shaderProgram, context);

        // Set bound status
        context.setBoundMaterial(m_sortIndex);
    }
}


void Material::release(RenderContext& context)
{
    Q_UNUSED(context);
    // 10/6/2020: Unnecessary, is overwritten by bind
	//for (const auto& textureHandle : m_handle->children()) {
 //       if (!textureHandle->getResourceType() == EResourceType::eTexture)
 //           Logger::Throw("Error, material has a child resource that is not a texture");
 //       if (!textureHandle->resource()) continue;
 //       QMutexLocker locker(&textureHandle->mutex());
 //       if (textureHandle->isConstructed()) {
 //           textureHandle->resourceAs<Texture>()->release();
 //       }
	//}

    // Necessary to ensure that material isn't incorrectly flagged as bound when
    // its textures may have been overwritten
    context.setBoundMaterial(-1);
}

void Material::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}

void to_json(json& orJson, const Material& korObject)
{
    // Store textures
    json textures;
    for (const std::shared_ptr<rev::ResourceHandle>& textureHandle : korObject.m_handle->children()) {
        Texture* texture = textureHandle->resourceAs<Texture>();
        int textureType = int(texture->getUsageType());
        textures[GString::FromNumber(textureType).c_str()] = textureHandle->getName().c_str();
    }
    orJson["textures"] = textures;

    // Properties
    json propertiesObject;
    propertiesObject["shininess"] = korObject.m_properties.m_shininess;
    propertiesObject["specularity"] = korObject.m_properties.m_specularity;
    orJson["properties"] = propertiesObject;

    // Sprite info
    orJson["spriteInfo"] = korObject.m_spriteSheetInfo;
}

void from_json(const json& korJson, Material& orObject)
{
    // Clear handle's previous children
    for (const auto& child : orObject.m_handle->children()) {
        if (child->isChild()) {
            child->removeFromCache(true);
        }
    }
    orObject.m_handle->children().clear();

    // Load texture
    const json& textures = korJson.at("textures");
    for(const auto& jsonPair: textures.items()) {
        GString texHandleName = jsonPair.value().get_ref<const std::string&>().c_str();
        //TextureUsageType texType = TextureUsageType(key.toInt());
        auto texHandle = ResourceCache::Instance().getHandleWithName(texHandleName, EResourceType::eTexture);
#ifdef DEBUG_MODE
        if (!texHandle)
        {
            Logger::Throw("Error, handle not found for given name");
        }
#endif
        orObject.m_handle->addChild(texHandle);
    }

    // Load properties
    const json& properties = korJson.at("properties");
    properties.at("shininess").get_to(orObject.m_properties.m_shininess);
    properties.at("specularity").get_to(orObject.m_properties.m_specularity);

    // Load sprite info
    if (korJson.contains("spriteInfo")) {
        orObject.m_spriteSheetInfo = korJson["spriteInfo"];
    }
    else if (korJson.contains("spriteDims")) {
        // Convenience option for generating sprite-sheet info
        if (!orObject.m_handle->children().size()) {
            Logger::Throw("Error, no children found");
        }
        std::shared_ptr<ResourceHandle> diffuseTexHandle = orObject.m_handle->children()[0];
        Texture* diffuseTex = diffuseTexHandle->resourceAs<Texture>();
        uint32_t w = diffuseTex->width();
        uint32_t h = diffuseTex->height();

        const json& dims = korJson["spriteDims"];
        uint32_t dimX = dims[0].get<Uint32_t>();
        uint32_t dimY = dims[1].get<Uint32_t>();
        uint32_t spriteWidth = w / dimX;
        uint32_t spriteHeight = h / dimY;
        
        orObject.m_spriteSheetInfo.m_packedTextures.clear();
        for (size_t j = 0; j < dimY; j++) {
            for (size_t i = 0; i < dimX; i++) {
                orObject.m_spriteSheetInfo.m_packedTextures.emplace_back(
                    PackedTextureInfo{Vector2i(i * spriteWidth, j * spriteHeight), Vector2i(spriteWidth, spriteHeight)}
                );
            }
        }
    }

}

bool Material::hasType(TextureUsageType type)
{
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != EResourceType::eTexture)
            Logger::Throw("Wrong resource type");
        auto texture = handle->resourceAs<Texture>();
        return texture->getUsageType() == type;
    });

    if (iter == m_handle->children().end()) {
        return false;
    }
    else {
        return true;
    }
}

void Material::initializeTexture(TextureUsageType intType, const MaterialData & data, bool serialLoad)
{
#ifdef DEBUG_MODE
    if (data.m_textureData.m_data.size() <= (size_t)intType) {
        // Type not found
        Logger::Throw("Texture type not found");
    }
#endif

    // Get texture data of the given type
    TextureUsageType textureType = TextureUsageType(intType);
    const TextureData& texData = data.textureData(textureType);
    GStringView textureFileName = texData.fileName();
    if (textureFileName.isEmpty()) {
        // If no texture name, texture not loaded
        throw std::exception("Error, texture not found");
    }

    // Get texture path from material data
    GString texturePath = resolveTexturePath(textureFileName);

    // Create handle to load texture from material data
    std::shared_ptr<ResourceHandle> textureHandle =
        Texture::CreateHandle(texturePath, textureType, *m_handle);

    // Load texture
    textureHandle->loadResource(serialLoad);
}

GString Material::resolveTexturePath(const GStringView& textureFileName)
{
    GString texturePath;
    if (!QFile::exists(textureFileName.c_str())) {
        // If filepath is not fully specified, walk directory structure to find
        GString dirStr = getSearchDirectory().toStdString();
        GDir dir(dirStr);
        bool exists = dir.containsFile(textureFileName.c_str(), true, texturePath);

        // If doesn't exist, use file manager to look through all search paths
        if (!exists) {
            exists = m_handle->engine()->fileManager()->searchFor(textureFileName.c_str(), texturePath);
        }

        if (!exists) {
#ifdef DEBUG_MODE
            Logger::Throw("Error, file " + textureFileName + " not found");
#else
            Logger::LogError("Error, file " + textureFileName + " not found");
#endif
        }
    }
    else {
        texturePath = textureFileName.c_str();
    }

    return texturePath;
}

void Material::getTextureData(MaterialTextureData& outData) const
{
    for (const auto& child : m_handle->children()) {
        if (Texture* texture = child->resourceAs<Texture>()) {
            size_t idx = (size_t)texture->getUsageType();
            if (idx >= outData.m_data.size()) {
                outData.m_data.resize((size_t)texture->getUsageType());
            }
            outData.m_data[idx] = texture->asTexdata();
        }
    }
}

std::shared_ptr<ResourceHandle> Material::getTexture(TextureUsageType type)
{
    // See if material has this type of texture
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != EResourceType::eTexture)
            Logger::Throw("Wrong resource type");
        auto texture = handle->resourceAs<Texture>();
        return texture->getUsageType() == type;
    });

    if (iter == m_handle->children().end())
        Logger::Throw("Error, texture of the specified type not found");

    return *iter;
}

void Material::setTexture(std::shared_ptr<rev::ResourceHandle> resourceHandle, 
    TextureUsageType type)
{
    if (resourceHandle->getResourceType() != EResourceType::eTexture) {
        Logger::Throw("Error, resource handle is of the incorrect type");
    }
    if (hasType(type)) Logger::Throw("Error, texture type already present in material");

    m_handle->addChild(resourceHandle);
}

void Material::setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram, RenderContext& context)
{
    // TODO: Use a UBO, containing the current material
    // OR, at least check that material doesn't need to be bound again
    static const GLint s_maxNumTexUnits = (GLint)gl::OpenGLFunctions::MaxNumTextureUnitsPerShader();

    // Array of texture types and whether or not they are being used
    static constexpr Uint32_t s_numTypes = (Uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE;
    static std::array<bool, s_numTypes> s_usedTypes;
    for (uint32_t i = 0; i < s_numTypes; i++) {
        s_usedTypes[i] = false;
    }

    // Set uniforms for textures
    int texUnit = s_reservedTextureCount; // Start after reserved units (shadow maps + ssao texture)

    // Bind blank texture (it's reserved)
    int blankTexUnit = texUnit;
    context.blankTexture().bind(blankTexUnit);
    texUnit++;

    // Bind material textures
    UniformContainer& uc = context.uniformContainer();
    Uint32_t textureUniformId;
    for (const auto& textureHandle : m_handle->children()) {
        // Bind each texture to a texture unit
        if (textureHandle->getResourceType() != EResourceType::eTexture) {
            Logger::Throw("Error, resource is not a texture");
        }

        // Continue if resource is loading
        if (textureHandle->isLoading()) {
            continue;
        }

        // Lock resource handle
        QMutexLocker locker(&textureHandle->mutex());
        Texture* texture = textureHandle->resourceAs<Texture>();

        // Return if texture not yet loaded
        if (!texture) { 
            continue; 
        }
#ifdef DEBUG_MODE
        if (!textureHandle->isConstructed()) {
            Logger::Throw("Error, texture should be constructed");
        }
#endif

        texture->bind(texUnit); // bind texture to this unit

        // Set corresponding uniform in the shader program to this texture
        TextureUsageType textureType = texture->getUsageType();
        Uint32_t textureTypeInt = (Uint32_t)textureType;
        s_usedTypes[textureTypeInt] = true;

        if (shaderProgram.hasTextureUniform(textureTypeInt, textureUniformId)) {
            /// @todo Set texture unit when textures are loaded
            UniformData& uniform = m_materialUniforms.m_textureUnits[textureTypeInt];
            uniform.setValue(texUnit, uc);
            shaderProgram.setUniformValue(textureUniformId, uniform);
        }

        // Increment texture unit
        texUnit++;

        if (texUnit >= s_maxNumTexUnits) {
            Logger::Throw("Error, exceeded number of allowed textures");
        }
    }

    // Set unused textures to blank
    for (uint32_t i = 0; i < s_numTypes; i++) {
        // Set texture to blank if not using the specified type
        if (s_usedTypes[i]) { 
            continue; 
        }

        // Skip uniform if not implemented in shader
        if (!shaderProgram.hasTextureUniform(i, textureUniformId)) {
            continue;
        }

        shaderProgram.setUniformValue(textureUniformId, m_materialUniforms.m_blankTextureUnit);
    }

    // Set uniforms for lighting attributes
    Int32_t materialSpecUniformId = shaderProgram.uniformMappings().m_materialSpecularity;
    if (materialSpecUniformId != -1) {
        /// @todo Set only when values are changed for material
        m_materialUniforms.m_shininess.setValue(m_properties.m_shininess, uc);
        m_materialUniforms.m_specularity.setValue(m_properties.m_specularity, uc);

        Int32_t materialShininessUniformId = shaderProgram.uniformMappings().m_materialShininess;
        shaderProgram.setUniformValue(materialSpecUniformId, m_materialUniforms.m_specularity);
        shaderProgram.setUniformValue(materialShininessUniformId, m_materialUniforms.m_shininess);
    }

}

//bool operator==(const Material & m1, const Material & m2)
//{
//    bool sameTextures = true;
//    if (m1.m_textures.size() != m2.m_textures.size()) {
//        // Texture maps are not the same size, so materials cannot be equivalent
//        sameTextures = false;
//    }
//    for (const auto& texturePair: m1.m_textures) {
//        // Iterate through textures to make sure they are the same
//        if (m2.m_textures.find(texturePair.first) != m2.m_textures.end()) {
//            sameTextures &= texturePair.second == m2.m_textures.at(texturePair.first);
//        }
//        else {
//            sameTextures &= false;
//        }
//    }
//
//    return sameTextures;
//}

uint32_t Material::s_materialCount = 0;



// End namespaces
}
#include "GMaterial.h"

// Qt
#include <QDebug>

// internal
#include "../../utils/GMemoryManager.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../shaders/GShaderProgram.h"
#include "../../processes/GProcess.h"
#include "../../readers/GJsonReader.h"
#include "../GGLFunctions.h"
//#include "QGLFramebufferObject"
#include "../../encoding/GProtocol.h"
#include "../lighting/GShadowMap.h"
#include "../lighting/GLightSettings.h"
#include "../renderer/GRenderContext.h"
#include "../../resource/GFileManager.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MaterialProperties
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialProperties::MaterialProperties()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialProperties::~MaterialProperties()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//QJsonValue MaterialProperties::asJson(const SerializationContext& context) const
//{
//    QJsonObject object;
//    object.insert("shininess", m_shininess);
//    object.insert("specularity", m_specularity.asJson());
//
//    return object;
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//void MaterialProperties::loadFromJson(const QJsonValue& json, const SerializationContext& context)
//{
//    const QJsonObject& object = json.toObject();
//    m_shininess = object.value("shininess").toDouble();
//    m_specularity = Vector3g(object.value("specularity"));
//}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MaterialData
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialData::MaterialData() {
    initialize();
}
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialData::~MaterialData()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialDataProtocol* MaterialData::createProtocol()
{
    // Create protocol with members
    // Note, pointers must be passed as a ProtocolField, since they require a count
    GStringProtocolField nameField(m_name.createProtocolField());
    GStringProtocolField pathField(m_path.createProtocolField());
    //GStringProtocolField nameField(m_name.c_str(), m_name.length());
    //GStringProtocolField pathField(m_path.c_str(), m_path.length());
    MaterialDataProtocol* protocol =  new MaterialDataProtocol(
        nameField,
        pathField,
        m_id, 
        m_properties, 
        m_textureData);

    return protocol;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MaterialData::createProtocol(MaterialDataProtocol & outProtocol)
{
    // Create protocol with members
    // Note, pointers must be passed as a ProtocolField, since they require a count
    GStringProtocolField nameField(m_name.createProtocolField());
    GStringProtocolField pathField(m_path.createProtocolField());
    //GStringProtocolField nameField(m_name.c_str(), m_name.length());
    //GStringProtocolField pathField(m_path.c_str(), m_path.length());
    outProtocol = MaterialDataProtocol(
        nameField,
        pathField,
        m_id,
        m_properties,
        m_textureData);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MaterialData::initialize()
{
    m_name = "";
    m_properties = MaterialProperties();
}
/////////////////////////////////////////////////////////////////////////////////////////////
const TextureData & MaterialData::textureData(TextureUsageType type) const
{
    // Add texture data to material data
    switch (type) {
    case TextureUsageType::kAmbient:
        return m_textureData.m_ambientTexture;
    case TextureUsageType::kDiffuse:
        return m_textureData.m_diffuseTexture;
    case TextureUsageType::kNormalMap:
        return m_textureData.m_normalTexture;
    case TextureUsageType::kSpecular:
        return m_textureData.m_specularTexture;
    case TextureUsageType::kBump:
        return m_textureData.m_bumpTexture;
    case TextureUsageType::kSpecularHighlight:
        return m_textureData.m_specularHighlightTexture;
    case TextureUsageType::kOpacity:
        return m_textureData.m_opacityTexture;
    case TextureUsageType::kDisplacement:
        return m_textureData.m_displacementTexture;
    case TextureUsageType::kReflection:
        return m_textureData.m_reflectionTexture;
    case TextureUsageType::kLightMap:
        return m_textureData.m_lightmapTexture;

    // PBR textures
    case TextureUsageType::kAlbedo_PBR:
        return m_textureData.m_albedoTexture;
    case TextureUsageType::kBump_PBR:
        return m_textureData.m_pbrBumpMapping;
    case TextureUsageType::kEmissive_PBR:
        return m_textureData.m_emissiveTexture;
    case TextureUsageType::kMetallic_PBR:
        return m_textureData.m_metallicTexture;
    case TextureUsageType::kRoughness_PBR:
        return m_textureData.m_roughnessTexture;
    case TextureUsageType::kAmbientOcclusion_PBR:
        return m_textureData.m_ambientOcclusionTexture;
    default:
#ifdef DEBUG_MODE
        throw("Texture type not recognized");
#endif
        break;
    }

    return m_textureData.m_unknown;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Material
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Material::CreateHandle(CoreEngine * engine, const GString & mtlName)
{
    // Create handle for material
    auto handle = ResourceHandle::create(engine,
        ResourceType::kMaterial);
    //handle->setResourceType(ResourceType::kMaterial);
    handle->setRuntimeGenerated(true); // material is generated in engine
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_unique<Material>();
    handle->setResource(std::move(mtl), false);

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::clearCount()
{
    s_materialCount = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material():
    m_sortIndex(s_materialCount++)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Material::Material(const QString & name) :
//    Resource(name),
//    m_sortIndex(s_materialCount++)
//{
//}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::~Material()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Material::getSortID() const
{
    return m_sortIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Material::isBound(RenderContext & context) const
{
    return context.boundMaterial() == m_sortIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setData(const MaterialData & data)
{
    // Initialize properties
    //m_properties = data.m_properties;
    m_data = data;

    // Initialize textures
    initializeTexture(int(TextureUsageType::kDiffuse), data);
    initializeTexture(int(TextureUsageType::kNormalMap), data);
    initializeTexture(int(TextureUsageType::kSpecular), data);

    // TODO: Load options
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Material::getSearchDirectory() const
{
    QFileInfo info = QFileInfo(m_handle->getPath().c_str());
    return info.absoluteDir().absolutePath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setDiffuseTexture(std::shared_ptr<rev::ResourceHandle> diffuse)
{
	setTexture(diffuse, TextureUsageType::kDiffuse);
}

/////////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////////
void Material::release(RenderContext& context)
{
    Q_UNUSED(context);
    // 10/6/2020: Unnecessary, is overwritten by bind
	//for (const auto& textureHandle : m_handle->children()) {
 //       if (!textureHandle->getResourceType() == ResourceType::kTexture)
 //           throw("Error, material has a child resource that is not a texture");
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
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Material::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    // Store textures
    QJsonObject textures;
    for (const std::shared_ptr<rev::ResourceHandle>& textureHandle : m_handle->children()) {
        int textureType = int(textureHandle->resourceAs<Texture>()->getUsageType());
        textures.insert(QString::number(textureType), textureHandle->getName().c_str());
    }
    object.insert("textures", textures);

    // Store other attributes
    //object.insert("name", m_name.c_str());

    // Properties
    QJsonObject propertiesObject;
    propertiesObject.insert("shininess", m_data.m_properties.m_shininess);
    propertiesObject.insert("specularity", m_data.m_properties.m_specularity.asJson());
    object.insert("properties", propertiesObject);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    if (!context.m_engine) {
        throw("Error, engine required");
    }

    const QJsonObject& object = json.toObject();

    // Load texture
    QString jsonStr = JsonReader::ToString<QString>(json);
    const QJsonObject& textures = object.value("textures").toObject();
    for(const QString& key: textures.keys()) {
        QString texHandleName = textures.value(key).toString();
        //TextureUsageType texType = TextureUsageType(key.toInt());
        auto texHandle = context.m_engine->resourceCache()->getHandleWithName(texHandleName, ResourceType::kTexture);
#ifdef DEBUG_MODE
        if (!texHandle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(texHandle);
    }

    // Load name
    //m_name = object.value("name").toString();
    //m_handle->setName(m_name); // Nice for names to match for book-keeping

    // Load properties
    QJsonObject properties = object.value("properties").toObject();
    m_data.m_properties.m_shininess = properties.value("shininess").toDouble();
    m_data.m_properties.m_specularity = Vector3(properties.value("specularity"));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Material::hasType(TextureUsageType type)
{
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != ResourceType::kTexture)
            throw("Wrong resource type");
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
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::initializeTexture(int intType, const MaterialData & data)
{
    // Get texture data of the given type
    TextureUsageType textureType = TextureUsageType(intType);
    const TextureData& texData = data.textureData(textureType);
    GStringView textureFileName = texData.fileName();
    if (textureFileName.isEmpty()) {
        // If no texture name, texture not loaded
        return;
    }

    // Get texture path from material data
    QString texturePath;
    if (!QFile::exists(textureFileName)) {
        // If filepath is not fully specified, walk directory structure to find
        QString dir = getSearchDirectory();
        bool exists = FileReader::FileExists(dir, textureFileName.c_str(), texturePath);

        // If doesn't exist, use file manager to look through all search paths
        if (!exists) {
            exists = m_handle->engine()->fileManager()->searchFor(textureFileName.c_str(), texturePath);
        }

        if (!exists) {
#ifdef DEBUG_MODE
            throw("Error, file " + textureFileName + " not found");
#else
            logError("Error, file " + textureFileName + " not found");
#endif
        }
    }
    else {
        texturePath = textureFileName.c_str();
    }

    // Create handle to load texture from material data
    std::shared_ptr<ResourceHandle> textureHandle =
        Texture::CreateHandle(texturePath, textureType, *m_handle);

    // Load texture
    textureHandle->loadResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Material::getTexture(TextureUsageType type)
{
    // See if material has this type of texture
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != ResourceType::kTexture)
            throw("Wrong resource type");
        auto texture = handle->resourceAs<Texture>();
        return texture->getUsageType() == type;
    });

    if (iter == m_handle->children().end())
        throw("Error, texture of the specified type not found");

    return *iter;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setTexture(std::shared_ptr<rev::ResourceHandle> resourceHandle, 
    TextureUsageType type)
{
    if (resourceHandle->getResourceType() != ResourceType::kTexture) {
        throw("Error, resource handle is of the incorrect type");
    }
    if (hasType(type)) throw("Error, texture type already present in material");

    m_handle->addChild(resourceHandle);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram, RenderContext& context)
{
    // TODO: Use a UBO, containing the current material
    // OR, at least check that material doesn't need to be bound again
    GLint maxNumTexUnits = (GLint)GL::OpenGLFunctions::MaxNumTextureUnits();

    // Array of texture types and whether or not they are being used
    std::array<bool, (uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE> usedTypes;
    uint32_t numTypes = (uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE;
    for (uint32_t i = 0; i < numTypes; i++) {
        usedTypes[i] = false;
    }

    // Set uniforms for textures
    int texUnit = NUM_SHADOW_MAP_TEXTURES + 1; // Start after reserved units (shadow maps + ssao texture)

    // Bind blank texture (it's reserved)
    int blankTexUnit = texUnit;
    context.blankTexture().bind(blankTexUnit);
    texUnit++;

    // Bind material textures
    for (const auto& textureHandle : m_handle->children()) {
        // Bind each texture to a texture unit
        if (textureHandle->getResourceType() != ResourceType::kTexture) {
            throw("Error, resource is not a texture");
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
            throw("Error, texture should be constructed");
        }
#endif

        texture->bind(texUnit); // bind texture to this unit

        usedTypes[(int)texture->getUsageType()] = true;

        // Set corresponding uniform in the shader program to this texture
        const GString& texUniformName = Texture::GetUniformName(texture->getUsageType());
        shaderProgram.setUniformValue(texUniformName, texUnit);

        // Increment texture unit
        texUnit++;

        if (texUnit >= maxNumTexUnits) {
            throw("Error, exceeded number of allowed textures");
        }
    }

    // Set unused textures to blank
    for (uint32_t i = 0; i < numTypes; i++) {
        // Set texture to blank if not using the specified type
        if (usedTypes[i]) { 
            continue; 
        }

        // Skip uniform if not implemented in shader
        GStringView texUniformName(Texture::GetUniformName((TextureUsageType)i));
        if (!shaderProgram.hasUniform(texUniformName)) {
            continue;
        }

        shaderProgram.setUniformValue(texUniformName, blankTexUnit);
    }

    // Set uniforms for lighting attributes
    shaderProgram.setUniformValue("material.specularity", m_data.m_properties.m_specularity);
    shaderProgram.setUniformValue("material.shininess", (real_g)m_data.m_properties.m_shininess);

}
///////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////
uint32_t Material::s_materialCount = 0;


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
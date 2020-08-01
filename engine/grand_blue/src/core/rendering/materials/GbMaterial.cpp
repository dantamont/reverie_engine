#include "GbMaterial.h"

// Qt
#include <QDebug>

// internal
#include "../../utils/GbMemoryManager.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../shaders/GbShaders.h"
#include "../../processes/GbProcess.h"
#include "../../readers/GbJsonReader.h"
#include "../GbGLFunctions.h"
#include "QGLFramebufferObject"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// MaterialProperties
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialProperties::MaterialProperties()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialProperties::MaterialProperties(const QJsonValue & json)
{
    loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
MaterialProperties::~MaterialProperties()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue MaterialProperties::asJson() const
{
    QJsonObject object;
    object.insert("shininess", m_shininess);
    object.insert("specularity", m_specularity.asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void MaterialProperties::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    m_shininess = object.value("shininess").toDouble();
    m_specularity = Vector3g(object.value("specularity"));
}




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
void MaterialData::initialize()
{
    m_name = "";
    m_properties = MaterialProperties();
}
/////////////////////////////////////////////////////////////////////////////////////////////
TextureData & MaterialData::textureData(Texture::TextureType type)
{
    // Add texture data to material data
    switch (type) {
    case Texture::kAmbient:
        return m_ambientTexture;
    case Texture::kDiffuse:
        return m_diffuseTexture;
    case Texture::kNormalMap:
        return m_normalTexture;
    case Texture::kSpecular:
        return m_specularTexture;
    case Texture::kBump:
        return m_bumpTexture;
    case Texture::kSpecularHighlight:
        return m_specularHighlightTexture;
    case Texture::kOpacity:
        return m_opacityTexture;
    case Texture::kDisplacement:
        return m_displacementTexture;
    case Texture::kReflection:
        return m_reflectionTexture;
    case Texture::kLightMap:
        return m_lightmapTexture;

    // PBR textures
    case Texture::kAlbedo_PBR:
        return m_albedoTexture;
    case Texture::kBump_PBR:
        return m_pbrBumpMapping;
    case Texture::kEmissive_PBR:
        return m_emissiveTexture;
    case Texture::kMetallic_PBR:
        return m_metallicTexture;
    case Texture::kRoughness_PBR:
        return m_roughnessTexture;
    case Texture::kAmbientOcclusion_PBR:
        return m_ambientOcclusionTexture;
    default:
#ifdef DEBUG_MODE
        throw("Texture type not recognized");
#endif
        break;
    }

    return m_unknown;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Material
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Material::createHandle(CoreEngine * engine, const QString & mtlName)
{
    // Create handle for material
    auto handle = ResourceHandle::create(engine,
        Resource::kMaterial);
    //handle->setResourceType(Resource::kMaterial);
    handle->setUserGenerated(true); // material is generated in engine
    handle->setName(mtlName);

    // Create material
    auto mtl = prot_make_shared<Material>(engine, mtlName);
    handle->setResource(mtl, false);

    return handle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::clearCount()
{
    s_materialCount = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine* engine):
    Resource(kMaterial),
    m_engine(engine),
    m_sortID(s_materialCount++)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine* core, const QString & name) :
    Resource(name, kMaterial),
    m_engine(core),
    m_sortID(s_materialCount++)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////
//Material::Material(CoreEngine * core, MaterialData && data) :
//    Serializable(),
//    Resource(QString::fromStdString(data.m_name).toLower(), kMaterial),
//    m_engine(core),
//    m_sortID(s_materialCount++)
//{
//    setData(std::move(data));
//}
///////////////////////////////////////////////////////////////////////////////////////////////
//Material::Material(CoreEngine* core, const QString & name, const std::shared_ptr<ResourceHandle>& diffuse) :
//    Resource(name, kMaterial),
//    m_engine(core),
//    m_sortID(s_materialCount++)
//{
//    setTexture(diffuse, Texture::kDiffuse);
//}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::~Material()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Material::getSortID() const
{
    return m_sortID;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setData(MaterialData && data)
{
    // Initialize properties
    m_properties = data.m_properties;

    // Initialize textures
    initializeTexture(int(Texture::kDiffuse), data);
    initializeTexture(int(Texture::kNormalMap), data);

    // TODO: Load options
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Material::inResourceCache() const
{
    return m_engine->resourceCache()->getHandle(m_uuid) != nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Material::getSearchDirectory() const
{
    QFileInfo info = QFileInfo(m_handle->getPath());
    return info.absoluteDir().absolutePath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setDiffuseTexture(std::shared_ptr<Gb::ResourceHandle> diffuse)
{
	setTexture(diffuse, Texture::kDiffuse);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Material::bind(ShaderProgram& shaderProgram)
{
    // Set uniforms and bind textures
    setShaderUniformsAndBindTextures(shaderProgram);

    //// Update uniforms in GL if the shader program isn't null
    //if (shaderProgram) {
    //    // updateUniforms is an expensive call, and this is redundant
    //    // Update the uniforms in GL
    //    // shaderProgram->updateUniforms();
    //}
    //else {
    //    throw("Error, no shader program linked to material");
    //}

    // Set bound status
    m_isBound = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Material::release()
{
	for (const auto& textureHandle : m_handle->children()) {
        if (!textureHandle->getResourceType() == Resource::kTexture)
            throw("Error, material has a child resource that is not a texture");
        if (!textureHandle->resource(false)) continue;
        QMutexLocker locker(&textureHandle->mutex());
        if (textureHandle->isConstructed()) 
            textureHandle->resourceAs<Texture>(false)->release();
	}
	m_isBound = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::onRemoval(ResourceCache * cache)
{
    Q_UNUSED(cache)
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Material::asJson() const
{
    QJsonObject object;

    // Store textures
    QJsonObject textures;
    for (const std::shared_ptr<Gb::ResourceHandle>& textureHandle : m_handle->children()) {
        int textureType = int(textureHandle->resourceAs<Texture>(false)->getType());
        textures.insert(QString::number(textureType), textureHandle->getName());
    }
    object.insert("textures", textures);

    // Store other attributes
    object.insert("name", m_name);
    object.insert("properties", m_properties.asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();

    // Load texture
    QString jsonStr = JsonReader::ToQString(json);
    const QJsonObject& textures = object.value("textures").toObject();
    for(const QString& key: textures.keys()) {
        QString texHandleName = textures.value(key).toString();
        //Texture::TextureType texType = Texture::TextureType(key.toInt());
        auto texHandle = m_engine->resourceCache()->getHandleWithName(texHandleName, Resource::kTexture);
#ifdef DEBUG_MODE
        if (!texHandle) throw("Error, handle not found for given name");
#endif
        m_handle->addChild(texHandle);
    }

    // Load other attributes
    // Move the material in the resource cache map if it's been renamed
    m_name = object.value("name").toString();
    m_properties = MaterialProperties(object.value("properties"));
    m_handle->setName(m_name); // Nice for names to match for book-keeping
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Material::hasType(Texture::TextureType type)
{
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != Resource::kTexture)
            throw("Wrong resource type");
        auto texture = handle->resourceAs<Texture>(false);
        return texture->getType() == type;
    });

    if (iter == m_handle->children().end()) {
        return false;
    }
    else {
        return true;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::initializeTexture(int intType, MaterialData & data)
{
    // Get texture data of the given type
    Texture::TextureType textureType = Texture::TextureType(intType);
    TextureData& texData = data.textureData(textureType);
    if (texData.m_textureFileName.size() == 0) {
        // If no texture name, texture not loaded
        return;
    }

    // Get texture path from material data
    QString filename = QString::fromStdString(texData.m_textureFileName);
    QString dir;
    QString texturePath;
    if (texData.m_textureFileDir.empty()) {
        // If there is no directory specified, walk directory structure to find
        dir = getSearchDirectory();
        bool exists = FileReader::fileExists(dir, filename, texturePath);
        if (!exists) {
            throw("Error, file not found");
        }
    }
    else {
        dir = QString(texData.m_textureFileDir.c_str());
        texturePath = QDir::cleanPath(dir + QDir::separator() + filename);
    }

    // Create handle to load texture from material data
    std::shared_ptr<ResourceHandle> textureHandle =
        Texture::createHandle(texturePath, textureType, *m_handle);

    // Load texture
    textureHandle->loadResource();
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Material::getTexture(Texture::TextureType type)
{
    // See if material has this type of texture
    auto iter = std::find_if(m_handle->children().begin(), m_handle->children().end(),
        [&](const std::shared_ptr<ResourceHandle>& handle) {
        if (handle->getResourceType() != Resource::kTexture)
            throw("Wrong resource type");
        auto texture = handle->resourceAs<Texture>(false);
        return texture->getType() == type;
    });

    if (iter == m_handle->children().end())
        throw("Error, texture of the specified type not found");

    return *iter;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setTexture(std::shared_ptr<Gb::ResourceHandle> resourceHandle, 
    Texture::TextureType type)
{
    if (resourceHandle->getResourceType() != Resource::kTexture) {
        throw("Error, resource handle is of the incorrect type");
    }
    if (hasType(type)) throw("Error, texture type already present in material");

    m_handle->addChild(resourceHandle);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setShaderUniformsAndBindTextures(ShaderProgram& shaderProgram)
{
    // Set uniforms for textures
    int texUnit = 0;
    GL::OpenGLFunctions gl = GL::OpenGLFunctions();
    bool useDiffuseTexture = false;
    bool useNormalMap = false;
    for (const auto& textureHandle : m_handle->children()) {
        // Bind each texture to a texture unit
        if (textureHandle->getResourceType() != Resource::kTexture)
            throw("Error, resource is not a texture");


        // Continue if resource is loading
        if (textureHandle->isLoading()) {
            continue;
        }

        // Lock resource handle
        QMutexLocker locker(&textureHandle->mutex());
        auto texture = textureHandle->resourceAs<Texture>(false);

        // Return if texture not yet loaded
        if (!texture) 
            continue;
#ifdef DEBUG_MODE
        if (!textureHandle->isConstructed()) 
            throw("Error, texture should be constructed");
#endif
        gl.glActiveTexture(GL_TEXTURE0 + texUnit); // set active texture unit
        texture->bind(texUnit); // bind texture to this unit

        // Texture-type specific functions
        switch (texture->m_type) {
        case Texture::kDiffuse:
            useDiffuseTexture = true;
            break;
        case Texture::kNormalMap:
            useNormalMap = true;
            break;
        default:
            break;
        }

        // Set corresponding uniform in the shader program to this texture
        const QString& texUniformName = Texture::getUniformName(texture->m_type);
        shaderProgram.setUniformValue(texUniformName, texUnit);

        // Increment texture unit
        texUnit++;
    }

    // Set more uniforms for textures
    shaderProgram.setUniformValue("material.useDiffuseTexture", useDiffuseTexture);
    shaderProgram.setUniformValue("material.useNormalMap", useNormalMap);

    // Set uniforms for lighting attributes
    shaderProgram.setUniformValue("material.specularity", m_properties.m_specularity);
    shaderProgram.setUniformValue("material.shininess", (real_g)m_properties.m_shininess);

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
size_t Material::s_materialCount = 0;


/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
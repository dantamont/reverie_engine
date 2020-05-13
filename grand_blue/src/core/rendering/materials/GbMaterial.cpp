#include "GbMaterial.h"

// Qt
#include <QDebug>

// internal
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../shaders/GbShaders.h"
#include "../../processes/GbProcess.h"
#include "../../readers/GbJsonReader.h"
#include "../GbGLFunctions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
TextureData::TextureData(bool isBump)
{
    initialize(isBump);
}
/////////////////////////////////////////////////////////////////////////////////////////////
TextureData::~TextureData()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void TextureData::initialize(bool isBump)
{
    if (isBump) {
        m_properties.m_imfChannel = 'l';
    }
    else {
        m_properties.m_imfChannel = 'm';
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Texture
/////////////////////////////////////////////////////////////////////////////////////////////
const std::unordered_map<Texture::TextureType, QString> Texture::TYPE_TO_UNIFORM_MAP = {
    {kDiffuse, "material.diffuseTexture"},
    {kNormalMap, "material.normalMap"}
};

/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const QString & filePath, TextureType type):
    Resource(kTexture),
    m_texture(QOpenGLTexture::Target2D), // Does not create underlying OpenGL object, so safe without context
    m_type(type)
{
    // Load image
    initialize(filePath);

    // Run post-construction if on the main thread, otherwise return
    if (Process::isMainThread()) {
        postConstruction();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const Image & image, TextureType type,
    QOpenGLTexture::Filter minFilter,
    QOpenGLTexture::Filter magFilter,
    QOpenGLTexture::WrapMode wrapMode):
    Resource(kTexture),
    m_image(image),
    m_texture(QOpenGLTexture::Target2D), // Does not create underlying OpenGL object, so safe without context
    m_type(type),
    m_minFilter(minFilter),
    m_magFilter(magFilter),
    m_wrapMode(wrapMode)
{
    setCost();

    // Run post-construction if on the main thread, otherwise return
    if (Process::isMainThread()) {
        postConstruction();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Texture::~Texture()
{
    // Destroy underlying GL texture on destruction
    // Requires a current valid OpenGL context
    //if (!QOpenGLContext::currentContext()) {
    //    throw("Error, no current OpenGL context found");
    //}
    destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::bind(int texUnit)
{
    if (texUnit > -1) {
        // Texture unit was specified
        m_texture.bind(size_t(texUnit));
    }
    else {
        m_texture.bind();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    // Destroy GL-side texture on resource removal
    destroy();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::postConstruction()
{
#ifdef DEBUG_MODE
    logConstructedWarning();
    if (m_isConstructed) {
        return;
    }
    GL::OpenGLFunctions functions;
    bool error = functions.printGLError("Error before constructing texture");
#else
    if (m_isConstructed) {
        return;
    }
#endif

    // Set image for GL texture, mirrored to have (0,0) texture coordinate at the lower left
    m_texture.setData(m_image.m_image);

    // Set mipmap generation settings
    m_texture.setMinificationFilter(m_minFilter);
    m_texture.setMagnificationFilter(m_magFilter);
    m_texture.setWrapMode(m_wrapMode);

    // Print any GL errors
#ifdef DEBUG_MODE
    error = functions.printGLError("Error post-constructing texture");
#endif
	
	// Call parent class construction routine
	Resource::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::setCost()
{
    // Cost is twice the size of image in Mb, since image is also copied by GL
    // See: https://community.khronos.org/t/does-glteximage2d-copy-the-texture-data/1952
    // TODO: Possible remove image as a member
    if (!m_image.m_image.isNull()) {
        m_cost = 2 * m_image.sizeInMegaBytes();
    }
    else {
        m_cost = 0;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Texture::initialize(const QString& filePath)
{
    // Load image from file path and check validity
    // Need to flip image due to Qt image convention
    // Mirrored to have (0,0) texture coordinate at the lower left
    bool exists = QFile(filePath).exists();
    if (exists) {
        // For .tga: https://forum.qt.io/topic/74712/qimage-from-tga-with-alpha/11
        // https://forum.qt.io/topic/101971/qimage-and-tga-support-in-c/5
        //QImage qImage = QImage(filePath);
        //QImageReader reader(filePath);
        //QImage qImage = reader.read();
        m_image = Gb::Image(filePath);
        m_image.m_image = m_image.m_image.mirrored(false, true);
    }

    setCost();
  
}



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
Material::Material(CoreEngine* engine, const QJsonValue & json):
    m_engine(engine)
{
    loadFromJson(json);
}

/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine* core, const QString & name) :
    Object(name),
    m_engine(core)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine * core, MaterialData & data):
    Loadable(QString::fromStdString(data.m_path)),
    Object(QString::fromStdString(data.m_name).toLower()),
    m_engine(core)
{
    // Initialize properties
    m_properties = data.m_properties;

    // Initialize textures
    initializeTexture(int(Texture::kDiffuse), data);
    initializeTexture(int(Texture::kNormalMap), data);

    // TODO: Load options
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine * core, MaterialData && data) :
    Loadable(QString::fromStdString(data.m_path)),
    Object(QString::fromStdString(data.m_name).toLower()),
    m_engine(core)
{
    // Initialize properties
    m_properties = data.m_properties;

    // Initialize textures
    initializeTexture(int(Texture::kDiffuse), data);
    initializeTexture(int(Texture::kNormalMap), data);

    // TODO: Load options
}
/////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(CoreEngine* core, const QString & name, const std::shared_ptr<ResourceHandle>& diffuse) :
    Object(name),
    m_engine(core)
{
    setTexture(diffuse, Texture::kDiffuse);
}

/////////////////////////////////////////////////////////////////////////////////////////////
Material::~Material()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::rename(const QString & name)
{
    // TODO: Make this a common mixin for resource-likes
    if (name == m_name) return;

    // Delete from resource cache map
    if (inResourceCache()) {
        auto pointer_to_this = m_engine->resourceCache()->getMaterial(m_name);
        m_engine->resourceCache()->removeMaterial(m_name);

        // Set the name
        setName(name);

        // Add back to resource cache map
        m_engine->resourceCache()->addMaterial(pointer_to_this);
    }
    else {
        setName(name);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Material::inResourceCache() const
{
    return m_engine->resourceCache()->hasMaterial(m_name);
}
/////////////////////////////////////////////////////////////////////////////////////////////
QString Material::getSearchDirectory() const
{
    QFileInfo info = QFileInfo(m_path);
    return info.absoluteDir().absolutePath();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setDiffuseTexture(std::shared_ptr<Gb::ResourceHandle> diffuse)
{
	setTexture(diffuse, Texture::kDiffuse);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Material::bind(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Set uniforms and bind textures
    setShaderUniformsAndBindTextures(shaderProgram);

    // Update uniforms in GL if the shader program isn't null
    if (shaderProgram) {
        // updateUniforms is an expensive call, and this is redundant
        // Update the uniforms in GL
        // shaderProgram->updateUniforms();
    }
    else {
        throw("Error, no shader program linked to material");
    }

    // Set bound status
    m_isBound = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Material::release()
{
	for (const auto& texturePair : m_textures) {
        std::shared_ptr<ResourceHandle> textureHandle = getTexture(texturePair.first);
        QMutexLocker locker(&textureHandle->mutex());
        auto texture = std::static_pointer_cast<Texture>(textureHandle->resource(false));
        if (texture && texture->isConstructed()) texture->release();
	}
	m_isBound = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Material::asJson() const
{
    QJsonObject object = Loadable::asJson().toObject();
    // Store textures
    QJsonObject textures;
    for (const std::pair<Texture::TextureType, std::shared_ptr<Gb::ResourceHandle>>& texturePair : m_textures) {
        textures.insert(QString::number(int(texturePair.first)), texturePair.second->asJson());
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

    // Load path
    Loadable::loadFromJson(json);

    // Load texture
    QString jsonStr = JsonReader::getJsonValueAsQString(json);
    const QJsonObject& textures = object.value("textures").toObject();
    for(const QString& key: textures.keys()) {
        QJsonValue textureJson = textures.value(key);
        Texture::TextureType texType = Texture::TextureType(key.toInt());
        std::shared_ptr<ResourceHandle> texHandle = 
            m_engine->resourceCache()->getResourceHandle(textureJson);
        m_textures[texType] = texHandle;
    }

    // Load other attributes
    // Move the material in the resource cache map if it's been renamed
    rename(object.value("name").toString());
    m_properties = MaterialProperties(object.value("properties"));
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

    // Load texture from data
    QString filename = QString::fromStdString(texData.m_textureFileName);
    QString dir;
    if (texData.m_textureFileDir.empty()) {
        dir = getSearchDirectory();
    }
    else {
        dir = QString(texData.m_textureFileDir.c_str());
    }
    QString texturePath = QDir::cleanPath(dir + QDir::separator() + filename);
    std::shared_ptr<ResourceHandle> textureHandle = 
        m_engine->resourceCache()->getTexture(texturePath, intType);
    Map::Emplace(m_textures, textureType, textureHandle);

}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> Material::getTexture(Texture::TextureType type)
{
    // See if material has this type of texture
    if (m_textures.find(type) == m_textures.end()) {
        throw("Error, texture type not found in texture map");
    }
    return m_textures.at(type);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setTexture(std::shared_ptr<Gb::ResourceHandle> resourceHandle, 
    Texture::TextureType type)
{
    if (resourceHandle->getType() != Resource::kTexture) {
        throw("Error, resource handle is of the incorrect type");
    }
    m_textures[type] = resourceHandle;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Material::setShaderUniformsAndBindTextures(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Set uniforms for textures
    int texUnit = 0;
    GL::OpenGLFunctions gl = GL::OpenGLFunctions();
    bool useDiffuseTexture = false;
    bool useNormalMap = false;
    for (const auto& texturePair : m_textures) {
        // Bind each texture to a texture unit
        std::shared_ptr<ResourceHandle> textureHandle = getTexture(texturePair.first);

        // Continue if resource is loading
        if (textureHandle->getIsLoading()) {
            continue;
        }

        // Lock resource handle
        QMutexLocker locker(&textureHandle->mutex());
        auto texture = std::static_pointer_cast<Texture>(textureHandle->resource(false));

        // Return if texture not yet loaded
        if (!texture) continue;
        if (!texture->isConstructed()) continue;
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
        if (shaderProgram) {
            const QString& texUniformName = Texture::getUniformName(texture->m_type);
            shaderProgram->setUniformValue(texUniformName, texUnit);
        }

        // Increment texture unit
        texUnit++;
    }

    // Set more uniforms for textures
    shaderProgram->setUniformValue("material.useDiffuseTexture", useDiffuseTexture);
    shaderProgram->setUniformValue("material.useNormalMap", useNormalMap);

    // Set uniforms for lighting attributes
    shaderProgram->setUniformValue("material.specularity", m_properties.m_specularity);
    shaderProgram->setUniformValue("material.shininess", (real_g)m_properties.m_shininess);

}
/////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Material & m1, const Material & m2)
{
    bool sameTextures = true;
    if (m1.m_textures.size() != m2.m_textures.size()) {
        // Texture maps are not the same size, so materials cannot be equivalent
        sameTextures = false;
    }
    for (const auto& texturePair: m1.m_textures) {
        // Iterate through textures to make sure they are the same
        if (m2.m_textures.find(texturePair.first) != m2.m_textures.end()) {
            sameTextures &= texturePair.second == m2.m_textures.at(texturePair.first);
        }
        else {
            sameTextures &= false;
        }
    }

    return sameTextures;
}



/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}
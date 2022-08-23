#include "geppetto/qt/widgets/graphics/GSpriteSheetWidget.h"

#include "fortress/json/GJson.h"
#include "fortress/image/GTexturePacker.h"
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"

#include "geppetto/qt/layer/types/GQtConverter.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"
#include "geppetto/qt/widgets/list/GResourceListView.h"

#include "enums/GTextureUsageTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GAddMaterialResourceMessage.h"
#include "ripple/network/messages/GAddTextureToMaterialMessage.h"
#include "ripple/network/messages/GAdvanceProgressWidgetMessage.h"
#include "ripple/network/messages/GRequestAddTexturesToMaterialMessage.h"

namespace rev {

/// @fixme Remove this duplicate function of Texture::GetUniformName once rendering stuff gets its own library
const GString& GetUniformName(GTextureUsageType type)
{
    /// Map of texture types and corresponding uniform name
    static const std::vector<GString> s_typeToUniformVec = {
        "material.diffuseTexture",
        "material.normalMap",
        "material.ambientTexture",
        "material.specularTexture",
        "material.specularHighlightTexture",
        "material.bumpMap",
        "material.displacementTexture",
        "material.opacityTexture",
        "material.reflectionTexture",
        "material.lightmapTexture",
        "material.albedoTexture_pbr",
        "material.bumpTexture_pbr",
        "material.ambientOcclusionTexture_pbr",
        "material.roughnessTexture_pbr",
        "material.metallicTexture_pbr",
        "material.shininessTexture_pbr",
        "material.emissiveTexture_pbr"
    };

    return s_typeToUniformVec.at((Int32_t)type);
}

SpriteSheetWidget::SpriteSheetWidget(WidgetManager* wm, QWidget *parent):
    ParameterWidget(wm, parent)
{
    initialize();
}


SpriteSheetWidget::~SpriteSheetWidget()
{
}

QImage SpriteSheetWidget::packTextures(const std::vector<GString>& filenames, GTextureUsageType usageType)
{
    assert(usageType < ETextureUsageType::eCOUNT && "Invalid usage type");

    size_t index = (size_t)usageType;

    // Make sure internal structures can accommodate texture type
    if (m_texturePackers.size() <= index) {
        m_texturePackers.resize(index);
    }

    bool cacheSpriteInfo = m_spriteInfo == nullptr;
    if (cacheSpriteInfo) {
        m_spriteInfo = std::make_shared<SpriteSheetInfo>();
    }

    // Load images and pack into texture
    uint32_t size = (uint32_t)filenames.size();
    uint32_t height = size / 2;
    Vector2u dims(size % 2 ? height + 1 : height, height);
    m_texturePackers.push_back(std::make_shared<TexturePacker>(Image::ColorFormat::kRGBA8888));
    for (const GString& filename : filenames) {
        Image image(filename, false, Image::ColorFormat::kRGBA8888);
        PackedTextureInfo info = m_texturePackers[index]->packTexture(image);
        if (cacheSpriteInfo) {
            m_spriteInfo->m_packedTextures.push_back(info);
        }
    }

    //image.save("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\pack.png");
    return QConverter::ToQt(m_texturePackers[index]->getImage());
}

void SpriteSheetWidget::generateMaterial()
{
    //Logger::LogDebug("Generating material");


    // Make sure output directory exists
    QDir dir(m_outputDirectory->getPath());
    if (!dir.exists()) {
        dir.mkpath(m_outputDirectory->getPath());
    }

    // Request create material
    static GAddMaterialResourceMessage s_message;
    m_materialId = Uuid();
    s_message.setUuid(m_materialId);
    s_message.setName(m_materialName->text().toStdString());
    s_message.setFilePath((m_outputDirectory->getPath() + QDir::separator() + m_materialName->text() + ".mtl").toStdString());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);

    // TODO: Make textures child of material, but actually write material
    // to a file for direct load.
    // See unimplemented branch of LoadProcess
    // Create material
    //auto handle = Material::CreateHandle(m_engine, m_materialName->text());
    //handle->setStatusFlags(ResourceStatusFlag::kIsLoading);
    //ResourceCache::Instance().incrementLoadCount();
    //handle->setPath(m_outputDirectory->getPath() + QDir::separator() + m_materialName->text() + ".mtl");
    //Material* material = handle->resourceAs<Material>();
    //progress.setValue(currentProgress++);

    //// Save textures and create as resources
    //progress.setValue(currentProgress++);
    //std::vector<QString> textureFilePaths;
    //for (size_t i = 0; i < m_texturePackers.size(); i++) {
    //    const std::shared_ptr<TexturePacker>& packer = m_texturePackers[i];
    //    if (!packer) {
    //        textureFilePaths.push_back(QString());
    //        continue;
    //    }
    //    TextureUsageType texType = (TextureUsageType)i;

    //    // Generate unique texture name to save texture to file
    //    Image image = packer->getImage();
    //    GString textureName = GString(Texture::GetUniformName(texType)).replace("material.", "");
    //    textureName += ".png";
    //    textureName += handle->getUuid().asString() + ".png";
    //    QString filepath = QDir::cleanPath(m_outputDirectory->getPath() +
    //        QDir::separator() + textureName.c_str());
    //    image.save(filepath);
    //    textureFilePaths.push_back(filepath);
    //    progress.setValue(currentProgress++);
    //}

    // Create material
    //auto handle = Material::CreateHandle(m_engine, m_materialName->text());
    //handle->setStatusFlags(ResourceStatusFlag::kIsLoading);
    //ResourceCache::Instance().incrementLoadCount();
    //handle->setPath(m_outputDirectory->getPath() + QDir::separator() + m_materialName->text() + ".mtl");
    //Material* material = handle->resourceAs<Material>();
    //progress.setValue(currentProgress++);
    //// Create texture handle, loading it serially
    //std::shared_ptr<ResourceHandle> textureHandle =
    //    Texture::CreateHandle(m_engine, GString(filepath), texType, true);

    //// Create material data from textures
    //MaterialData data;
    //data.m_name = handle->getName();
    //data.m_id = material->getSortID();
    //data.m_textureData.m_data.resize((size_t)ETextureUsageType::eCOUNT);
    //for (size_t i = 0; i < m_texturePackers.size(); i++) {
    //    QString texFilePath = textureFilePaths[i];
    //    if (texFilePath.isEmpty()) {
    //        continue;
    //    }
    //    TextureUsageType texType = (TextureUsageType)i;
    //    TextureData& texData = data.m_textureData.m_data[i];
    //    GString texName = FileReader::PathToName(texFilePath, true, true);
    //    texData.setFileName(texName);
    //    texData.m_properties.m_usageType = texType;
    //    // Make texture a child of the material
    //    handle->addChild(textureHandle);

    //    progress.setValue(currentProgress++);
    //}
    //progress.setValue(currentProgress++);

    //// Load textures as resources via material data
    //// Note, need to load textures on this thread, or updating ResourceTreeWidget
    //// causes a crash due to unloaded textures
    //material->setData(data, true);
    //// Set sprite info
    ////material->setData(data, true); 
    //material->setSpriteInfo(*m_spriteInfo);
    //progress.setValue(currentProgress);

    //emit ResourceCache::Instance().doneLoadingResource(handle->getUuid());

    //// Close the widget
    //QWidget::close();

}

void SpriteSheetWidget::addTexturesToMaterial(const GRequestAddTexturesToMaterialMessage& message)
{
    static GAddTextureToMaterialMessage s_message;
    std::vector<GStringFixedSize<>> texturePaths;
    std::vector<Int32_t> textureTypes;

    // Save textures and create as resources
    for (size_t i = 0; i < m_texturePackers.size(); i++) {
        const std::shared_ptr<TexturePacker>& packer = m_texturePackers[i];
        if (!packer) {
            continue;
        }
        GTextureUsageType texType = (GTextureUsageType)i;

        // Generate unique texture name to save texture to file
        Image image = packer->getImage();
        GString textureName = GString(GetUniformName(texType)).replace("material.", "");
        textureName += m_materialId.asString() + ".png";
        QString filePath = QDir::cleanPath(m_outputDirectory->getPath() + QDir::separator() + textureName.c_str());
        std::string filePathStr = filePath.toStdString();
        image.save(filePathStr);

        texturePaths.push_back(filePathStr.c_str());
        textureTypes.push_back((Int32_t)texType);
    }
    s_message.setMaterialUuid(m_materialId);
    s_message.setImageFilePaths(texturePaths);
    s_message.setTextureTypes(textureTypes);

    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}


void SpriteSheetWidget::initializeWidgets()
{
    m_materialName = new QLineEdit(this);
    const json& resourcesJson = m_widgetManager->scenarioJson()["resourceCache"]["resources"];
    Uint32_t count = (Uint32_t)resourcesJson.size();
    m_materialName->setText(GString::Format("sprite_sheet_%d", count).c_str());

    m_textureType = new QComboBox(this);

    m_fileLoad = new FileLoadWidget(m_widgetManager, "", 
        "Select Image Files", 
        "Image Files (*.png *.jpg *.bmp *.jpeg *.tiff *.tga)", 
        ".",
        PathAccessMode::kMultipleFiles,
        this);

    // Create bitmap widget
    m_bitmapWidget = new QLabel(this);

    // Create output dir widget
    GString outputDir;
    GString materialName = m_materialName->text().toStdString().c_str();
    if (!m_widgetManager->scenarioJson().empty()) {
        GString scenarioPath = m_widgetManager->scenarioJson()["filePath"];
        GFile scenarioFile(scenarioPath);
        outputDir = scenarioFile.getAbsoluteDirectory();
        outputDir += GPath::Separator() + scenarioFile.getFileName(false);
    }
    else {
        outputDir = GDir::CurrentWorkingDir();
    }
    outputDir += GPath::Separator() + materialName;

    m_outputDirectory = new FileLoadWidget(m_widgetManager, "",
        "Select Material Output Directory",
        QString(),
        outputDir.c_str(),
        PathAccessMode::kDirectory,
        this);
    m_outputDirectory->lineEdit()->setText(outputDir.c_str());

    // Create confirm/cancel buttons
    m_confirmButton = new QPushButton("Generate", this);
    m_cancelButton = new QPushButton("Cancel", this);
}

void SpriteSheetWidget::initializeConnections()
{

    connect(m_fileLoad, &FileLoadWidget::selectedFiles, this,
        [this]() {

            // TODO: Handle different texture types
            // Load images into a single texture
            const std::vector<GString>& filenames = m_fileLoad->fileNames();
            QImage image = packTextures(filenames, (GTextureUsageType)ETextureUsageType::eDiffuse);
            
            // Display packed texture as a pixmap
            /// @todo Reinstate scaling once viewport is accessible
            QPixmap pixmap = QPixmap::fromImage(image);
            //Vector2i screenDims = Viewport::ScreenDimensions();
            //m_bitmapWidget->setPixmap(pixmap.scaled(screenDims.x() / 3, screenDims.y() / 3, Qt::KeepAspectRatio));
            m_bitmapWidget->setPixmap(pixmap);
            m_bitmapWidget->show();
        }
        );

    connect(m_confirmButton, &QPushButton::clicked, this, &SpriteSheetWidget::generateMaterial);

    connect(m_cancelButton, &QPushButton::clicked, this, &QWidget::close);

    connect(m_widgetManager, &WidgetManager::receivedAdvanceProgressWidgetMessage, this,
        [this](const GAdvanceProgressWidgetMessage& message) {
            if (!m_progressDialog) {
                m_progressDialog = new QProgressDialog("Generating Textures and Creating Material", "Abort", 0, (uint32_t)m_texturePackers.size() + 3 /*progressMax*/, nullptr);
                m_currentProgress = 0;
            }
            m_progressDialog->setValue(m_currentProgress++);

            // Close the dialogue and the sprite sheet widget
            if (message.getClose()) {
                m_progressDialog->reset();
                delete m_progressDialog;
                QWidget::close();
            }
        }
    );

    connect(m_widgetManager, &WidgetManager::receivedRequestAddTexturesToMaterialMessage, this, &SpriteSheetWidget::addTexturesToMaterial);
}

void SpriteSheetWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();

    QHBoxLayout* layout0 = new QHBoxLayout();
    layout0->addLayout(LabeledLayout("Material Name", m_materialName));
    layout0->addSpacing(15);
    layout0->addWidget(new QLabel("Texture Type"));
    layout0->addWidget(m_textureType);

    QVBoxLayout* layout1 = new QVBoxLayout();
    layout1->addLayout(LabeledLayout("Sprite Textures", m_fileLoad));
    layout1->addWidget(m_bitmapWidget);
    layout1->addLayout(LabeledLayout("Output Directory", m_outputDirectory));

    QHBoxLayout* layout2 = new QHBoxLayout();
    layout2->addWidget(m_confirmButton);
    layout2->addWidget(m_cancelButton);

    m_mainLayout->addLayout(layout0);
    m_mainLayout->addLayout(layout1);
    m_mainLayout->addLayout(layout2);

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}


} // rev
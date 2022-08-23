#include "geppetto/qt/widgets/tree/GResourceWidgets.h"

#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/json/GJson.h"
#include "fortress/system/path/GPath.h"
#include "fortress/layer/framework/GFlags.h"

#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"
#include "geppetto/qt/widgets/types/GModelWidgets.h"
#include "geppetto/qt/widgets/types/GMaterialWidgets.h"
#include "geppetto/qt/widgets/types/GLoadAudioWidget.h"

#include "enums/GResourceTypeEnum.h"
#include "ripple/network/messages/GLoadTextureResourceMessage.h"
#include "ripple/network/messages/GUnloadTextureResourceMessage.h"
#include "ripple/network/messages/GAddMaterialResourceMessage.h"
#include "ripple/network/messages/GRemoveMaterialResourceMessage.h"
#include "ripple/network/messages/GCopyMaterialResourceMessage.h"
#include "ripple/network/messages/GRemoveMeshResourceMessage.h"
#include "ripple/network/messages/GAddModelResourceMessage.h"
#include "ripple/network/messages/GRemoveModelResourceMessage.h"
#include "ripple/network/messages/GRenameModelMessage.h"
#include "ripple/network/messages/GLoadModelMessage.h"
#include "ripple/network/messages/GCopyModelResourceMessage.h"
#include "ripple/network/messages/GLoadAudioResourceMessage.h"
#include "ripple/network/messages/GRemoveAudioResourceMessage.h"
#include "ripple/network/messages/GLoadShaderResourceMessage.h"
#include "ripple/network/messages/GAddShaderResourceMessage.h"
#include "ripple/network/messages/GRemoveShaderResourceMessage.h"
#include "ripple/network/messages/GReloadResourceMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"

#include "ripple/network/messages/GResourceAddedMessage.h"
#include "ripple/network/messages/GResourceModifiedMessage.h"
#include "ripple/network/messages/GResourceRemovedMessage.h"
#include "ripple/network/messages/GResourceDataMessage.h"
#include "ripple/network/messages/GGetResourceDataMessage.h"

namespace rev {


LoadTextureCommand::LoadTextureCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent)
{
}

LoadTextureCommand::~LoadTextureCommand()
{
}

void LoadTextureCommand::redo()
{
    // Create widget for loading in model
    if (!m_textureLoadWidget) {
        m_textureLoadWidget = new LoadTextureWidget(m_widgetManager);
    }

    // Show add model dialog on redo
    m_textureLoadWidget->show();
}

void LoadTextureCommand::undo()
{
    if (!m_textureLoadWidget->m_textureHandleID.isNull()) {
        static GUnloadTextureResourceMessage s_unloadMessage;
        s_unloadMessage.setUuid(m_textureLoadWidget->m_textureHandleID);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_unloadMessage);
        m_textureLoadWidget->m_textureHandleID = Uuid::NullID();
    }
}


AddMaterialCommand::AddMaterialCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent)
{
}

AddMaterialCommand::~AddMaterialCommand()
{
}

void AddMaterialCommand::redo()
{
    static GAddMaterialResourceMessage s_message;
    m_materialID = Uuid();
    s_message.setUuid(m_materialID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void AddMaterialCommand::undo()
{
    // Delete material
    static GRemoveMaterialResourceMessage s_unloadMessage;
    s_unloadMessage.setUuid(m_materialID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_unloadMessage);
    m_materialID = Uuid::NullID();
}




CopyMaterialCommand::CopyMaterialCommand(WidgetManager* wm, const QString & text, const json& materialJson, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_materialJson(materialJson)
{
    m_materialJson["name"] = getUniqueName();
}

CopyMaterialCommand::~CopyMaterialCommand()
{
}

void CopyMaterialCommand::redo()
{
    static GCopyMaterialResourceMessage s_message;
    m_materialUuid = Uuid();
    s_message.setJsonBytes(GJson::ToBytes(m_materialJson));
    s_message.setUuid(m_materialUuid);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void CopyMaterialCommand::undo()
{
    static GRemoveMaterialResourceMessage s_unloadMessage;
    s_unloadMessage.setUuid(m_materialUuid);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_unloadMessage);
    m_materialUuid = Uuid::NullID();
}

GString CopyMaterialCommand::getUniqueName()
{
    GString name = m_materialJson["name"].get<GString>().asLower();
    GString newName = name + "_" + Uuid::UniqueName();
    return newName;
}



AddMeshCommand::AddMeshCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_meshWidget(nullptr)
{
}

AddMeshCommand::~AddMeshCommand()
{
    delete m_meshWidget;
}

void AddMeshCommand::redo()
{
    // Create widget for loading in model
    if (!m_meshWidget) {
        m_meshWidget = new CreateMeshWidget(m_widgetManager);
    }

    // Show add model dialog on redo
    m_meshWidget->show();
}

void AddMeshCommand::undo()
{
    if (!m_meshWidget->m_meshId.isNull()) {
        static GRemoveMeshResourceMessage s_unloadMessage;
        s_unloadMessage.setUuid(m_meshWidget->m_meshId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_unloadMessage);
        m_meshWidget->m_meshId = Uuid::NullID();
    }
}




AddModelCommand::AddModelCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent)
{
}

AddModelCommand::~AddModelCommand()
{
}

void AddModelCommand::redo()
{
    static GAddModelResourceMessage s_message;
    m_modelHandleID = Uuid();
    s_message.setUuid(m_modelHandleID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void AddModelCommand::undo()
{
    static GRemoveModelResourceMessage s_message;
    s_message.setUuid(m_modelHandleID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
    m_modelHandleID = Uuid(false);
}




LoadModelCommand::LoadModelCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_modelWidget(nullptr)
{
}

LoadModelCommand::~LoadModelCommand()
{
    delete m_modelWidget;
}

void LoadModelCommand::redo()
{
    // Create widget for loading in model
    if (!m_modelWidget) {
        createModelWidget();
    }
    
    // Show add model dialog on redo
    m_modelWidget->show();
}

void LoadModelCommand::undo()
{
    if (!m_modelWidget->m_modelId.isNull()) {
        static GRemoveModelResourceMessage s_message;
        s_message.setUuid(m_modelWidget->m_modelId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        m_modelWidget->m_modelId = Uuid(false);
    }
}

void LoadModelCommand::createModelWidget()
{
    // Create widget to add model to resource cache
    m_modelWidget = new LoadModelWidget(m_widgetManager);
}




CopyModelCommand::CopyModelCommand(WidgetManager* wm, const QString & text, const json& modelJson, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_modelJson(modelJson)
{
    m_modelJson["name"] = getUniqueName();
}

CopyModelCommand::~CopyModelCommand()
{
}

void CopyModelCommand::redo()
{
    static GCopyModelResourceMessage s_message;
    m_modelHandleID = Uuid();
    s_message.setUuid(m_modelHandleID);
    s_message.setJsonBytes(m_modelJson);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void CopyModelCommand::undo()
{
    static GRemoveModelResourceMessage s_message;
    s_message.setUuid(m_modelHandleID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
    m_modelHandleID = Uuid(false);
}

GString CopyModelCommand::getUniqueName()
{
    // Note: Name isn't totally unique, but will at least not match the given one
    // Try to get index of count suffix
    std::vector<GString> parts;
    GString name = m_modelJson["name"].get<GString>().asLower();
    name.split("_", parts);
    int count = parts.back().toInt();

    GString newName = name;
    if (parts.back().isInteger()) {
        // Last portion of name was an integer, so increment count and create name from that
        newName.replace(parts.back(), GString::FromNumber(++count));
    }
    else {
        // Last portion of name was not an integer, so append a new part to the name
        newName += "_0";
    }

    return newName.c_str();
}




LoadAudioCommand::LoadAudioCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent):
    UndoCommand(wm, text, parent),
    m_audioWidget(nullptr)
{
}

LoadAudioCommand::~LoadAudioCommand()
{
    delete m_audioWidget;
}

void LoadAudioCommand::redo()
{
    // Create widget for loading in model
    if (!m_audioWidget) {
        createAudioWidget();
    }

    // Show add model dialog on redo
    m_audioWidget->show();
}

void LoadAudioCommand::undo()
{
    if (!m_audioWidget->m_audioHandleId.isNull()) {
        static GRemoveAudioResourceMessage s_message;
        s_message.setUuid(m_audioWidget->m_audioHandleId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        m_audioWidget->m_audioHandleId = Uuid::NullID();
    }
}

void LoadAudioCommand::createAudioWidget()
{
    // Create widget to add audio to resource cache
    m_audioWidget = new LoadAudioWidget(m_widgetManager);
}





AddShaderCommand::AddShaderCommand(WidgetManager* wm, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_shaderWidget(nullptr)
{
}

AddShaderCommand::~AddShaderCommand()
{
    delete m_shaderWidget;
}

void AddShaderCommand::redo()
{
    if (!m_shaderWidget) {
        createShaderWidget();
    }
    m_shaderWidget->show();
}

void AddShaderCommand::undo()
{
    if (!m_shaderProgramID.isNull()) {
        static GRemoveShaderResourceMessage s_message;
        s_message.setUuid(m_shaderProgramID);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
        m_shaderProgramID == Uuid::NullID();
    }
}

void AddShaderCommand::createShaderWidget()
{
    // Create widget to add shader to resource cache
    m_shaderWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    // Create new layouts -----------------------------------------------------

    // Fragment shader
    QHBoxLayout* fragLayout = new QHBoxLayout();
    QLabel* fragLabel = new QLabel();
    fragLabel->setText("Fragment Shader: ");
    m_fragmentWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Open Fragment Shader",
        "Fragment Shaders (*.frag *.fsh *.glsl)");
    fragLayout->addWidget(fragLabel);
    fragLayout->addWidget(m_fragmentWidget);
    fragLayout->setAlignment(Qt::AlignCenter);

    // Vertex shader
    QHBoxLayout* vertLayout = new QHBoxLayout();
    QLabel* vertLabel = new QLabel();
    vertLabel->setText("Vertex Shader: ");
    m_vertexWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Open Vertex Shader",
        "Vertex Shaders (*.vert *.vsh *.glsl)");
    vertLayout->addWidget(vertLabel);
    vertLayout->addWidget(m_vertexWidget);
    vertLayout->setAlignment(Qt::AlignCenter);

    // Geometry shader
    QHBoxLayout* geometryLayout = new QHBoxLayout();
    QLabel* geometryLabel = new QLabel();
    geometryLabel->setText("Geometry Shader (Optional): ");
    m_geometryWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Open Geometry Shader",
        "Geometry Shaders (*.geom *.glsl)");
    geometryLayout->addWidget(geometryLabel);
    geometryLayout->addWidget(m_geometryWidget);
    geometryLayout->setAlignment(Qt::AlignCenter);

    // Compute shader
    QHBoxLayout* computeLayout = new QHBoxLayout();
    QLabel* computeLabel = new QLabel();
    computeLabel->setText("Compute Shader (Optional): ");
    m_computeWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Open Compute Shader",
        "Compute Shaders (*.comp *.glsl)");
    computeLayout->addWidget(computeLabel);
    computeLayout->addWidget(m_computeWidget);
    computeLayout->setAlignment(Qt::AlignCenter);

    // Connect widgets ---------------------------------------------------

    // Vert
    m_vertexWidget->connect(m_vertexWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_vertFile = text;

        // Auto-complete other entries
        GString fragPath = m_vertFile.toStdString();
        fragPath.replace(".vert", ".frag");
        if (GPath::Exists(fragPath) && fragPath != m_vertFile) {
            m_fragFile = fragPath.c_str();
            m_fragmentWidget->lineEdit()->setText(m_fragFile);
        }

        /// @todo Remove this, deprecating geometry shaders
        //QString geomPath = m_vertFile;
        //geomPath.replace(".vert", ".geom");
        //if (GPath::Exists(geomPath) && geomPath != m_vertFile) {
        //    m_geomFile = geomPath;
        //    m_geometryWidget->lineEdit()->setText(m_geomFile);
        //}

        GString compPath = m_vertFile.toStdString();
        compPath.replace(".vert", ".comp");
        if (GPath::Exists(compPath) && compPath != m_vertFile) {
            m_compFile = compPath.c_str();
            m_computeWidget->lineEdit()->setText(m_compFile);
        }
    });

    // Frag
    m_fragmentWidget->connect(m_fragmentWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_fragFile = text;
    });

    // Geometry 
    /// @todo: Deprecate this
    m_geometryWidget->connect(m_geometryWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_geomFile = text;
    });

    // Compute
    m_computeWidget->connect(m_computeWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_compFile = text;

        // Clear other entries
        m_fragmentWidget->lineEdit()->setText("");
        m_vertexWidget->lineEdit()->setText("");
        m_geometryWidget->lineEdit()->setText("");
    });


    // Dialog Buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    QDialogButtonBox* buttons = new QDialogButtonBox(dialogButtons);
    m_shaderWidget->connect(buttons, &QDialogButtonBox::accepted, m_shaderWidget,
        [&]() 
        {
            static GLoadShaderResourceMessage s_message;
            m_shaderProgramID = Uuid();
            s_message.setUuid(m_shaderProgramID);
            s_message.setVertexFilePath(m_vertFile.toStdString().c_str());
            s_message.setFragmentFilePath(m_fragFile.toStdString().c_str());
            s_message.setComputeFilePath(m_compFile.toStdString().c_str());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
            m_shaderWidget->close();
        }
    );

    // Add layouts to main layout
    layout->addLayout(vertLayout);
    layout->addLayout(fragLayout);
    layout->addLayout(geometryLayout);
    layout->addLayout(computeLayout);
    layout->addWidget(buttons);

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    m_shaderWidget->setLayout(layout);
    m_shaderWidget->setWindowTitle("Select Shader Files");
}





DeleteMaterialCommand::DeleteMaterialCommand(WidgetManager* wm, const json& materialResourceJson, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_materialResourceJson(materialResourceJson),
    m_materialName(materialResourceJson["name"].get_ref<const std::string&>().c_str()),
    m_materialHandleID(materialResourceJson["uuid"].get<Uuid>())
{
}

DeleteMaterialCommand::~DeleteMaterialCommand()
{
}

void DeleteMaterialCommand::redo()
{
    static GRemoveMaterialResourceMessage s_message;
    s_message.setUuid(m_materialHandleID);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void DeleteMaterialCommand::undo()
{
    static GAddMaterialResourceMessage s_message;
    s_message.setName(m_materialName.c_str());
    s_message.setUuid(m_materialHandleID);
    s_message.setMaterialJsonBytes(GJson::ToBytes(m_materialResourceJson["resourceJson"]));
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}





DeleteModelCommand::DeleteModelCommand(WidgetManager* wm, const json& modelResourceJson, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_modelResourceJson(modelResourceJson),
    m_modelName(modelResourceJson["name"])
{
#ifdef DEBUG_MODE
    assert(m_modelResourceJson.contains("resourceJson") && "No resource JSON specified");
#endif
}

DeleteModelCommand::~DeleteModelCommand()
{
}

void DeleteModelCommand::redo()
{
    static GRemoveModelResourceMessage s_message;
    s_message.setUuid(m_modelResourceJson["uuid"].get<Uuid>());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void DeleteModelCommand::undo()
{
    static GAddModelResourceMessage s_message;
    s_message.setName(m_modelName.c_str());
    s_message.setUuid(m_modelResourceJson["uuid"].get<Uuid>());
    s_message.setModelJsonBytes(GJson::ToBytes(m_modelResourceJson["resourceJson"]));
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}





DeleteShaderCommand::DeleteShaderCommand(WidgetManager* wm, const json& shaderResourceJson, const QString & text, QUndoCommand * parent) :
    UndoCommand(wm, text, parent),
    m_shaderResourceJson(shaderResourceJson),
    m_shaderName(m_shaderResourceJson["name"].get_ref<const std::string&>().c_str())
{
}

DeleteShaderCommand::~DeleteShaderCommand()
{
}

void DeleteShaderCommand::redo()
{
    static GRemoveShaderResourceMessage s_message;
    s_message.setUuid(m_shaderResourceJson["uuid"].get<Uuid>());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void DeleteShaderCommand::undo()
{
    static GAddShaderResourceMessage s_message;
    s_message.setUuid(m_shaderResourceJson["uuid"].get<Uuid>());
    s_message.setName(m_shaderName.c_str());
    s_message.setShaderJsonBytes(GJson::ToBytes(m_shaderResourceJson["resourceJson"]));
}


ResourceJsonWidget::ResourceJsonWidget(WidgetManager* wm, const json& resourceHandleJson, QWidget *parent) :
    ParameterWidget(wm, parent),
    m_resourceHandleJson(resourceHandleJson),
    m_resourceHandleID(resourceHandleJson["uuid"]),
    m_resourceName(resourceHandleJson["name"].get_ref<const std::string&>().c_str())
{
    initialize();
}

ResourceJsonWidget::~ResourceJsonWidget()
{
}

void ResourceJsonWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}

void ResourceJsonWidget::initializeWidgets()
{
    QString resourceTypeName = GResourceType::ToString(static_cast<EResourceType>(m_resourceHandleJson["type"].get<Int32_t>()));
    m_typeLabel = new QLabel(resourceTypeName + ": " + m_resourceName);
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_textEdit = new QTextEdit();
    //json oJson = handle->getResourceJson();
    QString text = GJson::ToString<QString>(m_resourceHandleJson["resourceJson"], true);
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

    m_textEdit->setText(text);
}

void ResourceJsonWidget::initializeConnections()
{

    // Make component to recolor text on change
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        // Block signals to avoid infinite loop of signal sends
        m_textEdit->blockSignals(true);

        // Get text and color if not valid JSON
        QString text = m_textEdit->toPlainText();
        json contents = json::parse(text.toStdString());
        QPalette palette;
        QColor badColor = QApplication::palette().color(QPalette::BrightText);
        QColor goodColor = QApplication::palette().color(QPalette::Highlight);
        if (contents.is_null()) {
            palette.setColor(QPalette::Text, badColor);
            m_textEdit->setPalette(palette);
        }
        else {
            palette.setColor(QPalette::Text, goodColor);
            m_textEdit->setPalette(palette);
        }

        m_textEdit->blockSignals(false);

    });

    // Make connection to reload component 
    if (m_confirmButton) {
        connect(m_confirmButton, &QPushButton::clicked, this, 
            [this](bool checked) {
                Q_UNUSED(checked)

                // Get text and return if not valid JSON
                const QString& text = m_textEdit->toPlainText();
                json contents = json::parse(text.toStdString());
                if (contents.is_null()) return;

                static GReloadResourceMessage s_message;
                s_message.setName(m_resourceHandleJson["name"].get_ref<const std::string&>().c_str());
                s_message.setUuid(m_resourceHandleJson["uuid"].get<Uuid>());
                s_message.setResourceJsonBytes(GJson::ToBytes(m_resourceHandleJson["resourceJson"]));
                m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
            }
        );
    }
}

void ResourceJsonWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    // Create new layouts
    QBoxLayout* labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_typeLabel);
    labelLayout->setAlignment(Qt::AlignCenter);

    QBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_textEdit);
    if(m_confirmButton)
        layout->addWidget(m_confirmButton);

    // Add layouts to main layout
    m_mainLayout->addLayout(labelLayout);
    m_mainLayout->addLayout(layout);

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}

ResourceItem::ResourceItem(const char* name) :
    QTreeWidgetItem((int)kCategory),
    m_widget(nullptr)
{
    setText(0, name);
    initializeItem();
}


ResourceItem::ResourceItem(const json& handleJson) :
    QTreeWidgetItem((int)kResourceHandle),
    m_resourceHandleJson(handleJson),
    m_handleId(handleJson["uuid"].get<Uuid>()),
    m_widget(nullptr)
{
    initializeItem();
}

ResourceItem::~ResourceItem()
{
}

void ResourceItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    resourceTreeWidget()->m_widgetManager->actionManager()->performAction(command);
}

void ResourceItem::setWidget()
{
    // Throw error if the widget already exists
    assert(!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    ResourceTreeWidget * parentWidget = static_cast<ResourceTreeWidget*>(treeWidget());

    // Set widget
    m_widget = new ResourceJsonWidget(parentWidget->m_widgetManager, m_resourceHandleJson);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void ResourceItem::removeWidget()
{
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

ResourceTreeWidget * ResourceItem::resourceTreeWidget() const
{
    return static_cast<ResourceTreeWidget*>(treeWidget());
}

void ResourceItem::initializeItem()
{
    // Set column text
    //refreshText();

    // Set default size of item
    //setSizeHint(0, QSize(200, 400));

    // Set flags to be drag and drop enabled
    //setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
}





ResourceTreeWidget::ResourceTreeWidget(WidgetManager* wm, const QString& name, QWidget* parent) :
    rev::NameableInterface(name.toStdString().c_str()),
    QTreeWidget(parent),
    m_widgetManager(wm),
    m_textureItem(nullptr),
    m_materialItem(nullptr),
    m_meshItem(nullptr),
    m_cubeTextureItem(nullptr),
    m_animationItem(nullptr),
    m_skeletonItem(nullptr),
    m_modelItem(nullptr),
    m_shaderItem(nullptr),
    m_scriptItem(nullptr),
    m_audioItem(nullptr)
{
    initializeWidget();
}


ResourceTreeWidget::~ResourceTreeWidget()
{
}

void ResourceTreeWidget::repopulate(const GScenarioJsonMessage& message)
{
    // Return if there are still resources loading
    ////size_t loadProcessCount = m_widgetManager->processManager()->loadProcessCount();
    ////if (loadProcessCount) return;
    //if (ResourceCache::Instance().isLoadingResources()) return;
    Q_UNUSED(message);

    // Disable repaint while big visual changes happening, slightly more performant
    setUpdatesEnabled(false);

    // Clear the widget
    clear();

    // Set header item and disable scene object dragging
    //setHeaderItem(scenario);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);

    // Re-initialize categories
    initializeCategories();

    // Get resources json, more efficient to use JSON from widget manager than to load from message a second time
    const json& resourcesJson = m_widgetManager->scenarioJson()["resourceCache"]["resources"];

    // Add resources
    for (const json& resourceJson : resourcesJson) {
        addItem(resourceJson);
    }

    resizeColumns();

    // Disable repaint while big visual changes happening, slightly more performant
    setUpdatesEnabled(true);
}

void ResourceTreeWidget::requestReloadItem(const Uuid& handleId)
{
    static GGetResourceDataMessage s_message;
    s_message.setUuid(handleId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_message);
}

void ResourceTreeWidget::reloadItem(const GResourceDataMessage& message)
{
    reloadItem(message.getUuid(), GJson::FromBytes(message.getResourceJsonBytes()));
}

void ResourceTreeWidget::reloadItem(const GResourceAddedMessage& message)
{
    reloadItem(message.getUuid(), GJson::FromBytes(message.getResourceJsonBytes()));
}

void ResourceTreeWidget::reloadItem(const GResourceModifiedMessage& message)
{
    reloadItem(message.getUuid(), GJson::FromBytes(message.getResourceJsonBytes()));
}

void ResourceTreeWidget::addItem(const json& handleJson)
{
    // Create resource item
    ResourceItem* resourceItem = new ResourceItem(handleJson);

    // Add resource item
    addItem(resourceItem);
}

void ResourceTreeWidget::addItem(ResourceItem * item)
{
#ifdef DEBUG_MODE
    assert(!item->resourceHandleJson().empty() && "Error, item has no handle");
#endif

    //// Create widget for component
    //if (item->handle()->isLoading()) {
    //    item->setText(0, "Loading");
    //    return;
    //}

    // Insert resource item into the widget
    ResourceItem* parent = nullptr;
    EResourceType type = static_cast<GResourceType>(item->resourceHandleJson()["type"].get<Int32_t>());
    switch (type) {
    case EResourceType::eTexture:
        parent = m_textureItem;
        break;
    case EResourceType::eMaterial:
        parent = m_materialItem;
        break;
    case EResourceType::eMesh:
        parent = m_meshItem;
        break;
    case EResourceType::eCubeTexture:
        parent = m_cubeTextureItem;
        break;
    case EResourceType::eAnimation:
        parent = m_animationItem;
        break;
    case EResourceType::eSkeleton:
        parent = m_skeletonItem;
        break;
    case EResourceType::eModel:
        parent = m_modelItem;
        break;
    case EResourceType::eShaderProgram:
        parent = m_shaderItem;
        break;
    case EResourceType::ePythonScript:
        parent = m_scriptItem;
        break;
    case EResourceType::eAudio:
        parent = m_audioItem;
        break;
    default:
        assert(false && "Error, item type not recognized");
        break;
    }
    if (parent)
        parent->addChild(item);
    else
        assert(false && "No parent found");

    item->setWidget();
}

void ResourceTreeWidget::removeItem(ResourceItem * resourceItem)
{
    if (resourceItem) {
        delete resourceItem;
    }
}

void ResourceTreeWidget::removeItem(const Uuid& handleId)
{
    ResourceItem* item = getItem(handleId);
    if (item) {
        delete item;
    }
}

ResourceItem * ResourceTreeWidget::getItem(const Uuid& handleId)
{
#ifdef DEBUG_MODE
    assert(!handleId.isNull() && "Null ID provided");
#endif

    QTreeWidgetItemIterator it(this);
    while (*it) {
        ResourceItem* item = static_cast<ResourceItem*>(*it);
        if (item->handleId() == handleId) {
            return item;
        }
        ++it;
    }

    return nullptr;
}

void ResourceTreeWidget::resizeColumns()
{
    resizeColumnToContents(0);
}

void ResourceTreeWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);
}

void ResourceTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}

void ResourceTreeWidget::initializeCategories()
{
    // Insert categorioes into the widget
    m_textureItem = new ResourceItem("Textures");
    m_materialItem = new ResourceItem("Materials");
    m_meshItem = new ResourceItem("Meshes");
    m_cubeTextureItem = new ResourceItem("Cube Textures");
    m_animationItem = new ResourceItem("Animations");
    m_skeletonItem = new ResourceItem("Skeletons");
    m_modelItem = new ResourceItem("Models");
    m_shaderItem = new ResourceItem("Shaders");
    m_scriptItem = new ResourceItem("Scripts");
    m_audioItem = new ResourceItem("Audio");

    addTopLevelItem(m_textureItem);
    addTopLevelItem(m_materialItem);
    addTopLevelItem(m_meshItem);
    addTopLevelItem(m_cubeTextureItem);
    addTopLevelItem(m_animationItem);
    addTopLevelItem(m_skeletonItem);
    addTopLevelItem(m_modelItem);
    addTopLevelItem(m_shaderItem);
    addTopLevelItem(m_scriptItem);
    addTopLevelItem(m_audioItem);
    
    resizeColumns();
}

void ResourceTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}

void ResourceTreeWidget::dropEvent(QDropEvent * event)
{
    QTreeWidget::dropEvent(event);
}

void ResourceTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}

void ResourceTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}

void ResourceTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}

void ResourceTreeWidget::initializeWidget()
{
    setMinimumWidth(350);

    // Set tree widget settings
    setColumnCount(0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    // Insert categorioes into the widget
    initializeCategories();

    // Initialize add actions
    m_addTexture = new QAction(tr("&Load Texture"), this);
    m_addTexture->setStatusTip("Load a texture into the scenario");
    connect(m_addTexture,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new LoadTextureCommand(m_widgetManager,
                "Load Texture"));
    });

    m_addMaterial = new QAction(tr("&Load Material"), this);
    m_addMaterial->setStatusTip("Add an empty material to the scenario");
    connect(m_addMaterial,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new AddMaterialCommand(m_widgetManager,
                "Add Material"));
    });

    m_copyMaterial = new QAction(tr("&Copy Material"), this);
    m_copyMaterial->setStatusTip("Copy a material into the scenario");
    connect(m_copyMaterial,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new CopyMaterialCommand(m_widgetManager,
                "Copy Material",
                m_currentResourceItem->resourceHandleJson()["resourceJson"]
            ));
    });

    m_addModel = new QAction(tr("&Add Model"), this);
    m_addModel->setStatusTip("Create a new model in the current scenario");
    connect(m_addModel,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new AddModelCommand(m_widgetManager,
                "Add Model"));
    });

    m_loadModel = new QAction(tr("&Add Model"), this);
    m_loadModel->setStatusTip("Load a model into the scenario");
    connect(m_loadModel,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new LoadModelCommand(m_widgetManager,
                "Load Model"));
    });

    m_addMesh = new QAction(tr("&Add Mesh"), this);
    m_addMesh->setStatusTip("Create a mesh in the current scenario");
    connect(m_addMesh,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new AddMeshCommand(m_widgetManager, "Add Mesh"));
    });

    m_copyModel = new QAction(tr("&Copy Model"), this);
    m_copyModel->setStatusTip("Copy a model from this scenario");
    connect(m_copyModel,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new CopyModelCommand(m_widgetManager, "Copy Model", 
                m_currentResourceItem->resourceHandleJson()["resourceJson"]
            ));
    });

    m_addShaderProgram = new QAction(tr("&Load Shader Program"), this);
    m_addShaderProgram->setStatusTip("Load a shader program into the scenario");
    connect(m_addShaderProgram,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new AddShaderCommand(m_widgetManager,
                "Load Shader Program"));
    });

    // Initialize delete actions
    m_deleteMaterial = new QAction(tr("&Delete Material"), this);
    m_deleteMaterial->setStatusTip("Delete the selected material");
    connect(m_deleteMaterial,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new DeleteMaterialCommand(m_widgetManager,
                m_currentResourceItem->resourceHandleJson(),
                "Delete Material"));
    });

    m_deleteModel = new QAction(tr("&Delete Model"), this);
    m_deleteModel->setStatusTip("Delete the selected model");
    connect(m_deleteModel,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new DeleteModelCommand(m_widgetManager,
                m_currentResourceItem->resourceHandleJson(),
                "Delete Model"));
    });

    m_deleteShaderProgram = new QAction(tr("&Delete Shader Program"), this);
    m_deleteShaderProgram->setStatusTip("Delete the selected shader program");
    connect(m_deleteShaderProgram,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new DeleteShaderCommand(m_widgetManager,
                m_currentResourceItem->resourceHandleJson(),
                "Delete Shader Program"));
    });

    // Connect signal for double click events
    connect(this, &ResourceTreeWidget::itemDoubleClicked,
        this, &ResourceTreeWidget::onItemDoubleClicked);

    // Connect signal for item expansion
    connect(this, &ResourceTreeWidget::itemExpanded,
        this, &ResourceTreeWidget::onItemExpanded);

    // Connect signal for repopulating on scenario load
    connect(m_widgetManager,
        &WidgetManager::receivedScenarioJsonMessage,
        this,
        &ResourceTreeWidget::repopulate,
        Qt::DirectConnection);


    // Connect signals for repopulating on resource modifications
    connect(m_widgetManager,
        &WidgetManager::receivedResourceDataMessage,
        this,
        Signal<const GResourceDataMessage&>::Cast(&ResourceTreeWidget::reloadItem),
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourceAddedMessage,
        this,
        Signal<const GResourceAddedMessage&>::Cast(&ResourceTreeWidget::reloadItem),
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourceModifiedMessage,
        this,
        Signal<const GResourceModifiedMessage&>::Cast(&ResourceTreeWidget::reloadItem),
        Qt::DirectConnection);
    connect(m_widgetManager,
        &WidgetManager::receivedResourceRemovedMessage,
        this,
        [&](const GResourceRemovedMessage& message) {
            return removeItem(message.getUuid());
        },
        Qt::DirectConnection);
}

void ResourceTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    // Return if there is no scene object selected
    if (!m_currentResourceItem) {
        return;
    }

    // Create menu
    QMenu menu(this);

    // Add actions to the menu
    if (itemAt(event->pos())) {
        auto* item = static_cast<ResourceItem*>(itemAt(event->pos()));
        m_currentResourceItem = item;
        const json& handleJson = item->resourceHandleJson();
        if (!handleJson.empty()) {
            EResourceType type = static_cast<EResourceType>(handleJson["type"].get<Int32_t>());
            Int32_t behaviorFlags = handleJson["behaviorFlags"].get<Int32_t>();
            bool isCore = (behaviorFlags & (1 << 4)) != 0; //  Since isCore flag == 16
            switch (type) {
            case EResourceType::eMaterial:
                menu.addAction(m_copyMaterial);
                if (!isCore) {
                    menu.addAction(m_deleteMaterial);
                }
                break;
            case EResourceType::eModel:
                menu.addAction(m_copyModel);
                if (!isCore) {
                    menu.addAction(m_deleteModel);
                }
                break;
            case EResourceType::eShaderProgram:
                if (!isCore) {
                    menu.addAction(m_deleteShaderProgram);
                }
                break;
            default:
                break;
            }
        }
    }
    else {
        // Options to add things when there is no item selected
        menu.addAction(m_addMesh);
        menu.addSeparator();
        menu.addAction(m_addModel);
        menu.addAction(m_loadModel);
        menu.addSeparator();
        menu.addAction(m_addTexture);
        menu.addAction(m_addMaterial);
        menu.addSeparator();
        menu.addAction(m_addShaderProgram);
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}

void ResourceTreeWidget::reloadItem(const Uuid& handleId, const json& handleJson)
{
#ifdef DEBUG_MODE
    assert(!handleJson.empty() && "Error, no handle passed through");
#endif

    if (getItem(handleId)) {
        removeItem(handleId);
    }

    addItem(handleJson);
}




// End namespaces 
}
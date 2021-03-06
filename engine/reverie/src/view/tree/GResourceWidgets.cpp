#include "GResourceWidgets.h"

#include "../../model_control/commands/GActionManager.h"

#include "../../core/resource/GResourceCache.h"
#include "../../core/resource/GResource.h"
#include "../../core/scene/GScenario.h"
#include "../../core/rendering/models/GModel.h"
#include "../../core/rendering/materials/GCubeTexture.h"
#include "../../core/rendering/materials/GMaterial.h"
#include "../../core/rendering/shaders/GShaderProgram.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../../core/processes/GProcessManager.h"
#include "../../core/readers/GJsonReader.h"

#include "../GWidgetManager.h"
#include "../../GMainWindow.h"
#include "../parameters/GModelWidgets.h"
#include "../parameters/GMaterialWidgets.h"
#include "../parameters/GLoadAudioWidget.h"
#include "../../core/containers/GFlags.h"

namespace rev {

#define FULL_DELETE_RESOURCE Flags<ResourceDeleteFlag>(ResourceDeleteFlag::kDeleteHandle | ResourceDeleteFlag::kForce )

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LoadTextureCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadTextureCommand::LoadTextureCommand(CoreEngine * core,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadTextureCommand::~LoadTextureCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadTextureCommand::redo()
{
    // Create widget for loading in model
    if (!m_textureLoadWidget) {
        m_textureLoadWidget = new View::LoadTextureWidget(m_engine);
    }

    // Show add model dialog on redo
    m_textureLoadWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadTextureCommand::undo()
{
    if (!m_textureLoadWidget->m_textureHandleID.isNull()) {
        auto handle = m_engine->resourceCache()->getHandle(m_textureLoadWidget->m_textureHandleID);
        m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
        m_textureLoadWidget->m_textureHandleID = Uuid(false);
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddMaterialCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteTextureCommand::DeleteTextureCommand(CoreEngine * core,
    const QString & text,
    const std::shared_ptr<ResourceHandle>& texture,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_textureHandleID(texture->getUuid())
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteTextureCommand::~DeleteTextureCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteTextureCommand::redo()
{
    //// Add material to resource cache
    //QString name = "material_" + Uuid().asString();
    //auto handle = Material::createHandle(m_engine, name);
    //m_materialID = handle->getUuid();

    //// Emit signal that material was created
    //emit m_engine->resourceCache()->resourceAdded(handle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteTextureCommand::undo()
{
    //// Delete material
    //auto handle = m_engine->resourceCache()->getHandle(m_uuid);
    //m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
    //m_materialID = Uuid();
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddMaterialCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
AddMaterialCommand::AddMaterialCommand(CoreEngine * core, 
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AddMaterialCommand::~AddMaterialCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddMaterialCommand::redo()
{
    // Add material to resource cache
    QString name = "material_" + (QString)Uuid().asString();
    auto handle = Material::CreateHandle(m_engine, name);
    m_materialID = handle->getUuid();

    // Emit signal that material was created
    emit m_engine->resourceCache()->resourceAdded(handle->getUuid());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddMaterialCommand::undo()
{
    // Delete material
    auto handle = m_engine->resourceCache()->getHandle(m_uuid);
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
    m_materialID = Uuid(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Copy Material Command
///////////////////////////////////////////////////////////////////////////////////////////////////
CopyMaterialCommand::CopyMaterialCommand(CoreEngine * core, 
    const QString & text, 
    const Material & mtl, 
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_materialJson(mtl.asJson())
{
    QJsonObject obj = m_materialJson.toObject();
    obj["name"] = getUniqueName();
    m_materialJson = obj;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CopyMaterialCommand::~CopyMaterialCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyMaterialCommand::redo()
{
    // Add material to resource cache
    QString name = "material_" + (QString)Uuid().asString();
    auto matHandle = Material::CreateHandle(m_engine, name);
    matHandle->resourceAs<Material>()->loadFromJson(m_materialJson, { m_engine });
    m_materialUuid = matHandle->getUuid();

    // Emit signal that material was created
    emit m_engine->resourceCache()->resourceAdded(matHandle->getUuid());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyMaterialCommand::undo()
{
    auto mtl = m_engine->resourceCache()->getHandle(m_materialUuid);
    m_materialJson = mtl->asJson();
    m_engine->resourceCache()->remove(mtl, FULL_DELETE_RESOURCE);
    m_materialUuid = Uuid(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QString CopyMaterialCommand::getUniqueName()
{
    QString name = m_materialJson["name"].toString().toLower();
    //int count = 1;
    //while (m_engine->resourceCache()->materials().find(Uuid(newName))
    //    != m_engine->resourceCache()->materials().end()) {
    //    newName = name + "_" + QString::number(count++);
    //}
    QString newName = name + "_" + Uuid::UniqueQName();
    return newName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Add Mesh Command
///////////////////////////////////////////////////////////////////////////////////////////////////
AddMeshCommand::AddMeshCommand(CoreEngine * core,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_meshWidget(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AddMeshCommand::~AddMeshCommand()
{
    delete m_meshWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddMeshCommand::redo()
{
    // Create widget for loading in model
    if (!m_meshWidget) {
        m_meshWidget = new View::CreateMeshWidget(m_engine);
    }

    // Show add model dialog on redo
    m_meshWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddMeshCommand::undo()
{
    if (!m_meshWidget->m_meshHandleID.isNull()) {
        auto handle = m_engine->resourceCache()->getHandle(m_meshWidget->m_meshHandleID);
        m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
        m_meshWidget->m_meshHandleID = Uuid(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Add Model Command
///////////////////////////////////////////////////////////////////////////////////////////////////
AddModelCommand::AddModelCommand(CoreEngine * core,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AddModelCommand::~AddModelCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddModelCommand::redo()
{
    // Add model to resource cache
    auto handle = Model::CreateHandle(m_engine);
    m_modelHandleID = handle->getUuid();
    handle->setRuntimeGenerated(true);
    handle->setIsLoading(true);
    //handle->setResourceJson(m_modelJson.toObject());
    //handle->loadResource();
    GString uniqueName = Uuid::UniqueName("model_");
    auto model = std::make_unique<Model>(*handle, handle->resourceJson());
    //model->setName(uniqueName);
    handle->setName(uniqueName);
    handle->setResource(std::move(model), false);
    handle->setConstructed(true);

    emit m_engine->resourceCache()->resourceAdded(handle->getUuid());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddModelCommand::undo()
{
    auto handle = m_engine->resourceCache()->getHandle(m_modelHandleID);
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
    m_modelHandleID = Uuid(false);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Load Model Command
///////////////////////////////////////////////////////////////////////////////////////////////////

LoadModelCommand::LoadModelCommand(CoreEngine * core, 
    const QString & text, 
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_modelWidget(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadModelCommand::~LoadModelCommand()
{
    delete m_modelWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelCommand::redo()
{
    // Create widget for loading in model
    if (!m_modelWidget) {
        createModelWidget();
    }
    
    // Show add model dialog on redo
    m_modelWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelCommand::undo()
{
    if (!m_modelWidget->m_modelID.isNull()) {
        auto handle = m_engine->resourceCache()->getHandle(m_modelWidget->m_modelID);
        m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
        m_modelWidget->m_modelID = Uuid(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelCommand::createModelWidget()
{
    // Create widget to add model to resource cache
    m_modelWidget = new View::LoadModelWidget(m_engine);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CopyModelCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
CopyModelCommand::CopyModelCommand(CoreEngine * core,
    const QString & text,
    const Model& model,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_modelJson(model.asJson())
{
    QJsonObject obj = m_modelJson.toObject();
    obj["name"] = getUniqueName();
    m_modelJson = obj;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CopyModelCommand::~CopyModelCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyModelCommand::redo()
{
    // Add model to resource cache
    auto handle = Model::CreateHandle(m_engine);
    handle->setName(m_modelJson["name"].toString());
    m_modelHandleID = handle->getUuid();
    handle->setRuntimeGenerated(true);
    handle->setResourceJson(m_modelJson.toObject());
    handle->loadResource();

    // Shouldn't be necessary now that loadResource is being called
    //emit m_engine->resourceCache()->resourceAdded(handle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyModelCommand::undo()
{
    auto handle = m_engine->resourceCache()->getHandle(m_modelHandleID);
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
    m_modelHandleID = Uuid(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QString CopyModelCommand::getUniqueName()
{
    // Note: Name isn't totally unique, but will at least not match the given one
    // Try to get index of count suffix
    std::vector<GString> parts;
    GString name = GString(m_modelJson["name"].toString()).asLower();
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

    //int count = 1;
    //const ResourceCache::ResourceMap& models = m_engine->resourceCache()->models();
    //auto iter = std::find_if(models.begin(), models.end(),
    //    [&](const auto& modelPair) {
    //    return modelPair.second->getName() == name;
    //});
    //while (iter != models.end()) {
    //    newName = name + "_" + GString::FromNumber(count++);

    //    iter = std::find_if(models.begin(), models.end(),
    //        [&](const auto& modelPair) {
    //        return modelPair.second->getName() == newName;
    //    });
    //}
    return newName.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LoadAudioCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadAudioCommand::LoadAudioCommand(CoreEngine * core, 
    const QString & text,
    QUndoCommand * parent):
    UndoCommand(core, text, parent),
    m_audioWidget(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadAudioCommand::~LoadAudioCommand()
{
    delete m_audioWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadAudioCommand::redo()
{
    // Create widget for loading in model
    if (!m_audioWidget) {
        createAudioWidget();
    }

    // Show add model dialog on redo
    m_audioWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadAudioCommand::undo()
{
    if (!m_audioWidget->m_audioHandleID.isNull()) {
        auto handle = m_engine->resourceCache()->getHandle(m_audioWidget->m_audioHandleID);
        m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
        m_audioWidget->m_audioHandleID = Uuid(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadAudioCommand::createAudioWidget()
{
    // Create widget to add audio to resource cache
    m_audioWidget = new View::LoadAudioWidget(m_engine);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddShaderCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
AddShaderCommand::AddShaderCommand(CoreEngine * core, 
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_shaderWidget(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AddShaderCommand::~AddShaderCommand()
{
    delete m_shaderWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddShaderCommand::redo()
{
    if (!m_shaderWidget) {
        createShaderWidget();
    }
    m_shaderWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddShaderCommand::undo()
{
    if (!m_shaderProgramID.isNull()) {
        auto handle = m_engine->resourceCache()->getHandle(m_shaderProgramID);
        m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
        m_shaderProgramID == Uuid(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
    m_fragmentWidget = new View::FileLoadWidget(m_engine,
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
    m_vertexWidget = new View::FileLoadWidget(m_engine,
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
    m_geometryWidget = new View::FileLoadWidget(m_engine,
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
    m_computeWidget = new View::FileLoadWidget(m_engine,
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
        QString fragPath = m_vertFile;
        fragPath.replace(".vert", ".frag");
        if (FileReader::FileExists(fragPath) && fragPath != m_vertFile) {
            m_fragFile = fragPath;
            m_fragmentWidget->lineEdit()->setText(m_fragFile);
        }

        QString geomPath = m_vertFile;
        geomPath.replace(".vert", ".geom");
        if (FileReader::FileExists(geomPath) && geomPath != m_vertFile) {
            m_geomFile = geomPath;
            m_geometryWidget->lineEdit()->setText(m_geomFile);
        }

        QString compPath = m_vertFile;
        compPath.replace(".vert", ".comp");
        if (FileReader::FileExists(compPath) && compPath != m_vertFile) {
            m_compFile = compPath;
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
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    QDialogButtonBox* buttons = new QDialogButtonBox(dialogButtons);
    m_shaderWidget->connect(buttons, &QDialogButtonBox::accepted, m_shaderWidget,
        [&]() {

        //QString name = m_shaderProgram->getName().toLower();
        //m_shaderProgram = std::make_shared<ShaderProgram>(m_vertFile, m_fragFile);
        //m_engine->resourceCache()->shaderPrograms().emplace(name, m_shaderProgram);
        auto handle = m_engine->resourceCache()->guaranteeHandleWithPath(
            { m_vertFile, m_fragFile, m_geomFile, m_compFile}, ResourceType::kShaderProgram);
        //handle->setName(handle->resourceAs<ShaderProgram>()->getName());
        m_shaderProgramID = handle->getUuid();

        // Repopulate resource widget
        emit m_engine->resourceCache()->resourceAdded(handle->getUuid());

        m_shaderWidget->close();
    });

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


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteMaterialCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteMaterialCommand::DeleteMaterialCommand(CoreEngine * core, 
    Material* material,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_materialJson(material->asJson()),
    m_materialName(material->handle()->getName()),
    m_materialHandleID(material->handle()->getUuid())
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteMaterialCommand::~DeleteMaterialCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteMaterialCommand::redo()
{
    auto handle = m_engine->resourceCache()->getHandle(m_materialHandleID);
    if (!handle) throw("Handle with the given name does not exist");
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteMaterialCommand::undo()
{
    auto handle = Material::CreateHandle(m_engine, m_materialName);
    handle->resourceAs<Material>()->loadFromJson(m_materialJson, { m_engine });

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(handle->getUuid());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteModelCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteModelCommand::DeleteModelCommand(CoreEngine * core,
    Model* model,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_modelJson(model->asJson()),
    m_modelName(model->handle()->getName())
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteModelCommand::~DeleteModelCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteModelCommand::redo()
{
    auto handle = m_engine->resourceCache()->getHandleWithName(m_modelName, ResourceType::kModel);
    if (!handle) throw("Handle with the given name does not exist");
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteModelCommand::undo()
{
    // Add model back to resource cache
    auto handle = Model::CreateHandle(m_engine, m_modelName);
    handle->resourceAs<Model>()->loadFromJson(m_modelJson);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(handle->getUuid());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteShaderCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteShaderCommand::DeleteShaderCommand(CoreEngine * core, 
    ShaderProgram* shader,
    const QString & text, 
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_shaderJson(shader->asJson()),
    m_shaderName(shader->handle()->getName())
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteShaderCommand::~DeleteShaderCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteShaderCommand::redo()
{
    auto handle = m_engine->resourceCache()->getHandleWithName(m_shaderName, ResourceType::kShaderProgram);
    m_engine->resourceCache()->remove(handle, FULL_DELETE_RESOURCE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteShaderCommand::undo()
{
    // Add shader back to resource cache
    auto handle = ShaderProgram::CreateHandle(m_engine, m_shaderJson, m_shaderName);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(handle->getUuid());
}



namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceJsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ResourceHandle>& resource, QWidget *parent) :
    ParameterWidget(core, parent),
    m_resourceHandleID(resource->getUuid()),
    m_resourceName(resource->getName())
{
#ifdef DEBUG_MODE
    if (m_resourceHandleID != resource->getUuid())
        throw("Error, copied UUID does not match resource UUID");
#endif
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::~ResourceJsonWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<ResourceHandle> ResourceJsonWidget::resourceHandle() const
{
    auto handle = m_engine->resourceCache()->getHandle(m_resourceHandleID);
    return handle;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceJsonWidget::initializeWidgets()
{
    auto handle = resourceHandle();
    while (!handle) {
        // FIXME: See if this actually stops crash
        //throw("No handle found");
        logWarning("Resource deleted or not found, waiting to load");
        handle = resourceHandle();
    }
    m_typeLabel = new QLabel(QString(handle->resource()->className()) + ": " +
        handle->getName().c_str());
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_textEdit = new QTextEdit();
    QString text;
    Serializable* sptr = handle->resourceAs<Serializable>();
    if (sptr) {
        Serializable& serializable = *sptr;
        QJsonValue json = serializable.asJson();
        text = JsonReader::ToString<QString>(json, true);
        m_confirmButton = new QPushButton();
        m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    }
    else {
        if (!handle->getName().isEmpty())
            text = handle->getName().c_str();
        else
            text = handle->getUuid().asQString();
        m_textEdit->setEnabled(false);
    }
    m_textEdit->setText(text);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceJsonWidget::initializeConnections()
{

    // Make component to recolor text on change
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        // Block signals to avoid infinite loop of signal sends
        m_textEdit->blockSignals(true);

        // Get text and color if not valid JSON
        QString text = m_textEdit->toPlainText();
        QJsonDocument contents = JsonReader::ToJsonDocument(text);
        QPalette palette;
        QColor badColor = QApplication::palette().color(QPalette::BrightText);
        QColor goodColor = QApplication::palette().color(QPalette::Highlight);
        if (contents.isNull()) {
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
        connect(m_confirmButton, &QPushButton::clicked, this, [this](bool checked) {
            //QProgressDialog progress("Reloading resource", "Close", 0, 1, 
            //    m_engine->widgetManager()->mainWindow());
            //progress.setWindowModality(Qt::WindowModal);
            Q_UNUSED(checked)

                // Get text and return if not valid JSON
                const QString& text = m_textEdit->toPlainText();
            QJsonDocument contents = JsonReader::ToJsonDocument(text);
            if (contents.isNull()) return;

            std::shared_ptr<ResourceHandle> handle = resourceHandle();
            if (!handle) {
                throw("Error, somehow no handle found");
            }

            // Copy name prior to reload
            GString name = handle->getName();

            // Pause scenario to edit resource
            SimulationLoop* simLoop = m_engine->simulationLoop();
            bool wasPlaying = simLoop->isPlaying();
            if (wasPlaying) {
                simLoop->pause();
            }
            m_engine->simulationLoop()->pause();

            // Edit resource via JSON
            handle->resourceAs<Serializable>()->loadFromJson(contents.object());

            // If object name has changed, emit signal to repopulate widget
            if (name != handle->getName()) {
                emit m_engine->resourceCache()->resourceChanged(handle->getUuid());
            }

            // Unpause scenario
            if (wasPlaying) {
                simLoop->play();
            }

            //progress.setValue(1);
        });
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceItem
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceItem::ResourceItem(const QString& text) :
    QTreeWidgetItem((int)kCategory),
    m_widget(nullptr)
{
    setText(0, text);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceItem::ResourceItem(const std::shared_ptr<ResourceHandle>& object) :
    QTreeWidgetItem((int)getItemType(object)),
    m_handle(object),
    m_widget(nullptr)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceItem::~ResourceItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    resourceTreeWidget()->m_engine->actionManager()->performAction(command);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) 
        throw("Error, item already has a widget");

    // Get parent tree widget
    ResourceTreeWidget * parentWidget = static_cast<ResourceTreeWidget*>(treeWidget());

    // Set widget if resource is JSON serializable
    Serializable* sptr = m_handle->resourceAs<Serializable>();
    if (sptr) {
        QJsonDocument doc(sptr->asJson().toObject());
        QString rep(doc.toJson(QJsonDocument::Indented));

        m_widget = new ResourceJsonWidget(parentWidget->m_engine, m_handle);

        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, 0, m_widget);
    }
    else {
        setText(0, (QString)m_handle->getName());
    }


}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceItem::removeWidget()
{
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
View::ResourceTreeWidget * ResourceItem::resourceTreeWidget() const
{
    return static_cast<View::ResourceTreeWidget*>(treeWidget());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceItem::ResourceItemType ResourceItem::getItemType(std::shared_ptr<Object> object)
{
    QString name = object->className();
    if (name == "ResourceHandle") {
        return kResourceHandle;
    }
    else {
        throw("Error, type not recognized");
    }
    return ResourceItemType();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceItem::initializeItem()
{
    // Set column text
    //refreshText();

    // Set default size of item
    //setSizeHint(0, QSize(200, 400));

    // Set flags to be drag and drop enabled
    //setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceTreeWidget::ResourceTreeWidget(CoreEngine* engine, const QString& name, QWidget* parent) :
    AbstractService(name),
    QTreeWidget(parent),
    m_engine(engine),
    m_imageItem(nullptr),
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

///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceTreeWidget::~ResourceTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::repopulate()
{
    // Return if there are still resources loading
    //size_t loadProcessCount = m_engine->processManager()->loadProcessCount();
    //if (loadProcessCount) return;
    if (m_engine->resourceCache()->isLoadingResources()) return;

    // Disable repaint while big visual changes happening, slightly more performant
    setUpdatesEnabled(false);

    // Clear the widget
    clear();

    // Set header item and disable scene object dragging
    //setHeaderItem(scenario);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);

    // Re-initialize categories
    initializeCategories();

    // Get resource cache
    ResourceCache* cache = m_engine->resourceCache();

    // Add resources
    //for (const auto& resourcePair : cache->resources()) {
    //    addItem(resourcePair.second);
    //}
    cache->resources().forEach(
        [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
    {
        addItem(resourcePair.second);
    });

    resizeColumns();

    // Disable repaint while big visual changes happening, slightly more performant
    setUpdatesEnabled(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::reloadItem(const Uuid& handleId)
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandle(handleId);
    if (!handle) {
        // FIXME: See what causes this
        std::vector<GString> uuids;
        std::vector<GString> resourceNames;
        for (const auto& uuidPair : m_engine->resourceCache()->resources()) {
            uuids.push_back(uuidPair.second->getUuid().asString());
            resourceNames.push_back(uuidPair.second->getName());
        }
        throw("Error, no handle passed through");
    }

    if (getItem(handle)) {
        removeItem(handleId);
    }
    
    addItem(handle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::addItem(std::shared_ptr<ResourceHandle> handle)
{
    // Create resource item
    ResourceItem* resourceItem = new View::ResourceItem(handle);

    // Add resource item
    addItem(resourceItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::addItem(View::ResourceItem * item)
{
    // Return if this item is a category
    if (!item->handle()) {
        throw("Error, item has no handle");
    }

    // Create widget for component
    if (item->handle()->isLoading()) {
        item->setText(0, "Loading");
        return;
    }

    // Insert resource item into the widget
    ResourceItem* parent = nullptr;
    switch (item->handle()->getResourceType()) {
    case ResourceType::kImage:
        parent = m_imageItem;
        break;
    case ResourceType::kTexture:
        parent = m_textureItem;
        break;
    case ResourceType::kMaterial:
        parent = m_materialItem;
        break;
    case ResourceType::kMesh:
        parent = m_meshItem;
        break;
    case ResourceType::kCubeTexture:
        parent = m_cubeTextureItem;
        break;
    case ResourceType::kAnimation:
        parent = m_animationItem;
        break;
    case ResourceType::kSkeleton:
        parent = m_skeletonItem;
        break;
    case ResourceType::kModel:
        parent = m_modelItem;
        break;
    case ResourceType::kShaderProgram:
        parent = m_shaderItem;
        break;
    case ResourceType::kPythonScript:
        parent = m_scriptItem;
        break;
    case ResourceType::kAudio:
        parent = m_audioItem;
        break;
    default:
        throw("Error, item type not recognized");
        break;
    }
    if (parent)
        parent->addChild(item);
    else
        throw("No parent found");

    item->setWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::removeItem(ResourceItem * resourceItem)
{
    if(resourceItem)
        delete resourceItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::removeItem(const Uuid& handleId)
{
    const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandle(handleId);
    ResourceItem* item = getItem(handle);
    if (item) {
        delete item;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::ResourceItem * ResourceTreeWidget::getItem(std::shared_ptr<ResourceHandle> handle)
{
    if (!handle) {
        logWarning("No handle specified");
        return nullptr;
    }

    QTreeWidgetItemIterator it(this);
    while (*it) {
        ResourceItem* item = static_cast<ResourceItem*>(*it);
        if (item->handle()) {
            if (item->handle()->getUuid() == handle->getUuid()) {
                return item;
            }
        }
        ++it;
    }

    //throw("Error, no item found that corresponds to the given object");

    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::resizeColumns()
{
    resizeColumnToContents(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::initializeCategories()
{
    // Insert categorioes into the widget
    m_imageItem = new ResourceItem("Images");
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

    addTopLevelItem(m_imageItem);   
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
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::dropEvent(QDropEvent * event)
{
    QTreeWidget::dropEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::initializeWidget()
{
    setMinimumWidth(350);

    // Set tree widget settings
    setColumnCount(0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    // Enable drag and drop
    //setDragEnabled(true);
    //setDragDropMode(DragDrop);
    //setDefaultDropAction(Qt::MoveAction);
    //setDragDropOverwriteMode(false); // is default, but making explicit for reference

    // Insert categorioes into the widget
    initializeCategories();

    // Initialize add actions
    m_addTexture = new QAction(tr("&Load Texture"), this);
    m_addTexture->setStatusTip("Load a texture into the scenario");
    connect(m_addTexture,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadTextureCommand(m_engine,
                "Load Texture"));
    });

    m_addMaterial = new QAction(tr("&Load Material"), this);
    m_addMaterial->setStatusTip("Add an empty material to the scenario");
    connect(m_addMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMaterialCommand(m_engine,
                "Add Material"));
    });

    m_copyMaterial = new QAction(tr("&Copy Material"), this);
    m_copyMaterial->setStatusTip("Copy a material into the scenario");
    connect(m_copyMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new CopyMaterialCommand(m_engine,
                "Copy Material",
                *m_currentResourceItem->handle()->resourceAs<Material>()
            ));
    });

    m_addModel = new QAction(tr("&Add Model"), this);
    m_addModel->setStatusTip("Create a new model in the current scenario");
    connect(m_addModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddModelCommand(m_engine,
                "Add Model"));
    });

    m_loadModel = new QAction(tr("&Add Model"), this);
    m_loadModel->setStatusTip("Load a model into the scenario");
    connect(m_loadModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new LoadModelCommand(m_engine,
                "Load Model"));
    });

    m_addMesh = new QAction(tr("&Add Mesh"), this);
    m_addMesh->setStatusTip("Create a mesh in the current scenario");
    connect(m_addMesh,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMeshCommand(m_engine, "Add Mesh"));
    });

    m_copyModel = new QAction(tr("&Copy Model"), this);
    m_copyModel->setStatusTip("Copy a model from this scenario");
    connect(m_copyModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new CopyModelCommand(m_engine, "Copy Model", 
                *m_currentResourceItem->handle()->resourceAs<Model>()
            ));
    });

    m_addShaderProgram = new QAction(tr("&Load Shader Program"), this);
    m_addShaderProgram->setStatusTip("Load a shader program into the scenario");
    connect(m_addShaderProgram,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddShaderCommand(m_engine,
                "Load Shader Program"));
    });

    // Initialize delete actions
    m_deleteMaterial = new QAction(tr("&Delete Material"), this);
    m_deleteMaterial->setStatusTip("Delete the selected material");
    connect(m_deleteMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteMaterialCommand(m_engine,
                m_currentResourceItem->handle()->resourceAs<Material>(),
                "Delete Material"));
    });

    m_deleteModel = new QAction(tr("&Delete Model"), this);
    m_deleteModel->setStatusTip("Delete the selected model");
    connect(m_deleteModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteModelCommand(m_engine,
                m_currentResourceItem->handle()->resourceAs<Model>(),
                "Delete Model"));
    });

    m_deleteShaderProgram = new QAction(tr("&Delete Shader Program"), this);
    m_deleteShaderProgram->setStatusTip("Delete the selected shader program");
    connect(m_deleteShaderProgram,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteShaderCommand(m_engine,
                m_currentResourceItem->handle()->resourceAs<ShaderProgram>(),
                "Delete Shader Program"));
    });

    // Connect signal for double click events
    connect(this, &ResourceTreeWidget::itemDoubleClicked,
        this, &ResourceTreeWidget::onItemDoubleClicked);

    // Connect signal for item expansion
    connect(this, &ResourceTreeWidget::itemExpanded,
        this, &ResourceTreeWidget::onItemExpanded);

    // Connect signal for repopulating on scenario load
    connect(m_engine,
        &CoreEngine::scenarioLoaded,
        this,
        &ResourceTreeWidget::repopulate,
        Qt::QueuedConnection);


    // Connect signals for repopulating on resource modifications
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceChanged,
        this,
        &ResourceTreeWidget::reloadItem,
        Qt::QueuedConnection);
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceAdded,
        this,
        &ResourceTreeWidget::reloadItem,
        Qt::QueuedConnection);
    connect(m_engine->resourceCache(),
        &ResourceCache::resourceDeleted,
        this,
        static_cast<void(ResourceTreeWidget::*)(const Uuid&)>(&ResourceTreeWidget::removeItem),
        Qt::QueuedConnection);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
        if (item->handle()) {
            switch (item->handle()->getResourceType()) {
            case ResourceType::kMaterial:
                menu.addAction(m_copyMaterial);
                if (!item->handle()->isCore()) {
                    menu.addAction(m_deleteMaterial);
                }
                break;
            case ResourceType::kModel:
                menu.addAction(m_copyModel);
                if (!item->handle()->isCore()) {
                    menu.addAction(m_deleteModel);
                }
                break;
            case ResourceType::kShaderProgram:
                if (!item->handle()->isCore()) {
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



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}
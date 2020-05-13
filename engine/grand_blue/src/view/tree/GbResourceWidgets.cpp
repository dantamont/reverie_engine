#include "GbResourceWidgets.h"

#include "../../model_control/commands/GbActionManager.h"

#include "../../core/resource/GbResourceCache.h"
#include "../../core/resource/GbResource.h"
#include "../../core/rendering/models/GbModel.h"
#include "../../core/rendering/materials/GbCubeMap.h"
#include "../../core/rendering/materials/GbMaterial.h"
#include "../../core/rendering/shaders/GbShaders.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../../core/processes/GbProcessManager.h"
#include "../../core/readers/GbJsonReader.h"

#include "../GbWidgetManager.h"
#include "../../GbMainWindow.h"

namespace Gb {

// TODO: Remove shared pointers to resources, need to be WEAK
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
    QString name = "material_" + Uuid().asString();
    m_material = std::make_shared<Material>(m_engine, name);
    m_engine->resourceCache()->materials().emplace(m_material->getName().toLower(), 
        m_material);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(m_material);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddMaterialCommand::undo()
{
    // Delete material
    m_materialJson = m_material->asJson();
    m_engine->resourceCache()->removeMaterial(m_material);
    m_material = nullptr;

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceDeleted(m_material);
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
    auto mtl = m_engine->resourceCache()->createMaterial(m_materialJson);
    emit m_engine->resourceCache()->resourceAdded(mtl);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyMaterialCommand::undo()
{
    QString name = m_materialJson["name"].toString();
    auto mtl = m_engine->resourceCache()->getMaterial(name);
    m_engine->resourceCache()->removeMaterial(name);
    emit m_engine->resourceCache()->resourceDeleted(mtl);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QString CopyMaterialCommand::getUniqueName()
{
    QString name = m_materialJson["name"].toString().toLower();
    int count = 1;
    QString newName = name;
    while (m_engine->resourceCache()->materials().find(newName)
        != m_engine->resourceCache()->materials().end()) {
        newName = name + "_" + QString::number(count++);
    }
    return newName;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Add Model Command
///////////////////////////////////////////////////////////////////////////////////////////////////

AddModelCommand::AddModelCommand(CoreEngine * core, 
    const QString & text, 
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_modelWidget(nullptr),
    m_model(nullptr)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AddModelCommand::~AddModelCommand()
{
    delete m_modelWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddModelCommand::redo()
{
    // Create widget for loading in model
    if (!m_modelWidget) {
        createModelWidget();
    }
    
    // Show add model dialog on redo
    m_modelWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddModelCommand::undo()
{
    if (m_model) {
        m_modelJson = m_model->asJson();
        m_engine->resourceCache()->removeModel(m_model);
        emit m_engine->resourceCache()->resourceDeleted(m_model);
        m_model = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddModelCommand::createModelWidget()
{
    // Create widget to add model to resource cache
    m_modelWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    // Create new layouts
    QHBoxLayout* modelLayout = new QHBoxLayout();
    QLabel* modelLabel = new QLabel();
    modelLabel->setText("Model: ");
    View::FileLoadWidget* modelFileWidget = new View::FileLoadWidget(m_engine,
        "",
        "Open Model",
        "Models (*.obj)");
    modelLayout->addWidget(modelLabel);
    modelLayout->addWidget(modelFileWidget);
    modelLayout->setAlignment(Qt::AlignCenter);
    modelFileWidget->connect(modelFileWidget->lineEdit(), &QLineEdit::textChanged,
        m_modelWidget,
        [&](const QString& text) {
        m_modelFileOrName = text;
    });

    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    QDialogButtonBox* buttons = new QDialogButtonBox(dialogButtons);
    m_modelWidget->connect(buttons, &QDialogButtonBox::accepted, m_modelWidget,
        [&]() {

        QFile modelFile(m_modelFileOrName);
        if (modelFile.exists()) {
            // Load model file if it exists
            m_model = m_engine->resourceCache()->getModelByFilePath(m_modelFileOrName);
        }
        else {
            // Obtain model by name if the file does not exist
            m_model = m_engine->resourceCache()->getModel(m_modelFileOrName);
        }

        if (!m_model) {
            logError("Error, failed to load model, filepath/name " 
                + m_modelFileOrName + " not found");
        }

        // Repopulate resource widget
        emit m_engine->resourceCache()->resourceAdded(m_model);

        m_modelWidget->close();
    });

    // Add layouts to main layout
    layout->addLayout(modelLayout);
    layout->addWidget(buttons);

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    m_modelWidget->setLayout(layout);
    m_modelWidget->setWindowTitle("Select Model File");
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
    auto model = m_engine->resourceCache()->createModel(m_modelJson);
    emit m_engine->resourceCache()->resourceAdded(model);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CopyModelCommand::undo()
{
    QString name = m_modelJson["name"].toString();
    auto model = m_engine->resourceCache()->getModel(name);
    m_engine->resourceCache()->removeModel(name);
    emit m_engine->resourceCache()->resourceDeleted(model);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QString CopyModelCommand::getUniqueName()
{
    QString name = m_modelJson["name"].toString().toLower();
    int count = 1;
    QString newName = name;
    while (m_engine->resourceCache()->models().find(newName)
        != m_engine->resourceCache()->models().end()) {
        newName = name + "_" + QString::number(count++);
    }
    return newName;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddShaderCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
AddShaderCommand::AddShaderCommand(CoreEngine * core, 
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_shaderProgram(nullptr),
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
    if (m_shaderProgram) {
        m_shaderJson = m_shaderProgram->asJson();
        m_engine->resourceCache()->removeShaderProgram(m_shaderProgram);
        emit m_engine->resourceCache()->resourceDeleted(m_shaderProgram);
        m_shaderProgram = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AddShaderCommand::createShaderWidget()
{
    // Create widget to add shader to resource cache
    m_shaderWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    // Create new layouts
    QHBoxLayout* fragLayout = new QHBoxLayout();
    QLabel* fragLabel = new QLabel();
    fragLabel->setText("Fragment Shader: ");
    View::FileLoadWidget* fragmentWidget = new View::FileLoadWidget(m_engine,
        "",
        "Open Fragment Shader",
        "Fragment Shaders (*.frag)");
    fragLayout->addWidget(fragLabel);
    fragLayout->addWidget(fragmentWidget);
    fragLayout->setAlignment(Qt::AlignCenter);
    fragmentWidget->connect(fragmentWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_fragFile = text;
    });

    QHBoxLayout* vertLayout = new QHBoxLayout();
    QLabel* vertLabel = new QLabel();
    vertLabel->setText("Vertex Shader: ");
    View::FileLoadWidget*  vertexWidget = new View::FileLoadWidget(m_engine,
        "",
        "Open Vertex Shader",
        "Vertex Shaders (*.vert)");
    vertLayout->addWidget(vertLabel);
    vertLayout->addWidget(vertexWidget);
    vertLayout->setAlignment(Qt::AlignCenter);
    vertexWidget->connect(vertexWidget->lineEdit(), &QLineEdit::textChanged,
        m_shaderWidget,
        [&](const QString& text) {
        m_vertFile = text;
    });

    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    QDialogButtonBox* buttons = new QDialogButtonBox(dialogButtons);
    m_shaderWidget->connect(buttons, &QDialogButtonBox::accepted, m_shaderWidget,
        [&]() {

        QString name = m_shaderProgram->getName().toLower();
        m_shaderProgram = std::make_shared<ShaderProgram>(m_vertFile, m_fragFile);
        m_engine->resourceCache()->shaderPrograms().emplace(name, m_shaderProgram);

        // Repopulate resource widget
        emit m_engine->resourceCache()->resourceAdded(m_shaderProgram);

        m_shaderWidget->close();
    });

    // Add layouts to main layout
    layout->addLayout(fragLayout);
    layout->addLayout(vertLayout);
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
    std::shared_ptr<Material> material,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_material(material)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteMaterialCommand::~DeleteMaterialCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteMaterialCommand::redo()
{
    m_materialJson = m_material->asJson();
    m_engine->resourceCache()->removeMaterial(m_material);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceDeleted(m_material);
    m_material = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteMaterialCommand::undo()
{
    // Add material back to resource cache
    // TODO: Make it such that materials are added back into models
    m_material = std::make_shared<Material>(m_engine, m_materialJson);
    m_engine->resourceCache()->materials().emplace(m_material->getName().toLower(), 
        m_material);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(m_material);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteModelCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteModelCommand::DeleteModelCommand(CoreEngine * core,
    std::shared_ptr<Model> model,
    const QString & text,
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_model(model)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteModelCommand::~DeleteModelCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteModelCommand::redo()
{
    m_modelJson = m_model->asJson();
    m_engine->resourceCache()->removeModel(m_model);
    
    // Repopulate model widget
    emit m_engine->resourceCache()->resourceDeleted(m_model);
    m_model = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteModelCommand::undo()
{
    // Add model back to resource cache
    // TODO: Make it such that models are added back into renderers
    // Note: Does not add model back into renderers, this must be done manually
    m_model = Model::create(m_engine, m_modelJson);
    m_engine->resourceCache()->models().emplace(m_model->getName().toLower(), m_model);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(m_model);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteShaderCommand
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteShaderCommand::DeleteShaderCommand(CoreEngine * core, 
    std::shared_ptr<ShaderProgram> shader,
    const QString & text, 
    QUndoCommand * parent) :
    UndoCommand(core, text, parent),
    m_shaderProgram(shader)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
DeleteShaderCommand::~DeleteShaderCommand()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteShaderCommand::redo()
{
    m_shaderJson = m_shaderProgram->asJson();
    m_engine->resourceCache()->removeShaderProgram(m_shaderProgram);

    // Repopulate model widget
    emit m_engine->resourceCache()->resourceDeleted(m_shaderProgram);
    m_shaderProgram = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteShaderCommand::undo()
{
    // Add shader back to resource cache
    m_shaderProgram = std::make_shared<ShaderProgram>(m_shaderJson);
    m_engine->resourceCache()->shaderPrograms().emplace(m_shaderProgram->getName().toLower(),
        m_shaderProgram);

    // Repopulate resource widget
    emit m_engine->resourceCache()->resourceAdded(m_shaderProgram);
}



namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceLike
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceLike::ResourceLike() :
    m_handle(nullptr),
    m_model(nullptr),
    m_material(nullptr),
    m_shaderProgram(nullptr) {

}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceLike::ResourceLike(std::shared_ptr<Object> object) :
    m_handle(nullptr),
    m_model(nullptr),
    m_material(nullptr),
    m_shaderProgram(nullptr)
{
    QString className(object->className());
    if (className.contains("ResourceHandle")) {
        m_handle = std::static_pointer_cast<ResourceHandle>(object);
    }
    else if (className.contains("Model") || className.contains("CubeMap")) {
        m_model = std::static_pointer_cast<Model>(object);
    }
    else if (className.contains("Material")) {
        m_material = std::static_pointer_cast<Material>(object);
    }
    else if (className.contains("Shader")) {
        m_shaderProgram = std::static_pointer_cast<ShaderProgram>(object);
    }
    else {
        throw("Error, bad type");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceLike::~ResourceLike()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Object> ResourceLike::object() const
{
    if (m_handle) {
        return m_handle;
    }
    else if (m_model) {
        return m_model;
    }
    else if (m_material) {
        return m_material;
    }
    else if (m_shaderProgram) {
        return m_shaderProgram;
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Serializable> ResourceLike::serializable() const
{
    if (m_handle) {
        return m_handle;
    }
    else if (m_model) {
        return m_model;
    }
    else if (m_material) {
        return m_material;
    }
    else if (m_shaderProgram) {
        return m_shaderProgram;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceJsonWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ResourceHandle>& resource, QWidget *parent) :
    ParameterWidget(core, parent),
    m_object(resource)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<Model>& model, QWidget *parent) :
    ParameterWidget(core, parent),
    m_object(model)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<Material>& mtl, QWidget *parent) :
    ParameterWidget(core, parent),
    m_object(mtl)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ResourceJsonWidget::ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ShaderProgram>& shader, QWidget *parent) :
    ParameterWidget(core, parent),
    m_object(shader)
{
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
void ResourceJsonWidget::initializeWidgets()
{
    m_typeLabel = new QLabel(QString(m_object.object()->className()) + ": " + m_object.object()->getName());
    m_typeLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_textEdit = new QTextEdit();
    m_textEdit->setText(JsonReader::getJsonValueAsQString(m_object.serializable()->asJson(), true));
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
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
        QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);
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

    // Make connection to resize component 
    connect(m_confirmButton, &QPushButton::clicked, this, [this](bool checked) {
        //QProgressDialog progress("Reloading resource", "Close", 0, 1, 
        //    m_engine->widgetManager()->mainWindow());
        //progress.setWindowModality(Qt::WindowModal);
        Q_UNUSED(checked)

        // Get text and return if not valid JSON
        const QString& text = m_textEdit->toPlainText();
        QJsonDocument contents = JsonReader::getQStringAsJsonDocument(text);
        if (contents.isNull()) return;

        QString name = m_object.object()->getName();

        // Pause scenario to edit resource
        SimulationLoop* simLoop = m_engine->simulationLoop();
        bool wasPlaying = simLoop->isPlaying();
        if (wasPlaying) {
            simLoop->pause();
        }
        m_engine->simulationLoop()->pause();

        // Edit resource via JSON
        m_object.serializable()->loadFromJson(contents.object());

        // If object name has changed, emit signal to repopulate widget
        if (name != m_object.object()->getName()) {
            emit m_engine->resourceCache()->resourceChanged(m_object.object());
        }

        // Unpause scenario
        if (wasPlaying) {
            simLoop->play();
        }

        //progress.setValue(1);
    });
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
ResourceItem::ResourceItem(std::shared_ptr<Object> object) :
    QTreeWidgetItem((int)getItemType(object)),
    m_object(object),
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
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    ResourceTreeWidget * parentWidget = static_cast<ResourceTreeWidget*>(treeWidget());

    // Set widget
    QJsonDocument doc(m_object.serializable()->asJson().toObject());
    QString rep(doc.toJson(QJsonDocument::Indented));
    if (m_object.m_handle) {
        m_widget = new ResourceJsonWidget(parentWidget->m_engine, m_object.m_handle);
    }
    else if (m_object.m_model) {
        m_widget = new ResourceJsonWidget(parentWidget->m_engine, m_object.m_model);
    }
    else if (m_object.m_material) {
        m_widget = new ResourceJsonWidget(parentWidget->m_engine, m_object.m_material);
    }
    else if (m_object.m_shaderProgram) {
        m_widget = new ResourceJsonWidget(parentWidget->m_engine, m_object.m_shaderProgram);
    }
    else {
        throw("Error, correct object type not found");
    }

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
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
    else if (name == "Material") {
        return kMaterial;
    }
    else if (name == "Model" || name == "CubeMap") {
        return kModel;
    }
    else if (name.contains("ShaderProgram")) {
        return kShaderProgram;
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
    m_resourceItem(nullptr),
    m_modelItem(nullptr),
    m_materialItem(nullptr),
    m_shaderItem(nullptr)
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
    if (m_engine->processManager()->loadProcessCount()) return;

    // Clear the widget
    clear();

    // Set header item and disable scene object dragging
    //setHeaderItem(scenario);
    invisibleRootItem()->setFlags(invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled);

    // Re-initialize categories
    initializeCategories();

    // Get resource cache
    ResourceCache* cache = m_engine->resourceCache();

    // Add shader programs
    for (const auto& shaderPair : cache->shaderPrograms()) {
        addItem(shaderPair.second);
    }

    // Add models
    for (const auto& modelPair : cache->models()) {
        addItem(modelPair.second);
    }

    // Add materials
    for (const auto& materialPair : cache->materials()) {
        addItem(materialPair.second);
    }

    // Add resource items
    for (const auto& resourcePair : cache->resources()) {
        addItem(resourcePair.second);
    }

    //setUpdatesEnabled(true);

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::reloadItem(std::shared_ptr<Object> object)
{
    if (getItem(object)) {
        removeItem(object);
    }
    
    addItem(object);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::addItem(std::shared_ptr<Object> object)
{
    // Create resource item
    ResourceItem* resourceItem = new View::ResourceItem(object);

    // Add resource item
    addItem(resourceItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::addItem(View::ResourceItem * item)
{
    // Insert resource item into the widget
    ResourceItem* parent = nullptr;
    switch (item->itemType()) {
    case ResourceItem::kResourceHandle:
        parent = m_resourceItem;
        break;
    case ResourceItem::kMaterial:
        parent = m_materialItem;
        break;
    case ResourceItem::kModel:
        parent = m_modelItem;
        break; 
    case ResourceItem::kShaderProgram:
        parent = m_shaderItem;
        break;
    case ResourceItem::kCategory:
    default:
        throw("Error, invalid item type");
    }
    parent->addChild(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::removeItem(ResourceItem * resourceItem)
{
    if(resourceItem)
        delete resourceItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ResourceTreeWidget::removeItem(std::shared_ptr<Object> itemObject)
{
    ResourceItem* item = getItem(itemObject);
    if(item)
        delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::ResourceItem * ResourceTreeWidget::getItem(std::shared_ptr<Object> itemObject)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ResourceItem* item = static_cast<ResourceItem*>(*it);
        if (item->object().object()) {
            if (item->object().object()->getUuid() == itemObject->getUuid()) {
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
    m_resourceItem = new ResourceItem("Resources");
    m_shaderItem = new ResourceItem("Shaders");
    m_modelItem = new ResourceItem("Models");
    m_materialItem = new ResourceItem("Materials");

    addTopLevelItem(m_shaderItem);
    addTopLevelItem(m_modelItem);
    addTopLevelItem(m_materialItem);
    //addTopLevelItem(scriptItem);
    addTopLevelItem(m_resourceItem);
    
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
    m_addMaterial = new QAction(tr("&Load Material"), this);
    m_addMaterial->setStatusTip("Load a material into the scenario");
    connect(m_addMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddMaterialCommand(m_engine,
                "Load Material"));
    });
    m_copyMaterial = new QAction(tr("&Copy Material"), this);
    m_copyMaterial->setStatusTip("Copy a material into the scenario");
    connect(m_copyMaterial,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new CopyMaterialCommand(m_engine,
                "Copy Material",
                *std::static_pointer_cast<Material>(m_currentResourceItem->object().object())
            ));
    });


    m_addModel = new QAction(tr("&Load Model"), this);
    m_addModel->setStatusTip("Load a model into the scenario");
    connect(m_addModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new AddModelCommand(m_engine,
                "Load Model"));
    });
    m_copyModel = new QAction(tr("&Copy Model"), this);
    m_copyModel->setStatusTip("Copy a model from this scenario");
    connect(m_copyModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new CopyModelCommand(m_engine, "Copy Model", 
                *std::static_pointer_cast<Model>(m_currentResourceItem->object().object())
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
                m_currentResourceItem->object().m_material,
                "Delete Material"));
    });

    m_deleteModel = new QAction(tr("&Delete Model"), this);
    m_deleteModel->setStatusTip("Delete the selected model");
    connect(m_deleteModel,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteModelCommand(m_engine,
                m_currentResourceItem->object().m_model,
                "Delete Model"));
    });

    m_deleteShaderProgram = new QAction(tr("&Delete Shader Program"), this);
    m_deleteShaderProgram->setStatusTip("Delete the selected shader program");
    connect(m_deleteShaderProgram,
        &QAction::triggered,
        m_engine->actionManager(),
        [this] {m_engine->actionManager()->performAction(
            new DeleteShaderCommand(m_engine,
                m_currentResourceItem->object().m_shaderProgram,
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
        static_cast<void(ResourceTreeWidget::*)(std::shared_ptr<Object>)>(&ResourceTreeWidget::removeItem),
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
        switch (item->itemType()) {
        case ResourceItem::kResourceHandle:
            break;
        case ResourceItem::kMaterial:
            menu.addAction(m_copyMaterial);
            menu.addAction(m_deleteMaterial);
            break;
        case ResourceItem::kModel:
            menu.addAction(m_copyModel);
            menu.addAction(m_deleteModel);
            break;
        case ResourceItem::kShaderProgram:
            const QString& name = m_currentResourceItem->object().m_shaderProgram->className();
            if (name != "BasicShaderProgram" && name != "CubemapShaderProgram") {
                menu.addAction(m_deleteShaderProgram);
            }
            break;
        }
    }
    else {
        // Options to add things when there is no item selected
        menu.addAction(m_addModel);
        menu.addAction(m_addMaterial);
        menu.addAction(m_addShaderProgram);
    }

    // Display menu at click location
    menu.exec(event->globalPos());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}
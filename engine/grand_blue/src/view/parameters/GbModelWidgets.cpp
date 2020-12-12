#include "GbModelWidgets.h"
#include "../style/GbFontIcon.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbModelComponent.h"
#include "../../core/rendering/models/GbModel.h"
#include "../../core/utils/GbMemoryManager.h"
#include "../../core/resource/GbResourceCache.h"
#include "../../core/resource/GbResource.h"
#include "../../core/GbCoreEngine.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LoadModelWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadModelWidget::LoadModelWidget(CoreEngine * core, QWidget * parent):
    ParameterWidget(core, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadModelWidget::~LoadModelWidget()
{
    delete m_fileLoadWidget;
    delete m_confirmButtons;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelWidget::initializeWidgets()
{
    // Load mode
    //m_loadMode = new QComboBox();
    //m_loadMode->addItem(SAIcon("file-import"), "Import");
    //m_loadMode->addItem(SAIcon("magic"), "Create");

    // File load widget
    //m_loadWidgets = new QWidget();
    m_fileLoadWidget = new View::FileLoadWidget(m_engine,
        "",
        "Open Model",
        "Models (*.obj *.fbx *.mdl)");

    // Dialog buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);

    // Create model widgets
    //m_createModelWidget = new ModelResourceWidget(m_engine, nullptr);

    // Stacked widget
    //m_stackedWidget = new QStackedWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelWidget::initializeConnections()
{
    // Switch mode
    //connect(m_loadMode, qOverload<int>(&QComboBox::currentIndexChanged),
    //    this,
    //    [&](int index) {
    //    m_stackedWidget->setCurrentIndex(index);
    //});

    // Cache file name locally
    connect(m_fileLoadWidget->lineEdit(), &QLineEdit::textChanged,
        this,
        [&](const QString& text) {
        m_fileName = text;
    });

    // Dialog buttons
    connect(m_confirmButtons, &QDialogButtonBox::accepted,
        this,
        [this]() {

        // Load model from file
        // TODO: Check file extension, creating a new resource folder if not MDL
        QFile modelFile(m_fileName);
        if (modelFile.exists()) {
            // Load model file if path exists
            m_modelID = m_engine->resourceCache()->guaranteeHandleWithPath(m_fileName,
                Resource::kModel)->getUuid();
        }

        // Close this widget
        close();
    });
    connect(m_confirmButtons, &QDialogButtonBox::rejected,
        this,
        [this]() {
        // Close this widget
        close();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadModelWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    //QVBoxLayout* loadWidgetsLayout = new QVBoxLayout();
    //loadWidgetsLayout->setSpacing(0);

    // Model load widgets
    QBoxLayout* fileLoadLayout = LabeledLayout("Model File:", m_fileLoadWidget);
    fileLoadLayout->setAlignment(Qt::AlignCenter);
    //loadWidgetsLayout->addLayout(loadWidgetsLayout);
    //loadWidgetsLayout->addWidget(m_confirmButtons);
    //m_loadWidgets->setLayout(loadWidgetsLayout);

    // Model create widgets
    //m_stackedWidget->addWidget(m_loadWidgets);
    //m_stackedWidget->addWidget(m_createModelWidget);

    //m_mainLayout->addLayout(LabeledLayout("Mode:", m_loadMode));
    //m_mainLayout->addWidget(m_stackedWidget);

    m_mainLayout->addLayout(fileLoadLayout);
    m_mainLayout->addWidget(m_confirmButtons);

    setWindowTitle("Load Model");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshResourceWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
MeshResourceWidget::MeshResourceWidget(CoreEngine * core, 
    const std::shared_ptr<ResourceHandle>& meshHandle,
    QWidget * parent) :
    ParameterWidget(core, parent),
    m_meshHandle(meshHandle)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
MeshResourceWidget::~MeshResourceWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshResourceWidget::initializeWidgets()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshResourceWidget::initializeConnections()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshResourceWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateMeshWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CreateMeshWidget::CreateMeshWidget(CoreEngine* core, QWidget* parent) :
    ParameterWidget(core, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CreateMeshWidget::~CreateMeshWidget()
{
    //delete m_meshName;
    delete m_shapeOptions;
    delete m_confirmButtons;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CreateMeshWidget::initializeWidgets()
{
    m_shapeType = new QComboBox();
    for (const auto& namePair : m_engine->resourceCache()->polygonCache()->POLYGON_NAMES) {
        if (namePair.first < 2) continue; // Skip rectangle and cube
        QString name = namePair.second;
        name[0] = name[0].toUpper();
        m_shapeType->addItem(name);
    }

    // Sphere
    m_sphereWidget = new QWidget();
    QVBoxLayout* sphereLayout = new QVBoxLayout();
    sphereLayout->setSpacing(0);

    m_sphereStackCount = new QLineEdit("1.0");
    m_sphereStackCount->setMaximumWidth(75);
    m_sphereStackCount->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    m_sphereSectorCount = new QLineEdit("1.0");
    m_sphereSectorCount->setMaximumWidth(75);
    m_sphereSectorCount->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    sphereLayout->addLayout(LabeledLayout("Stack Count:", m_sphereStackCount));
    sphereLayout->addLayout(LabeledLayout("Sector Count:", m_sphereSectorCount));
    m_sphereWidget->setLayout(sphereLayout);

    // Grid plane
    m_gridPlaneWidget = new QWidget();
    QVBoxLayout* gridPlaneLayout = new QVBoxLayout();
    gridPlaneLayout->setSpacing(0);

    m_gridPlaneSpacing = new QLineEdit("1.0");
    m_gridPlaneSpacing->setMaximumWidth(75);
    m_gridPlaneSpacing->setValidator(new QDoubleValidator(1e-7, 1e7, 4));
    gridPlaneLayout->addLayout(LabeledLayout("Spacing:", m_gridPlaneSpacing));

    m_numPlaneHalfSpaces = new QLineEdit("5");
    m_numPlaneHalfSpaces->setMaximumWidth(75);
    m_numPlaneHalfSpaces->setValidator(new QIntValidator(1, 1e6));
    gridPlaneLayout->addLayout(LabeledLayout("Half Spaces:", m_numPlaneHalfSpaces));

    m_gridPlaneWidget->setLayout(gridPlaneLayout);

    // Grid cube
    m_gridCubeWidget = new QWidget();
    QVBoxLayout* gridCubeLayout = new QVBoxLayout();
    gridCubeLayout->setSpacing(0);

    m_gridCubeSpacing = new QLineEdit("1.0");
    m_gridCubeSpacing->setMaximumWidth(75);
    m_gridCubeSpacing->setValidator(new QDoubleValidator(1e-7, 1e7, 4));
    gridCubeLayout->addLayout(LabeledLayout("Spacing:", m_gridCubeSpacing));

    m_numCubeHalfSpaces = new QLineEdit("5");
    m_numCubeHalfSpaces->setMaximumWidth(75);
    m_numCubeHalfSpaces->setValidator(new QIntValidator(1, 1e6));
    gridCubeLayout->addLayout(LabeledLayout("Half Spaces:", m_numCubeHalfSpaces));

    m_gridCubeWidget->setLayout(gridCubeLayout);


    // Cylinder
    m_cylinderWidget = new QWidget();
    QVBoxLayout* cylinderLayout = new QVBoxLayout();
    cylinderLayout->setSpacing(0);

    m_baseRadius = new QLineEdit("1.0");
    m_baseRadius->setMaximumWidth(75);
    m_baseRadius->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    m_topRadius = new QLineEdit("1.0");
    m_topRadius->setMaximumWidth(75);
    m_topRadius->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    m_cylinderHeight = new QLineEdit("1.0");
    m_cylinderHeight->setMaximumWidth(75);
    m_cylinderHeight->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    m_cylinderSectorCount = new QLineEdit("36");
    m_cylinderSectorCount->setMaximumWidth(75);
    m_cylinderSectorCount->setValidator(new QIntValidator(3, 1e5));

    m_cylinderStackCount = new QLineEdit("1");
    m_cylinderStackCount->setMaximumWidth(75);
    m_cylinderStackCount->setValidator(new QIntValidator(3, 1e5));

    cylinderLayout->addLayout(LabeledLayout("Base Radius:", m_baseRadius));
    cylinderLayout->addLayout(LabeledLayout("Top Radius:", m_topRadius));
    cylinderLayout->addLayout(LabeledLayout("Height:", m_cylinderHeight));
    cylinderLayout->addLayout(LabeledLayout("Sector Count:", m_cylinderSectorCount));
    cylinderLayout->addLayout(LabeledLayout("Stack Count:", m_cylinderStackCount));
    m_cylinderWidget->setLayout(cylinderLayout);

    // Capsule
    m_capsuleWidget = new QWidget();
    QVBoxLayout* capsuleLayout = new QVBoxLayout();
    capsuleLayout->setSpacing(0);

    m_capsuleRadius = new QLineEdit("1.0");
    m_capsuleRadius->setMaximumWidth(75);
    m_capsuleRadius->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    m_capsuleHalfHeight = new QLineEdit("1.0");
    m_capsuleHalfHeight->setMaximumWidth(75);
    m_capsuleHalfHeight->setValidator(new QDoubleValidator(1e-7, 1e7, 4));

    capsuleLayout->addLayout(LabeledLayout("Radius:", m_capsuleRadius));
    capsuleLayout->addLayout(LabeledLayout("Half-Height:", m_capsuleHalfHeight));
    m_capsuleWidget->setLayout(capsuleLayout);

    // Main shape widget
    m_shapeOptions = new QStackedWidget();
    m_shapeOptions->addWidget(m_sphereWidget);
    m_shapeOptions->addWidget(m_gridPlaneWidget);
    m_shapeOptions->addWidget(m_gridCubeWidget);
    m_shapeOptions->addWidget(m_cylinderWidget);
    m_shapeOptions->addWidget(m_capsuleWidget);
    m_shapeOptions->setCurrentIndex(0);

    // Dialog buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CreateMeshWidget::initializeConnections()
{
    // Shape type
    connect(m_shapeType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        m_shapeOptions->setCurrentIndex(idx);
    }
    );

    // Dialog buttons
    connect(m_confirmButtons, &QDialogButtonBox::accepted,
        this,
        [this]() {

        createMesh();

        // Close this widget
        close();
    });
    connect(m_confirmButtons, &QDialogButtonBox::rejected,
        this,
        [this]() {
        // Close this widget
        close();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CreateMeshWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    m_mainLayout->addLayout(LabeledLayout("Shape:", m_shapeType));
    m_mainLayout->addWidget(m_shapeOptions);
    m_mainLayout->addWidget(m_confirmButtons);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CreateMeshWidget::createMesh()
{
    PolygonCache::PolygonType shapeType = PolygonCache::PolygonType(
        m_shapeOptions->currentIndex() + 2);
    const std::shared_ptr<PolygonCache>& cache = m_engine->resourceCache()->polygonCache();;
    switch (shapeType) {
    case PolygonCache::kLatLonSphere:
        m_meshHandleID = cache->getSphere(m_sphereStackCount->text().toInt(),
            m_sphereSectorCount->text().toInt())->handle()->getUuid();
        break;
    case PolygonCache::kGridPlane:
        m_meshHandleID = cache->getGridPlane(m_gridPlaneSpacing->text().toDouble(),
            m_numPlaneHalfSpaces->text().toInt())->handle()->getUuid();
        break;
    case PolygonCache::kGridCube:
        m_meshHandleID = cache->getGridCube(m_gridCubeSpacing->text().toDouble(),
            m_numCubeHalfSpaces->text().toInt())->handle()->getUuid();
        break;
    case PolygonCache::kCylinder:
        m_meshHandleID = cache->getCylinder(m_baseRadius->text().toDouble(),
            m_topRadius->text().toDouble(),
            m_cylinderHeight->text().toDouble(),
            m_cylinderSectorCount->text().toInt(),
            m_cylinderStackCount->text().toInt())->handle()->getUuid();
        break;
    case PolygonCache::kCapsule:
        m_meshHandleID = cache->getCapsule(m_capsuleRadius->text().toDouble(),
            m_capsuleHalfHeight->text().toDouble())->handle()->getUuid();
        break;
    case PolygonCache::kRectangle:
    case PolygonCache::kCube:
    default:
        throw("Error, shape type not recognized");
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// MeshTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
MeshTreeWidget::MeshTreeWidget(CoreEngine* engine,
    const std::shared_ptr<ResourceHandle>& modelHandle,
    QWidget* parent) :
    TreeWidget(engine, "Meshes", parent),
    m_modelHandle(modelHandle)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
MeshTreeWidget::~MeshTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Add loaded meshes
    //for (const auto& resourcePair : m_engine->resourceCache()->resources()) {
    //    if (resourcePair.second->getResourceType() != Resource::kMesh) continue;
    //    addItem(resourcePair.second);
    //}
    m_engine->resourceCache()->resources().forEach(
        [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
    {
        if (resourcePair.second->getResourceType() != Resource::kMesh) {
            return;
        }
        addItem(resourcePair.second);
    });

    // Resize columns
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::addItem(const std::shared_ptr<ResourceHandle>& meshHandle)
{
    if (meshHandle->getResourceType() != Resource::kMesh) {
        throw("Handle is of the incorrect resource type");
    }
    ResourceHandle* handlePtr = meshHandle.get();
    TreeItem<ResourceHandle>* item = new TreeItem<ResourceHandle>(handlePtr);
    item->setText(0, meshHandle->getName());
    addTopLevelItem(item);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::removeItem(const std::shared_ptr<ResourceHandle>& meshHandle)
{
    QTreeWidgetItem* item = getItem(*meshHandle);
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    Q_UNUSED(item)
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    setMinimumSize(350, 350);
    initializeAsList();
    //enableDragAndDrop();

    // Initialize actions
    //addAction(kNoItemSelected,
    //    "Add Shader Preset",
    //    "Add a shader preset to the scenario",
    //    [this] {
    //    bool created;
    //    m_engine->resourceCache()->getShaderPreset(Uuid::UniqueName("preset_"), created);
    //    if (!created) throw("Error, resource not created");
    //    repopulate();
    //});

    //addAction(kItemSelected,
    //    "Remove Shader Preset",
    //    "Remove shader preset from the scenario",
    //    [this] {
    //    // Add sorting layer
    //    m_engine->resourceCache()->removeShaderPreset(currentContextItem()->shaderPreset()->getName());
    //    repopulate();
    //});
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    // Create menu
    QMenu menu(this);

    // If a mesh is selected
    //if (!m_currentUniformWidget)
    //    menu.addAction(m_addUniform);
    //else
    //    menu.addAction(m_removeUniform);

    // Display menu at click location
    menu.exec(event->globalPos());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ModelResourceWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelResourceWidget::ModelResourceWidget(CoreEngine * core,
    const std::shared_ptr<ResourceHandle>& modelHandle,
    QWidget * parent):
    ParameterWidget(core, parent),
    m_modelHandle(modelHandle)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelResourceWidget::~ModelResourceWidget()
{
    delete m_meshesWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResourceWidget::initializeWidgets()
{
    m_nameWidget = new QLineEdit(Uuid::UniqueName("model"));
    m_meshesWidget = new MeshTreeWidget(m_engine, m_modelHandle);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResourceWidget::initializeConnections()
{
    connect(m_nameWidget, &QLineEdit::editingFinished,
        this,
        [this]() {
        m_modelHandle->setName(m_nameWidget->text());
        m_modelHandle->resourceAs<Model>()->setName(false);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResourceWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    m_mainLayout->addWidget(new QLabel("Meshes:"));
    m_mainLayout->addWidget(m_meshesWidget);
}





///////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
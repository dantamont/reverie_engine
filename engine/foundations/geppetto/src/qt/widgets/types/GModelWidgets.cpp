#include "geppetto/qt/widgets/types/GModelWidgets.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "fortress/system/memory/GPointerTypes.h"

#include "enums/GBasicPolygonTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

namespace rev {

LoadModelWidget::LoadModelWidget(WidgetManager* manager, QWidget * parent):
    ParameterWidget(manager, parent)
{
    initialize();
}

LoadModelWidget::~LoadModelWidget()
{
    delete m_fileLoadWidget;
    delete m_confirmButtons;
}

void LoadModelWidget::initializeWidgets()
{
    // File load widget
    // TODO: Create an output file prompt (".mdl") if format is not .mdl. This way,
    // the output location will be known and specified by the user
    m_fileLoadWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Open Model",
        "Models (*.obj *.fbx *.mdl)");

    // Dialog buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);

}

void LoadModelWidget::initializeConnections()
{
    // Cache file name locally
    connect(m_fileLoadWidget->lineEdit(), &QLineEdit::textChanged,
        this,
        [&](const QString& text) {
        m_loadModelMessage.setFilePath(text.toStdString().c_str());
    });

    // Dialog buttons
    connect(m_confirmButtons, &QDialogButtonBox::accepted,
        this,
        [this]() {

        m_modelId = Uuid();
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_loadModelMessage);

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

void LoadModelWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    // Model load widgets
    QBoxLayout* fileLoadLayout = LabeledLayout("Model File:", m_fileLoadWidget);
    fileLoadLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->addLayout(fileLoadLayout);
    m_mainLayout->addWidget(m_confirmButtons);

    setWindowTitle("Load Model");
}




//
//MeshResourceWidget::MeshResourceWidget(WidgetManager* manager,
//    const std::shared_ptr<ResourceHandle>& meshHandle,
//    QWidget * parent) :
//    ParameterWidget(manager, parent),
//    m_meshHandle(meshHandle)
//{
//    initialize();
//}
//
//MeshResourceWidget::~MeshResourceWidget()
//{
//}
//
//void MeshResourceWidget::initializeWidgets()
//{
//}
//
//void MeshResourceWidget::initializeConnections()
//{
//}
//
//void MeshResourceWidget::layoutWidgets()
//{
//    m_mainLayout = new QVBoxLayout();
//    m_mainLayout->setSpacing(0);
//}
//



// CreateMeshWidget

CreateMeshWidget::CreateMeshWidget(WidgetManager* manager, QWidget* parent) :
    ParameterWidget(manager, parent)
{
    initialize();
}

CreateMeshWidget::~CreateMeshWidget()
{
    //delete m_meshName;
    delete m_shapeOptions;
    delete m_confirmButtons;
}

void CreateMeshWidget::initializeWidgets()
{
    m_shapeType = new QComboBox();
    for (size_t i = 0; i < (size_t)EBasicPolygonType::eCOUNT; i++) {
        if (i < 2) continue; // Skip rectangle and cube
        GString name = GBasicPolygonType::ToString(GBasicPolygonType(i));
        name[0] = (char)GString(name[0]).toUpper();
        m_shapeType->addItem(name.c_str());
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

void CreateMeshWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    m_mainLayout->addLayout(LabeledLayout("Shape:", m_shapeType));
    m_mainLayout->addWidget(m_shapeOptions);
    m_mainLayout->addWidget(m_confirmButtons);
}

void CreateMeshWidget::createMesh()
{
    m_meshId = Uuid();
    GBasicPolygonType shapeType = GBasicPolygonType(m_shapeOptions->currentIndex() + 2);
    m_createMeshMessage.setUuid(m_meshId);
    m_createMeshMessage.setPolygonType(shapeType);
    switch ((EBasicPolygonType)shapeType) {
    case EBasicPolygonType::eLatLonSphere:
        m_createMeshMessage.setStackCount(m_sphereStackCount->text().toInt());
        m_createMeshMessage.setSectorCount(m_sphereSectorCount->text().toInt());
        break;
    case EBasicPolygonType::eGridPlane:
        m_createMeshMessage.setGridSpacing(m_gridPlaneSpacing->text().toDouble());
        m_createMeshMessage.setNumHalfSpaces(m_numPlaneHalfSpaces->text().toInt());
        break;
    case EBasicPolygonType::eGridCube:
        m_createMeshMessage.setGridSpacing(m_gridCubeSpacing->text().toDouble());
        m_createMeshMessage.setNumHalfSpaces(m_numCubeHalfSpaces->text().toInt());
        break;
    case EBasicPolygonType::eCylinder:
        m_createMeshMessage.setBaseRadius(m_baseRadius->text().toDouble());
        m_createMeshMessage.setTopRadius(m_topRadius->text().toDouble());
        m_createMeshMessage.setHeight(m_cylinderHeight->text().toDouble());
        m_createMeshMessage.setSectorCount(m_cylinderSectorCount->text().toInt());
        m_createMeshMessage.setStackCount(m_cylinderStackCount->text().toInt());
        break;
    case EBasicPolygonType::eCapsule:
        m_createMeshMessage.setRadius(m_capsuleRadius->text().toDouble());
        m_createMeshMessage.setHalfHeight(m_capsuleHalfHeight->text().toDouble());
        break;
    case EBasicPolygonType::eRectangle:
    case EBasicPolygonType::eCube:
    default:
        assert(false && "Error, shape type not recognized");
    }

    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_createMeshMessage);
}




//MeshTreeWidget::MeshTreeWidget(WidgetManager* manager,
//    const std::shared_ptr<ResourceHandle>& modelHandle,
//    QWidget* parent) :
//    TreeWidget(manager, nullptr, "Meshes", parent),
//    m_modelHandle(modelHandle)
//{
//    initializeWidget();
//    repopulate();
//}
//
//MeshTreeWidget::~MeshTreeWidget()
//{
//}
//
//void MeshTreeWidget::repopulate()
//{
//    // Clear the widget
//    clear();
//
//    // Add loaded meshes
//    ResourceCache::Instance().resources().forEach(
//        [this](const std::pair<Uuid, std::shared_ptr<ResourceHandle>>& resourcePair)
//    {
//        if (resourcePair.second->getResourceType() != EResourceType::eMesh) {
//            return;
//        }
//        addItem(resourcePair.second);
//    });
//
//    // Resize columns
//    resizeColumns();
//}
//
//void MeshTreeWidget::addItem(const std::shared_ptr<ResourceHandle>& meshHandle)
//{
//    if (meshHandle->getResourceType() != EResourceType::eMesh) {
//        Logger::Throw("Handle is of the incorrect resource type");
//    }
//    ResourceHandle* handlePtr = meshHandle.get();
//    TreeItem<ResourceHandle>* item = new TreeItem<ResourceHandle>(handlePtr);
//    item->setText(0, meshHandle->getName().c_str());
//    addTopLevelItem(item);
//}
//
//void MeshTreeWidget::removeItem(const std::shared_ptr<ResourceHandle>& meshHandle)
//{
//    QTreeWidgetItem* item = getItem(*meshHandle);
//    delete item;
//}
//
//void MeshTreeWidget::initializeItem(QTreeWidgetItem * item)
//{
//    Q_UNUSED(item)
//}
//
//void MeshTreeWidget::initializeWidget()
//{
//    TreeWidget::initializeWidget();
//
//    setMinimumSize(350, 350);
//    initializeAsList();
//}
//
//void MeshTreeWidget::contextMenuEvent(QContextMenuEvent * event)
//{
//    // Create menu
//    QMenu menu(this);
//
//    // Display menu at click location
//    menu.exec(event->globalPos());
//}
//





//ModelResourceWidget::ModelResourceWidget(CoreEngine * core,
//    const std::shared_ptr<ResourceHandle>& modelHandle,
//    QWidget * parent):
//    ParameterWidget(core, parent),
//    m_modelHandle(modelHandle)
//{
//    initialize();
//}
//
//ModelResourceWidget::~ModelResourceWidget()
//{
//    delete m_meshesWidget;
//}
//
//void ModelResourceWidget::initializeWidgets()
//{
//    m_nameWidget = new QLineEdit(Uuid::UniqueName("model").c_str());
//    m_meshesWidget = new MeshTreeWidget(m_engine, m_modelHandle);
//}
//
//void ModelResourceWidget::initializeConnections()
//{
//    connect(m_nameWidget, &QLineEdit::editingFinished,
//        this,
//        [this]() {
//        m_modelHandle->setName(m_nameWidget->text().toStdString());
//        //m_modelHandle->resourceAs<Model>()->setName(false);
//    });
//}
//
//void ModelResourceWidget::layoutWidgets()
//{
//    m_mainLayout = new QVBoxLayout();
//    m_mainLayout->setSpacing(0);
//
//    m_mainLayout->addWidget(new QLabel("Meshes:"));
//    m_mainLayout->addWidget(m_meshesWidget);
//}



} // rev
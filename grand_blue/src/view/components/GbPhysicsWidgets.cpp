#include "GbPhysicsWidgets.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/mixins/GbLoadable.h"
#include "../../core/readers/GbJsonReader.h"
#include "../GbWidgetManager.h"
#include "../../core/physics/GbPhysicsShape.h"
#include "../../core/physics/GbPhysicsGeometry.h"
#include "../../core/physics/GbPhysicsMaterial.h"
#include "../../core/physics/GbPhysicsManager.h"
#include "../../core/physics//GbPhysicsActor.h"
#include "../style/GbFontIcon.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicsGeometryWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsGeometryWidget::PhysicsGeometryWidget(CoreEngine* core,
    const std::shared_ptr<PhysicsShapePrefab>& prefab,
    QWidget* parent,
    RigidBody* body) :
    ParameterWidget(core, parent),
    m_prefab(prefab),
    m_body(body)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsGeometryWidget::~PhysicsGeometryWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsGeometryWidget::reinitialize(const std::shared_ptr<PhysicsShapePrefab>& prefab)
{
    clearLayout(m_mainLayout);
    delete m_mainLayout;
    m_prefab = prefab;
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsGeometryWidget::initializeWidgets()
{
    auto fab = prefab();
    if (!fab) return;
    std::shared_ptr<PhysicsGeometry> geometry = fab->geometry();

    // Create new geometry type widget
    PhysicsGeometry::GeometryType geometryType = geometry->getType();
    m_typeWidget = new QComboBox();
    m_typeWidget->addItem(SAIcon("cube"), tr("Cube"));
    m_typeWidget->addItem(SAIcon("globe"), tr("Sphere"));
    m_typeWidget->addItem(SAIcon("border-all"), tr("Plane"));
    m_typeWidget->setCurrentIndex(int(geometryType));

    // Get values to pre-populate geometry widgets
    double hx = 1;
    double hy = 1;
    double hz = 1;
    double r = 1;
    switch (geometryType) {
    case PhysicsGeometry::kBox: {
        auto boxGeometry = std::static_pointer_cast<BoxGeometry>(prefab()->geometry());
        hx = boxGeometry->hx();
        hy = boxGeometry->hy();
        hz = boxGeometry->hz();
        break;
    }
    case PhysicsGeometry::kSphere: {
        auto sphereGeometry = std::static_pointer_cast<SphereGeometry>(prefab()->geometry());
        r = sphereGeometry->radius();
        break;
    }
    case PhysicsGeometry::kPlane: {
        break;
    }
    default:
        throw("Geometry type not recognized");
    }

    // Box Geometry
    // hx
    m_hxLabel = new QLabel("hx:");
    m_hxLineEdit = new QLineEdit(QString::number(hx));
    m_hxLineEdit->setMaximumWidth(50);
    m_hxLineEdit->setValidator(new QDoubleValidator(1e-8, 1e20, 10));
    m_hxLineEdit->setToolTip("Half-width in x direction");

    // hy
    m_hyLabel = new QLabel("hy:");
    m_hyLineEdit = new QLineEdit(QString::number(hy));
    m_hyLineEdit->setMaximumWidth(50);
    m_hyLineEdit->setValidator(new QDoubleValidator(1e-8, 1e20, 10));
    m_hyLineEdit->setToolTip("Half-width in y direction");


    // hz
    m_hzLabel = new QLabel("hz:");
    m_hzLineEdit = new QLineEdit(QString::number(hz));
    m_hzLineEdit->setMaximumWidth(50);
    m_hzLineEdit->setValidator(new QDoubleValidator(1e-8, 1e20, 10));
    m_hzLineEdit->setToolTip("Half-width in z direction");

    // Sphere Geometry
    // radius
    m_radLabel = new QLabel("radius:");
    m_radiusLineEdit = new QLineEdit(QString::number(r));
    m_radiusLineEdit->setMaximumWidth(50);
    m_radiusLineEdit->setValidator(new QDoubleValidator(1e-8, 1e20, 10));
    m_radiusLineEdit->setToolTip("Sphere radius");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsGeometryWidget::initializeConnections()
{
    connect(m_typeWidget, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        pauseSimulation();

        switchGeometry(index);

        resumeSimulation();
    }
    );

    // Box widget
    connect(m_hxLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        pauseSimulation();

        double width = text.toDouble();
        std::static_pointer_cast<BoxGeometry>(prefab()->geometry())->setHx(width);

        resumeSimulation();
    }
    );

    connect(m_hyLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        pauseSimulation();

        double width = text.toDouble();
        std::static_pointer_cast<BoxGeometry>(prefab()->geometry())->setHy(width);

        resumeSimulation();
    }
    );

    connect(m_hzLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        pauseSimulation();

        double width = text.toDouble();
        std::static_pointer_cast<BoxGeometry>(prefab()->geometry())->setHz(width);

        resumeSimulation();
    }
    );

    // Sphere widget
    connect(m_radiusLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        pauseSimulation();

        double width = text.toDouble();
        std::static_pointer_cast<SphereGeometry>(prefab()->geometry())->setRadius(width);

        resumeSimulation();
    }
    );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsGeometryWidget::layoutWidgets()
{
    // Configure main layout
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(new QLabel("Geometry Type"));
    m_mainLayout->addWidget(m_typeWidget);

    // Box widget
    m_boxWidget = new QWidget();
    QHBoxLayout* boxLayout = new QHBoxLayout();
    boxLayout->addWidget(m_hxLabel);
    boxLayout->addWidget(m_hxLineEdit);
    boxLayout->addSpacing(25);
    boxLayout->addWidget(m_hyLabel);
    boxLayout->addWidget(m_hyLineEdit);
    boxLayout->addSpacing(25);
    boxLayout->addWidget(m_hzLabel);
    boxLayout->addWidget(m_hzLineEdit);
    boxLayout->setSpacing(0);
    m_boxWidget->setLayout(boxLayout);

    // Sphere widget
    m_sphereWidget = new QWidget();
    QHBoxLayout* sphereLayout = new QHBoxLayout();
    sphereLayout->addWidget(m_radLabel);
    sphereLayout->addWidget(m_radiusLineEdit);
    sphereLayout->setSpacing(0);
    m_sphereWidget->setLayout(sphereLayout);

    // Stacked layout
    m_stackedLayout = new QStackedLayout();
    m_stackedLayout->addWidget(m_boxWidget);
    m_stackedLayout->addWidget(m_sphereWidget);
    m_stackedLayout->addWidget(new QWidget()); // For plane

    m_mainLayout->addLayout(m_stackedLayout);
    switchGeometry(prefab()->geometry()->getType());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsGeometryWidget::switchGeometry(int index)
{
    // Add widgets to main layout
    PhysicsGeometry::GeometryType type = PhysicsGeometry::GeometryType(index);
    //std::shared_ptr<PhysicsGeometry> prevGeometry = prefab()->geometry();
    //if (type == prevGeometry->getType()) return;

    switch (type) {
    case PhysicsGeometry::kBox: {
        auto boxGeometry = std::make_shared<BoxGeometry>(
            m_hxLineEdit->text().toDouble(),
            m_hyLineEdit->text().toDouble(),
            m_hzLineEdit->text().toDouble()
            );
        prefab()->setGeometry(boxGeometry);
        break; 
    }
    case PhysicsGeometry::kSphere: {
        auto sphereGeometry = std::make_shared<SphereGeometry>(
            m_radiusLineEdit->text().toDouble()
            );
        m_radiusLineEdit->setText(QString::number(sphereGeometry->radius()));
        prefab()->setGeometry(sphereGeometry);
        break;
    }
    case PhysicsGeometry::kPlane:{
        if (m_body) {
            m_body->setRigidType(RigidBody::kStatic);
            m_body->reinitialize();
        }
        prefab()->setGeometry(std::make_shared<PlaneGeometry>());
        break;
    }
    default:
        throw("Geometry type not recognized");
    }

    m_stackedLayout->setCurrentIndex(index);
    emit switchedGeometry(prefab()->getName());
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PhysicsShapeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapeWidget::PhysicsShapeWidget(CoreEngine* core, 
    const std::shared_ptr<PhysicsShapePrefab>& prefab, 
    QWidget* parent,
    RigidBody* body) :
    ParameterWidget(core, parent),
    m_currentPrefab(prefab),
    m_body(body)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapeWidget::~PhysicsShapeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::showContextMenu(const QPoint& point)
{
    // for most widgets
    QPoint globalPos = mapToGlobal(point);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    //myMenu.addAction(m_deletePrefabAction);
    myMenu.exec(globalPos);

    //QAction* selectedItem = myMenu.exec(globalPos);
    //if (selectedItem)
    //{
    //    // something was chosen, do stuff
    //}
    //else
    //{
    //    // nothing was chosen
    //}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::initializeWidgets()
{
    m_prefabs = new QComboBox();
    repopulatePrefabs(false);
    m_prefabs->setEditable(true);
    //m_prefabs->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(m_prefabs, SIGNAL(customContextMenuRequested(const QPoint&)),
    //    this, SLOT(showContextMenu(const QPoint&)));
    //m_deletePrefabAction = new QAction(tr("Delete Prefab"), this);
    //connect(m_addPhysicsSceneComponent,
    //    &QAction::triggered,
    //    m_engine->actionManager(),
    //    [this] {m_engine->actionManager()->performAction(
    //        new AddPhysicsSceneCommand(m_engine, currentScene(), "Instantiate Scene Physics"));
    //});


    m_addPrefab = new QPushButton();
    m_addPrefab->setIcon(SAIcon(QStringLiteral("plus")));
    m_addPrefab->setToolTip("Add a new shape prefab");
    m_addPrefab->setMaximumWidth(35);

    m_removePrefab = new QPushButton();
    m_removePrefab->setIcon(SAIcon(QStringLiteral("minus")));
    m_removePrefab->setToolTip("Remove the selected shape prefab");
    m_removePrefab->setMaximumWidth(35);

    m_geometryWidget = new PhysicsGeometryWidget(m_engine, currentPrefab(), nullptr, m_body);

    m_materialWidget = new QComboBox();
    repopulateMaterials();

    if (currentPrefab()->getName() == PhysicsManager::DefaultShapeKey()) {
        m_geometryWidget->setDisabled(true);
        m_materialWidget->setDisabled(true);
    }
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::initializeConnections()
{
    // Prefabs
    connect(m_prefabs,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        pauseSimulation();

        // Set widget's current prefab
        const QString& prefabName = m_prefabs->itemText(index);
        const auto& prefab = PhysicsManager::shapes().at(prefabName);

        setCurrentPrefab(prefab);

        resumeSimulation();

    }
    );

    connect(m_prefabs->lineEdit(),
        &QLineEdit::editingFinished, // Important that this is not textChanged
        [this]() {
        pauseSimulation();

        const QString& text = m_prefabs->lineEdit()->text();
        if (currentPrefab()->getName() == "defaultShape") {
            logInfo("Ignoring modification of default shape");
        }
        else {
            // Remove and re-add to static map
            if (text != currentPrefab()->getName() && !Map::HasKey(PhysicsManager::shapes(), text)) {
                PhysicsManager::RenameShape(currentPrefab(), text);
                repopulatePrefabs();
            }
        }        

        resumeSimulation();
    }
    );

    connect(m_addPrefab, &QAbstractButton::clicked, this, 
        [this]() {
        pauseSimulation();

        QJsonObject json = PhysicsManager::DefaultShape()->asJson().toObject();
        json["name"] = Uuid::UniqueName();
        PhysicsShapePrefab::create(json);
        m_prefabs->addItem(SAIcon("cubes"), json["name"].toString());

        resumeSimulation();
    });

    connect(m_removePrefab, &QAbstractButton::clicked, this, 
        [this]() {
        pauseSimulation();

        if(currentPrefab()->getName() != PhysicsManager::DefaultShapeKey())
            // Remove prefab and all references to it
            PhysicsManager::RemoveShape(currentPrefab());

        setCurrentPrefab(PhysicsManager::DefaultShape());
        repopulatePrefabs();

        resumeSimulation();
    });

    // Material
    //connect(m_materialWidget,
    //    QOverload<int>::of(&QComboBox::currentIndexChanged),
    //    [this](int index) {
    //    pauseSimulation();

    //    resumeSimulation();
    //}
    //);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::layoutWidgets()
{
    // Configure main layout
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    // Prefabs layout
    QHBoxLayout* prefabsLayout = new QHBoxLayout();
    prefabsLayout->addWidget(m_prefabs);
    prefabsLayout->addSpacing(25);
    prefabsLayout->addWidget(m_addPrefab);
    prefabsLayout->addSpacing(5);
    prefabsLayout->addWidget(m_removePrefab);

    // Create new widgets
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(new QLabel("Shape Prefabs"));
    layout->addLayout(prefabsLayout);
    layout->addSpacing(10);
    layout->addWidget(m_geometryWidget);
    layout->addWidget(new QLabel("Materials:"));
    layout->addWidget(m_materialWidget);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::repopulatePrefabs(bool block)
{
    // Don't want to call signal when setting index programmatically
    if (block) {
        m_prefabs->blockSignals(true);
    }

    int count = 0;
    int index = 0;
    m_prefabs->clear();
    for (const std::pair<QString, std::shared_ptr<PhysicsShapePrefab>>& prefabPair : PhysicsManager::shapes()) {
        m_prefabs->addItem(SAIcon("cubes"), prefabPair.second->getName());
        if (prefabPair.second->getUuid() == currentPrefab()->getUuid())
            index = count;
        count++;
    }

    m_prefabs->setCurrentIndex(index);

    // Unblock signals
    if (block) {
        m_prefabs->blockSignals(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::repopulateMaterials()
{
    m_materialWidget->clear();
    for (const auto& material : PhysicsManager::materials()) {
        m_materialWidget->addItem(SAIcon("chess-board"), material.second->getName());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsShapeWidget::setCurrentPrefab(const std::shared_ptr<PhysicsShapePrefab>& prefab)
{
    m_currentPrefab = prefab;

    // Set prefab in the widget's rigid body
    m_body->shape(0).setPrefab(*currentPrefab());

    // Reinitialize geometry and material widgets
    m_geometryWidget->reinitialize(currentPrefab());
    repopulateMaterials();

    bool disabled = false;
    if (prefab->getName() == PhysicsManager::DefaultShapeKey()) {
        disabled = true;
    }
    m_geometryWidget->setDisabled(disabled);
    m_materialWidget->setDisabled(disabled);
    m_removePrefab->setDisabled(disabled);
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
#include "geppetto/qt/widgets/components/GPhysicsWidgets.h"

#include "fortress/types/GLoadable.h"
#include "fortress/json/GJson.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

#include "ripple/network/gateway/GMessageGateway.h"


namespace rev {

///< @todo Pull this out
static constexpr char* s_defaultPhysicsShapeName = "defaultShape";


PhysicsGeometryWidget::PhysicsGeometryWidget(WidgetManager* wm, const json& prefabJson, const json& bodyJson, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_prefabJson(prefabJson),
    m_bodyJson(bodyJson)
{
    initialize();
}

PhysicsGeometryWidget::~PhysicsGeometryWidget()
{
}

void PhysicsGeometryWidget::reinitialize(const json& prefabJson)
{
    clearLayout(m_mainLayout);
    delete m_mainLayout;
    m_prefabJson = prefabJson;
    initialize();
}

void PhysicsGeometryWidget::initializeWidgets()
{
    if (m_prefabJson.empty()) {
        return;
    }
    const json& geometry = m_prefabJson["geometry"];

    // Create new geometry type widget
    GPhysicsGeometryType geometryType = GPhysicsGeometryType(geometry["type"].get<Int32_t>());
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
    switch ((EPhysicsGeometryType)geometryType) {
    case EPhysicsGeometryType::eBox: {
        hx = geometry["hx"];
        hy = geometry["hy"];
        hz = geometry["hz"];
        break;
    }
    case EPhysicsGeometryType::eSphere: {
        r = geometry["radius"];
        break;
    }
    case EPhysicsGeometryType::ePlane: {
        break;
    }
    default:
        assert(false && "Geometry type not recognized");
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

void PhysicsGeometryWidget::initializeConnections()
{
    connect(m_typeWidget, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        updatePhysicsShapeGeometry(GPhysicsGeometryType(index));
    }
    );

    // Box widget
    connect(m_hxLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        updatePhysicsShapeGeometry((GPhysicsGeometryType)EPhysicsGeometryType::eBox);
    }
    );

    connect(m_hyLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        updatePhysicsShapeGeometry((GPhysicsGeometryType)EPhysicsGeometryType::eBox);
    }
    );

    connect(m_hzLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        updatePhysicsShapeGeometry((GPhysicsGeometryType)EPhysicsGeometryType::eBox);
    }
    );

    // Sphere widget
    connect(m_radiusLineEdit, &QLineEdit::textEdited,
        [this](const QString& text) {
        updatePhysicsShapeGeometry((GPhysicsGeometryType)EPhysicsGeometryType::eSphere);
    }
    );
}

void PhysicsGeometryWidget::layoutWidgets()
{
    // Configure main layout
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(new QLabel("Geometry Type"));
    m_mainLayout->addWidget(m_typeWidget);

    // Box widget
    m_boxWidget = new QWidget(this);
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
    m_sphereWidget = new QWidget(this);
    QHBoxLayout* sphereLayout = new QHBoxLayout();
    sphereLayout->addWidget(m_radLabel);
    sphereLayout->addWidget(m_radiusLineEdit);
    sphereLayout->setSpacing(0);
    m_sphereWidget->setLayout(sphereLayout);

    // Stacked layout
    m_stackedLayout = new QStackedLayout();
    m_stackedLayout->addWidget(m_boxWidget);
    m_stackedLayout->addWidget(m_sphereWidget);
    m_stackedLayout->addWidget(new QWidget(this)); // For plane

    m_mainLayout->addLayout(m_stackedLayout);
    if (!m_prefabJson.empty()) {
        updatePhysicsShapeGeometry(GPhysicsGeometryType(m_prefabJson["geometry"]["type"].get<Int32_t>()));
    }
}

void PhysicsGeometryWidget::updatePhysicsShapeGeometry(GPhysicsGeometryType type)
{
    // Switch the geometry of the prefab to a different type
    json& geometryJson = m_prefabJson["geometry"];
    geometryJson["type"] = Int32_t(type);

    switch ((EPhysicsGeometryType)type) {
    case EPhysicsGeometryType::eBox: {
        geometryJson["hx"] = m_hxLineEdit->text().toDouble();
        geometryJson["hy"] = m_hyLineEdit->text().toDouble();
        geometryJson["hz"] = m_hzLineEdit->text().toDouble();
        break; 
    }
    case EPhysicsGeometryType::eSphere: {
        geometryJson["radius"] = m_radiusLineEdit->text().toDouble();
        break;
    }
    case EPhysicsGeometryType::ePlane:{
        if (!m_bodyJson.empty()) {
            m_bodyJson["rigidType"] = Int32_t(ERigidBodyType::eStatic);
        }
        break;
    }
    default:
        assert(false && "Geometry type not recognized");
    }

    // Send request to main application to update physics shape
    sendUpdatePrefabMessage();

    // Send request to main application to update rigid body component
    sendUpdateRigidBodyComponentMessage();

    m_stackedLayout->setCurrentIndex(Int32_t(type));
    emit switchedGeometry(m_prefabJson["name"].get_ref<const std::string&>().c_str());
}

void PhysicsGeometryWidget::sendUpdatePrefabMessage()
{
    PhysicsShapeWidget* parent = dynamic_cast<PhysicsShapeWidget*>(parentWidget());
    if (parent) {
        parent->updatePrefabMessage().setJsonBytes(GJson::ToBytes(m_prefabJson));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(parent->updatePrefabMessage());
    }
}

void PhysicsGeometryWidget::sendUpdateRigidBodyComponentMessage()
{
    PhysicsShapeWidget* parent = dynamic_cast<PhysicsShapeWidget*>(parentWidget());
    if (parent) {
        parent->updateRigidBodyComponentMessage().setBodyJsonBytes(GJson::ToBytes(m_bodyJson));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(parent->updateRigidBodyComponentMessage());
    }
}




PhysicsShapeWidget::PhysicsShapeWidget(WidgetManager* wm, const json& prefabJson, const json& rigidBodyJson, const Uuid& componentId, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_currentPrefabJson(prefabJson),
    m_bodyJson(rigidBodyJson)
{
    m_updatePrefabMessage.setJsonBytes(GJson::ToBytes(prefabJson));
    m_updateRigidBodyComponentMessage.setBodyJsonBytes(GJson::ToBytes(rigidBodyJson));
    m_updateRigidBodyComponentMessage.setComponentId(componentId);
    m_updateRigidBodyComponentMessage.setSceneObjectId(wm->sceneTreeWidget()->getCurrentSceneObjectId());
    initialize();
}

PhysicsShapeWidget::~PhysicsShapeWidget()
{
}

void PhysicsShapeWidget::showContextMenu(const QPoint& point)
{
    // for most widgets
    QPoint globalPos = mapToGlobal(point);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.exec(globalPos);

}

void PhysicsShapeWidget::initializeWidgets()
{
    m_prefabs = new QComboBox();
    repopulatePrefabs(false);
    m_prefabs->setEditable(true);

    m_addPrefab = new QPushButton();
    m_addPrefab->setIcon(SAIcon(QStringLiteral("plus")));
    m_addPrefab->setToolTip("Add a new shape prefab");
    m_addPrefab->setMaximumWidth(35);

    m_removePrefab = new QPushButton();
    m_removePrefab->setIcon(SAIcon(QStringLiteral("minus")));
    m_removePrefab->setToolTip("Remove the selected shape prefab");
    m_removePrefab->setMaximumWidth(35);

    m_geometryWidget = new PhysicsGeometryWidget(m_widgetManager, m_currentPrefabJson, m_bodyJson, this);

    m_materialWidget = new QComboBox();
    repopulateMaterials();

    // Check if shape is default
    if (m_currentPrefabJson["name"].get_ref<const std::string&>() == s_defaultPhysicsShapeName) {
        m_geometryWidget->setDisabled(true);
        m_materialWidget->setDisabled(true);
    }
    
}

void PhysicsShapeWidget::initializeConnections()
{
    // Prefabs
    connect(m_prefabs,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {

        // Set widget's current prefab
        GString prefabName = m_prefabs->itemText(index).toStdString().c_str();
        setCurrentPrefab(prefabName);

    }
    );

    connect(m_prefabs->lineEdit(),
        &QLineEdit::editingFinished, // Important that this is not textChanged
        [this]() {
        GString text = m_prefabs->lineEdit()->text().toStdString();
        if (m_currentPrefabJson["name"] == s_defaultPhysicsShapeName) {
            std::cout << "Ignoring modification of default shape";
        }
    }
    );

    connect(m_addPrefab, &QAbstractButton::clicked, this, 
        [this]() {

        createPhysicsShapePrefab();

    });

    connect(m_removePrefab, &QAbstractButton::clicked, this, 
        [this]() {

        removePhysicsShapePrefab();

    });

}

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

void PhysicsShapeWidget::repopulatePrefabs(bool block)
{
    // Don't want to call signal when setting index programmatically
    if (block) {
        m_prefabs->blockSignals(true);
    }

    int count = 0;
    int index = 0;
    m_prefabs->clear();

    const json& shapesJson = m_widgetManager->scenarioJson()["physicsManager"]["shapes"];
    for (const auto& shapePrefabPair : shapesJson.items()) {
        const std::string& shapePrefabName = shapePrefabPair.value()["name"].get_ref<const std::string&>();
        m_prefabs->addItem(SAIcon("cubes"), shapePrefabName.c_str());
        if (shapePrefabName == m_currentPrefabJson["name"].get_ref<const std::string&>()) {
            index = count;
        }
        count++;
    }

    m_prefabs->setCurrentIndex(index);

    // Unblock signals
    if (block) {
        m_prefabs->blockSignals(false);
    }
}

void PhysicsShapeWidget::repopulateMaterials()
{
    m_materialWidget->clear();

    const json& materialsJson = m_widgetManager->scenarioJson()["physicsManager"]["materials"];
    for (const auto& matPair : materialsJson.items()) {
        m_materialWidget->addItem(SAIcon("chess-board"), 
            matPair.value()["name"].get_ref<const std::string&>().c_str());
    }
}

void PhysicsShapeWidget::setCurrentPrefab(const GString& prefabName)
{
    m_currentPrefabJson = m_widgetManager->scenarioJson()["physicsManager"]["shapes"][prefabName.c_str()];

    // Set prefab in the widget's rigid body
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_updateRigidBodyComponentMessage);

    // Reinitialize geometry and material widgets
    m_geometryWidget->reinitialize(m_currentPrefabJson);
    repopulateMaterials();

    bool disabled = false;
    if (m_currentPrefabJson["name"].get_ref<const std::string&>() == s_defaultPhysicsShapeName) {
        disabled = true;
    }
    m_geometryWidget->setDisabled(disabled);
    m_materialWidget->setDisabled(disabled);
    m_removePrefab->setDisabled(disabled);
}

void PhysicsShapeWidget::createPhysicsShapePrefab()
{
    // Generate unique name
    GString name = Uuid::UniqueName();
    
    // Send message to create shape prefab
    m_prefabs->addItem(SAIcon("cubes"), name.c_str());
    m_createPrefabMessage.setName(name.c_str());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_createPrefabMessage);

}

void PhysicsShapeWidget::removePhysicsShapePrefab()
{
    if (m_currentPrefabJson["name"].get_ref<const std::string&>() == s_defaultPhysicsShapeName) {
        setCurrentPrefab(s_defaultPhysicsShapeName);
    }
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_deletePrefabMessage);
}


} // rev
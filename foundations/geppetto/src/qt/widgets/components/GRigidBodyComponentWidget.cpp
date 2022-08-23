#include "geppetto/qt/widgets/components/GRigidBodyComponentWidget.h"

#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/json/GJson.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/components/GPhysicsWidgets.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

#include "enums/GRigidBodyTypeEnum.h"

namespace rev {

RigidBodyWidget::RigidBodyWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget *parent) :
    ComponentWidget(wm, componentJson, sceneObjectId, parent) {
    initialize();
    m_updateRigidBodyComponentMessage.setBodyJsonBytes(GJson::ToBytes(componentJson["body"]));
    m_updateRigidBodyComponentMessage.setComponentId(componentJson["id"].get<Uuid>());
    m_updateRigidBodyComponentMessage.setSceneObjectId(sceneObjectId);
}

RigidBodyWidget::~RigidBodyWidget()
{
}

void RigidBodyWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Rigid Type
    // TODO: "link"
    m_rigidTypeWidget = new QComboBox;
    m_rigidTypeWidget->setMaximumWidth(3000);
    m_rigidTypeWidget->addItem(SAIcon("lock"), "Static");
    m_rigidTypeWidget->addItem(SAIcon("tablets"), "Dynamic");
    GRigidBodyType rigidType = GRigidBodyType(m_componentJson["body"]["rigidType"].get<Int32_t>());
    m_rigidTypeWidget->setCurrentIndex(int(rigidType));

    // Is kinematic
    bool isKinematic = m_componentJson["body"]["isKinematic"].get<bool>();
    m_isKinematic = new QCheckBox();
    m_isKinematic->setChecked(isKinematic);

    // Density
    m_density = new QLineEdit;
    m_density->setText(QString::number(m_componentJson["body"]["density"].get<Float32_t>()));
    m_density->setMaximumWidth(75);
    m_density->setValidator(new QDoubleValidator(1e-8, 1e20, 10));

    if (rigidType == ERigidBodyType::eStatic)
        m_density->setDisabled(true);

    // Shapes
    const GString physicsShapeName = m_componentJson["body"]["shapes"][0].get_ref<const std::string&>();
    const json& shapesJson = m_widgetManager->scenarioJson()["physicsManager"]["shapes"];
    const json& prefabShapeJson = shapesJson[physicsShapeName.c_str()];
    m_shapeWidget = new PhysicsShapeWidget(m_widgetManager,
        prefabShapeJson,
        m_componentJson["body"],
        m_componentJson["id"].get<Uuid>(),
        this);
}

void RigidBodyWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Rigid type widget
    connect(m_rigidTypeWidget,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {

        GRigidBodyType type = GRigidBodyType(index);

        if (type == ERigidBodyType::eStatic) {
            m_density->setDisabled(true);
        }
        else {
            m_density->setDisabled(false);
        }
        m_componentJson["body"]["rigidType"] = Int32_t(type);
        m_updateRigidBodyComponentMessage.setBodyJsonBytes(GJson::ToBytes(m_componentJson["body"]));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_updateRigidBodyComponentMessage);

    }
    );

    // Is kinematic
    // Rigid type widget
    connect(m_isKinematic, &QCheckBox::clicked,
        [this](bool checked) {
        m_componentJson["body"]["isKinematic"] = Int32_t(checked);
        m_updateRigidBodyComponentMessage.setBodyJsonBytes(GJson::ToBytes(m_componentJson["body"]));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_updateRigidBodyComponentMessage);
    }
    );

    // Density
    connect(m_density, &QLineEdit::textEdited,
        [=](const QString& text) { 
        double newDensity = text.toDouble();
        m_componentJson["body"]["density"] = newDensity;
        m_updateRigidBodyComponentMessage.setBodyJsonBytes(GJson::ToBytes(m_componentJson["body"]));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_updateRigidBodyComponentMessage);
    }
    );

    // Geometry
    connect(m_shapeWidget->geometryWidget(), 
        &PhysicsGeometryWidget::switchedGeometry,
        this, 
        [this](const QString& prefabName) {
        const GString physicsShapeName = m_componentJson["body"]["shapes"][0].get_ref<const std::string&>();
        const json& prefabShapeJson = m_widgetManager->scenarioJson()["physicsManager"]["shapes"][physicsShapeName];

        bool disabled = false;
        GPhysicsGeometryType gType = GPhysicsGeometryType(prefabShapeJson["geometry"]["type"].get<Int32_t>());
        if (gType == EPhysicsGeometryType::ePlane) {
            disabled = true;
        }
        m_density->setDisabled(disabled);
        m_isKinematic->setDisabled(disabled);
        m_rigidTypeWidget->setCurrentIndex(m_componentJson["body"]["rigidType"].get<Int32_t>());
        m_rigidTypeWidget->setDisabled(disabled);
    });
}

void RigidBodyWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    QHBoxLayout* typeLayout = new QHBoxLayout;
    typeLayout->addWidget(new QLabel("Type: "));
    typeLayout->addWidget(m_rigidTypeWidget);

    QHBoxLayout* nextLine = new QHBoxLayout;
    nextLine->addWidget(new QLabel("Density: "));
    nextLine->addWidget(m_density);
    nextLine->addSpacing(25);
    nextLine->addWidget(new QLabel("Kinematic: "));
    nextLine->addWidget(m_isKinematic);

    m_mainLayout->addLayout(typeLayout);
    m_mainLayout->addLayout(nextLine);
    m_mainLayout->addWidget(m_shapeWidget);
}


} // rev
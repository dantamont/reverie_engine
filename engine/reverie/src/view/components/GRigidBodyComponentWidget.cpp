#include "GRigidBodyComponentWidget.h"

#include "../components/GPhysicsWidgets.h"
#include "../../core/physics/GPhysicsShapePrefab.h"
#include "../../core/physics/GPhysicsShape.h"
#include "../../core/physics/GPhysicsGeometry.h"
#include "../../core/physics/GPhysicsManager.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GComponent.h"
#include "../../core/components/GScriptComponent.h"
#include <core/components/GCharControlComponent.h>
#include <core/components/GRigidBodyComponent.h>
#include "../../core/components/GShaderComponent.h"
#include "../../core/scripting/GPythonScript.h"
#include "../../core/components/GLightComponent.h"
#include "../../core/components/GCameraComponent.h"

#include "../../core/components/GShaderComponent.h"
#include "../../core/components/GTransformComponent.h"
#include "../style/GFontIcon.h"
#include "../../core/geometry/GEulerAngles.h"

#include "../../core/physics/GPhysicsActor.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RigidBodyWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyWidget::RigidBodyWidget(CoreEngine* core,
    Component* component,
    QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyWidget::~RigidBodyWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RigidBodyComponent* RigidBodyWidget::rigidBodyComponent() const {
    return static_cast<RigidBodyComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    // Rigid Type
    // TODO: "link"
    m_rigidTypeWidget = new QComboBox;
    m_rigidTypeWidget->setMaximumWidth(3000);
    m_rigidTypeWidget->addItem(SAIcon("lock"), "Static");
    m_rigidTypeWidget->addItem(SAIcon("tablets"), "Dynamic");
    RigidBody::RigidType rigidType = rigidBodyComponent()->body()->rigidType();
    m_rigidTypeWidget->setCurrentIndex(int(rigidType));

    // Is kinematic
    bool isKinematic = rigidBodyComponent()->body()->isKinematic();
    m_isKinematic = new QCheckBox();
    m_isKinematic->setChecked(isKinematic);

    // Density
    m_density = new QLineEdit;
    m_density->setText(QString::number(rigidBodyComponent()->body()->density()));
    m_density->setMaximumWidth(75);
    m_density->setValidator(new QDoubleValidator(1e-8, 1e20, 10));

    if (rigidType == RigidBody::kStatic)
        m_density->setDisabled(true);

    // Shapes
    m_shapeWidget = new PhysicsShapeWidget(m_engine,
        shapePrefab(), 
        nullptr, 
        rigidBodyComponent()->body().get());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RigidBodyWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Rigid type widget
    connect(m_rigidTypeWidget,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        pauseSimulation();

        RigidBody::RigidType type = RigidBody::RigidType(index);

        rigidBodyComponent()->body()->setRigidType(type);
        rigidBodyComponent()->body()->reinitialize();

        if (type == RigidBody::kStatic)
            m_density->setDisabled(true);
        else
            m_density->setDisabled(false);

        resumeSimulation();
    }
    );

    // Is kinematic
    // Rigid type widget
    connect(m_isKinematic, &QCheckBox::clicked,
        [this](bool checked) {
        rigidBodyComponent()->body()->setKinematic(checked, true);
    }
    );

    // Density
    connect(m_density, &QLineEdit::textEdited,
        [=](const QString& text) { 
        double newDensity = text.toDouble();
        rigidBodyComponent()->body()->setDensity(newDensity);
    }
    );

    // Geometry
    connect(m_shapeWidget->geometryWidget(), &PhysicsGeometryWidget::switchedGeometry,
        this, [this](const QString& prefabName) {
        PhysicsShapePrefab* prefab = PhysicsManager::Shape(prefabName);
        bool disabled = false;
        if (prefab->geometry()->getType() == PhysicsGeometry::kPlane) {
            disabled = true;
        }
        m_density->setDisabled(disabled);
        m_isKinematic->setDisabled(disabled);
        m_rigidTypeWidget->setCurrentIndex(int(rigidBodyComponent()->body()->rigidType()));
        m_rigidTypeWidget->setDisabled(disabled);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsShapePrefab* RigidBodyWidget::shapePrefab()
{
    std::vector<PhysicsShape*>& shapes = rigidBodyComponent()->body()->shapes();
    const GString& prefabName = shapes[0]->prefab().getName();
    PhysicsShapePrefab* prefab = PhysicsManager::Shape(prefabName);
    return prefab;
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
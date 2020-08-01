#include "GbComponentModels.h"

#include "../../core/GbCoreEngine.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/components/GbComponent.h"
#include "../../core/components/GbScriptComponent.h"

#include "../../view/components/GbComponentWidgets.h"
#include "../../view/components/GbCharControlComponentWidget.h"
#include "../../view/components/GbLightComponentWidget.h"
#include "../../view/components/GbRigidBodyComponentWidget.h"
#include "../../view/components/GbScriptComponentWidget.h"
#include "../../view/components/GbShaderComponentWidget.h"
#include "../../view/components/GbTransformComponentWidget.h"
#include "../../view/components/GbCameraComponentWidget.h"
#include "../../view/components/GbModelComponentWidget.h"
#include "../../view/components/GbCubeMapComponentWidget.h"
#include "../../view/components/GbCanvasComponentWidget.h"
#include "../../view/tree/GbComponentWidget.h"

#include "../../view/parameters/GbRenderLayerWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SceneSceneItem
ComponentItem::ComponentItem(Component* component) :
    TreeItem(component, (int)getComponentType(component)),
    m_sceneObject(component->sceneObject().get())
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentItem::ComponentItem(SceneObject * sceneObject) :
    TreeItem(nullptr, (int)kSceneObjectSettings),
    m_sceneObject(sceneObject)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentItem::~ComponentItem()
{
    //removeWidget();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    componentWidget()->m_engine->actionManager()->performAction(command);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

// #ifdef DEBUG_MODE
    // logInfo("Setting widget " + component()->getName());
// #endif

    // Get parent tree widget
    ComponentTreeWidget * parentWidget = static_cast<ComponentTreeWidget*>(treeWidget());

    // Set widget based on component type
    ComponentItem::ComponentType type = componentType();
    switch (type) {
    case kScript:
    {
        // Create widget
        m_widget = new ScriptWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kTransform:
    {
        // Create widget
        m_widget = new TransformComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kShader:
    {
        // Create widget
        m_widget = new ShaderComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kRigidBody:
    {
        // Create widget
        m_widget = new RigidBodyWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kLight:
    {
        // Create widget
        m_widget = new LightComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kCamera:
    {
        // Create widget
        m_widget = new CameraComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kModel:
    {
        // Create widget
        m_widget = new ModelComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kCubeMap:
    {
        // Create widget
        m_widget = new CubeMapComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kCanvas:
    {
        // Create widget
        m_widget = new CanvasComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kListener:
    case kPhysicsScene:
    case kCharacterController:
    case kBoneAnimation:
    {
        // Create widget
        m_widget = new GenericComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kSceneObjectSettings:
    {
        m_widget = new RenderLayerSelectWidget(parentWidget->m_engine, m_sceneObject->_renderLayers());
        break;
    }
    default:
#ifdef DEBUG_MODE
        logWarning("Error, type of component item is not implemented");
#endif
        QJsonDocument doc(component()->asJson().toObject());
        QString rep(doc.toJson(QJsonDocument::Indented));
        m_widget = new QLabel(rep);
        break;
    }

    if (m_widget) {
        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, 0, m_widget);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentTreeWidget * ComponentItem::componentWidget() const
{
    return static_cast<ComponentTreeWidget*>(treeWidget());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentItem::ComponentType ComponentItem::getComponentType(Component* component)
{
    ComponentType type = ComponentType((int)component->getComponentType() + 2000);
#ifdef DEBUG_MODE
    if (component->getComponentType() == Component::ComponentType::kNone) {
        throw("Error, component type not implemented in UI");
    }
#endif
    return type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ComponentItem::initializeItem()
{
    // Set column text
    //refreshText();

    // Set default size of item
    //setSizeHint(0, QSize(200, 400));

    // Set flags to be drag and drop enabled
    setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
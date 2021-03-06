#include "GComponentModels.h"

#include "../../core/GCoreEngine.h"
#include "../../model_control/commands/GActionManager.h"
#include "../../model_control/commands/commands/GSceneCommand.h"

#include "../../core/rendering/renderer/GMainRenderer.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/scene/GBlueprint.h"
#include "../../core/components/GComponent.h"
#include "../../core/components/GScriptComponent.h"

#include "../../view/components/GComponentWidgets.h"
#include "../../view/components/GCharControlComponentWidget.h"
#include "../../view/components/GLightComponentWidget.h"
#include "../../view/components/GRigidBodyComponentWidget.h"
#include "../../view/components/GScriptComponentWidget.h"
#include "../../view/components/GShaderComponentWidget.h"
#include "../../view/components/GTransformComponentWidget.h"
#include "../../view/components/GCameraComponentWidget.h"
#include "../../view/components/GModelComponentWidget.h"
#include "../../view/components/GCubeMapComponentWidget.h"
#include "../../view/components/GCanvasComponentWidget.h"
#include "../../view/components/GListenerComponentWidget.h"
#include "../../view/components/GAnimationComponentWidget.h"
#include "../../view/components/GAudioSourceComponentWidget.h"
#include "../../view/components/GAudioListenerComponentWidget.h"
#include "../../view/tree/GComponentWidget.h"

#include "../../view/parameters/GRenderLayerWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
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
    ComponentItem::ComponentItemType type = componentType();
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
    case kAudioSource:
    {
        // Create widget
        m_widget = new AudioComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kListener:
    {
        // Create widget
        m_widget = new ListenerWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kAudioListener:
    {
        // Create widget
        m_widget = new AudioListenerWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kBoneAnimation:
    {
        // Create widget
        m_widget = new AnimationComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kPhysicsScene:
    case kCharacterController:
    {
        // Create widget
        m_widget = new GenericComponentWidget(parentWidget->m_engine, component(), parentWidget);
        break;
    }
    case kSceneObjectSettings:
    {
        m_widget = new RenderLayerSelectWidget(parentWidget->m_engine, m_sceneObject->renderLayerIds());
        break;
    }
    default:
        // Used for blueprint widget
//#ifdef DEBUG_MODE
//        logWarning("Error, type of component item is not implemented");
//#endif
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
ComponentItem::ComponentItemType ComponentItem::getComponentType(Component* component)
{
    ComponentItemType type = ComponentItemType((int)component->getComponentType() + 2000);
#ifdef DEBUG_MODE
    if (component->getComponentType() == ComponentType::kNone) {
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
// ComponentBlueprintItem
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentBlueprintItem::ComponentBlueprintItem(Blueprint * component) :
    TreeItem(component, (int)ComponentItem::ComponentItemType::kBlueprint)
{
}

ComponentBlueprintItem::~ComponentBlueprintItem()
{
}

void ComponentBlueprintItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    componentWidget()->m_engine->actionManager()->performAction(command);
}

void ComponentBlueprintItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    ComponentTreeWidget * parentWidget = static_cast<ComponentTreeWidget*>(treeWidget());

    m_widget = new JsonWidget(componentWidget()->m_engine, m_object, parentWidget, true);

    if (m_widget) {
        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, 0, m_widget);
    }
}

void ComponentBlueprintItem::removeWidget(int column)
{
    Q_UNUSED(column);
    // Only ever one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

View::ComponentTreeWidget * ComponentBlueprintItem::componentWidget() const
{
    return static_cast<ComponentTreeWidget*>(treeWidget());
}

void ComponentBlueprintItem::initializeItem()
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
} // rev
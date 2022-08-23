#include "geppetto/qt/widgets/tree/GComponentTreeItems.h"

#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GCharControlComponentWidget.h"
#include "geppetto/qt/widgets/components/GLightComponentWidget.h"
#include "geppetto/qt/widgets/components/GRigidBodyComponentWidget.h"
#include "geppetto/qt/widgets/components/GScriptComponentWidget.h"
#include "geppetto/qt/widgets/components/GShaderComponentWidget.h"
#include "geppetto/qt/widgets/components/GTransformComponentWidget.h"
#include "geppetto/qt/widgets/components/GCameraComponentWidget.h"
#include "geppetto/qt/widgets/components/GModelComponentWidget.h"
#include "geppetto/qt/widgets/components/GCubeMapComponentWidget.h"
#include "geppetto/qt/widgets/components/GCanvasComponentWidget.h"
#include "geppetto/qt/widgets/components/GListenerComponentWidget.h"
#include "geppetto/qt/widgets/components/GAnimationComponentWidget.h"
#include "geppetto/qt/widgets/components/GAudioSourceComponentWidget.h"
#include "geppetto/qt/widgets/components/GAudioListenerComponentWidget.h"
#include "geppetto/qt/widgets/components/GComponentJsonWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"
#include "geppetto/qt/widgets/general/GJsonWidget.h"
#include "geppetto/qt/widgets/types/GRenderLayerWidgets.h"

namespace rev {


ComponentItem::ComponentItem(ComponentItem::ComponentItemType type, const json& j, Uint32_t id ) :
    TreeItem(id, (int)type, j)
{
}

ComponentItem::~ComponentItem()
{
    // Force deletion of the child widget
    if (m_componentWidget) {
        delete m_componentWidget;
    }
}


void ComponentItem::setWidget()
{
    // Throw error if the widget already exists
    assert(nullptr == m_widget && "Error, item already has a widget");

    // Get parent tree widget
    ComponentTreeWidget * parentWidget = static_cast<ComponentTreeWidget*>(treeWidget());

    // Set widget based on component type
    json jsonObject;
    ComponentItem::ComponentItemType type = componentType();
    Int32_t sceneObjectId = parentWidget->m_currentSceneObjectId;

    QWidget* myWidget{ nullptr };
    switch (type) {
    case kScript:
    {
        // Create widget
        myWidget = new ScriptWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kTransform:
    {
        // Create widget
        myWidget = new TransformComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kShader:
    {
        // Create widget
        myWidget = new ShaderComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kRigidBody:
    {
        // Create widget
        myWidget = new RigidBodyWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kLight:
    {
        // Create widget
        myWidget = new LightComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kCamera:
    {
        // Create widget
        myWidget = new CameraComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kModel:
    {
        // Create widget
        myWidget = new ModelComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kCubeMap:
    {
        // Create widget
        myWidget = new CubeMapComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kCanvas:
    {
        // Create widget
        myWidget = new CanvasComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kAudioSource:
    {
        // Create widget
        myWidget = new AudioComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kListener:
    {
        // Create widget
        myWidget = new ListenerWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kAudioListener:
    {
        // Create widget
        myWidget = new AudioListenerWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kBoneAnimation:
    {
        // Create widget
        myWidget = new AnimationComponentWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, sceneObjectId, parentWidget);
        break;
    }
    case kPhysicsScene:
        // Create widget
        myWidget = new ComponentJsonWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, GSceneComponentType(ESceneComponentType::ePhysicsScene), parentWidget);
        break;
    case kCharacterController:
    {
        // Create widget
        myWidget = new ComponentJsonWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, GSceneObjectComponentType(ESceneObjectComponentType::eCharacterController), sceneObjectId, parentWidget);
        break;
    }
    case kSceneObjectSettings:
    {
        myWidget = new RenderLayerSelectWidget(parentWidget->m_widgetManager, sceneObjectId, ERenderLayerWidgetMode::eSceneObject);
        break;
    }
    default:
        assert(false && "Unsupported widget type");
        break;
    }

    // Wrap widget so it is not deleted on item removal/deletion
    m_componentWidget = myWidget;
    m_widget = new TreeWidgetKeeperWrapper(myWidget);

    if (m_widget) {
        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, 0, m_widget);
    }
}

void ComponentItem::removeWidget(int column)
{
    Q_UNUSED(column);     // Only ever one column, so don't need to worry about indexing

    // Need to manually delete child since TreeWidgetKeeperWrapper will preserve it
    delete static_cast<TreeWidgetKeeperWrapper*>(m_widget)->child();

    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

ComponentTreeWidget * ComponentItem::componentWidget() const
{
    return static_cast<ComponentTreeWidget*>(treeWidget());
}

void ComponentItem::initializeItem()
{
    // Set column text
    //refreshText();

    // Set default size of item
    //setSizeHint(0, QSize(200, 400));

    // Set flags to be drag and drop enabled
    setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
}



// ComponentBlueprintItem


ComponentBlueprintItem::ComponentBlueprintItem(const json& j) :
    TreeItem(-1, (int)ComponentItem::ComponentItemType::kBlueprint, j)
{
}

ComponentBlueprintItem::~ComponentBlueprintItem()
{
}

void ComponentBlueprintItem::setWidget()
{
    // Throw error if the widget already exists
    assert(m_widget == nullptr && "Error, item already has a widget");

    // Get parent tree widget
    ComponentTreeWidget * parentWidget = static_cast<ComponentTreeWidget*>(treeWidget());

    m_widget = new JsonWidget(
        componentWidget()->m_widgetManager,
        m_data.m_data.m_json,
        json{ {"isBlueprintItem", true} }, // metadata
        parentWidget, 
        true); // Initialize the widget

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

ComponentTreeWidget * ComponentBlueprintItem::componentWidget() const
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


} // rev
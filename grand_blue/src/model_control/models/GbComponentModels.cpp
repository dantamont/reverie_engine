#include "GbComponentModels.h"

#include "../../core/GbCoreEngine.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/components/GbComponent.h"
#include "../../core/components/GbScriptComponent.h"

#include "../../view/parameters/GbComponentWidgets.h"
#include "../../view/tree/GbComponentWidget.h"

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
    QTreeWidgetItem((int)getComponentType(component)),
    m_component(component),
    m_widget(nullptr)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ComponentItem::~ComponentItem()
{
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

    // Get parent tree widget
    ComponentTreeWidget * parentWidget = static_cast<ComponentTreeWidget*>(treeWidget());

    // Set widget based on component type
    ComponentItem::ComponentType type = componentType();
    switch (type) {
    case kScript:
    {
        // Create widget
        m_widget = new ScriptWidget(parentWidget->m_engine, m_component, parentWidget);
        break;
    }
    case kRenderer:
    case kTransform:
    case kCamera:
    case kLight:
    case kModel:
    case kListener:
    case kRigidBody:
    case kPhysicsScene:
    case kCanvas:
    case kCharacterController:
    case kBoneAnimation:
    {
        // Create widget
        m_widget = new GenericComponentWidget(parentWidget->m_engine, m_component, parentWidget);
        break;
    }
    default:
#ifdef DEBUG_MODE
        logWarning("Error, type of component item is not implemented");
#endif
        QJsonDocument doc(m_component->asJson().toObject());
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
void ComponentItem::removeWidget()
{
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
    ComponentType type = ComponentType(component->getComponentType() + 2000);
#ifdef DEBUG_MODE
    if (component->getComponentType() == Component::kNone) {
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
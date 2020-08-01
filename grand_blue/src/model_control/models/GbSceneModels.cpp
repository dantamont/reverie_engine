#include "GbSceneModels.h"

#include "../../core/GbCoreEngine.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"
#include "../../core/containers/GbColor.h"

#include "../../core/scene/GbScenario.h"
#include "../../core/scene/GbScene.h"
#include "../../core/scene/GbSceneObject.h"

#include "../../view/parameters/GbParameterWidgets.h"
#include "../../view/tree/GbSceneTreeWidget.h"
#include "../../view/style/GbFontIcon.h"

#include "../../core/utils/GbMemoryManager.h"

//#include "../GbWidgetManager.h"
//#include "../../GbMainWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SceneSceneItem
SceneRelatedItem::SceneRelatedItem(std::shared_ptr<Object> object, SceneRelatedItem * parent, SceneType type):
    QTreeWidgetItem(parent, (int)type),
    m_object(object),
    m_widget(nullptr)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneRelatedItem::~SceneRelatedItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::reparent(SceneRelatedItem * newParent)
{
    // Assert type
    if (sceneType() != SceneRelatedItem::kSceneObject) {
        throw("Error, only scene objects may be reparented");
    }
    auto sceneObject = std::static_pointer_cast<SceneObject>(object());

    // Set encapsulated object's parent
    switch (newParent->sceneType()) {
    case SceneRelatedItem::kScene: {
        auto* action = new ReparentSceneObjectCommand(nullptr,
            sceneTreeWidget()->m_engine,
            sceneObject,
            std::static_pointer_cast<Scene>(newParent->object()));
        action->perform();
        break;
    }
    case SceneRelatedItem::kSceneObject: {
        auto* action = new ReparentSceneObjectCommand(std::static_pointer_cast<SceneObject>(newParent->object()),
            sceneTreeWidget()->m_engine,
            sceneObject);
        action->perform();
        break;
    }
    default:
        throw("Error, scenario cannot be reparented");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::performAction(UndoCommand * command)
{
    // Add command to action manager
    sceneTreeWidget()->m_engine->actionManager()->performAction(command);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::setWidget()
{
    if (m_widget) {
        throw("Error, item already has a widget");
    }

    SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());
    QString name;
    switch (sceneType()) {
    case kScenario:
        break;
    case kScene:
    case kSceneObject:
    {
        // Create widget
        name = object()->getName();
        m_widget = new QLineEdit(name);
        m_widget->setFocusPolicy(Qt::StrongFocus);
        m_widget->show();

        // Set signal for widget value change
        QObject::connect(static_cast<QLineEdit*>(m_widget),
            &QLineEdit::editingFinished,
            static_cast<QLineEdit*>(m_widget),
            [this]() {
            if (!m_widget) {
                return;
            }
            SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty())
                newName = object()->getName();
            performAction(new Gb::ChangeNameCommand(newName, parentWidget->m_engine, object()));
        
        }
        );

        // Set signal for widget out of focus
        QObject::connect(treeWidget(),
            &QTreeWidget::itemSelectionChanged,
            m_widget,
            [this]() {
            if (m_widget) {
                removeWidget();
            }
        }
        );

        break;
    }
    case kComponent:
    default:
        throw("Error, type of item is not implemented");
        break;
    }

    if (m_widget) {
        // Assign widget to item in tree widget
        parentWidget->setItemWidget(this, SceneTreeWidget::s_numColumns - 1, m_widget);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::removeWidget()
{
    treeWidget()->removeItemWidget(this, SceneTreeWidget::s_numColumns - 1);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SceneTreeWidget * SceneRelatedItem::sceneTreeWidget() const
{
    return static_cast<View::SceneTreeWidget*>(treeWidget());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::initializeItem()
{
    // Set column text
    refreshText();

    // Set flags
    if (sceneType() != kScenario) {
        setFlags(flags() | (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled));
    }

    if (sceneType() == kSceneObject) {
        // Is scene object, set background color if auto-generated
        if (std::shared_ptr<Object> object = m_object.lock()) {
            auto sceneObject = S_CAST<SceneObject>(object);
            if (sceneObject->isPythonGenerated()) {
                setBackground(0, QBrush(Color(205, 125, 146)));
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::refreshText()
{
    QString text;
    auto obj = object();
    switch (sceneType()) {
    case kScenario:
        setText(0, obj->className());
        break;
    case kScene:
        setIcon(0, SAIcon("box-open"));
        break;
    case kSceneObject:
        setIcon(0, SAIcon("box"));
        break;
    case kComponent:
    default:
        throw("Error, this item type is not implemented");
        break;
    }

    setText(1, obj->getName());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
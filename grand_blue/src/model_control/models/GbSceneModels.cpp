#include "GbSceneModels.h"

#include "../../core/GbCoreEngine.h"
#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"

#include "../../core/rendering/renderer/GbMainRenderer.h"

#include "../../core/scene/GbScenario.h"
#include "../../core/scene/GbScene.h"
#include "../../core/scene/GbSceneObject.h"

#include "../../view/parameters/GbParameterWidgets.h"
#include "../../view/tree/GbSceneTreeWidget.h"

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

        // Set signal for widget value change
        QObject::connect(static_cast<QLineEdit*>(m_widget),
            &QLineEdit::editingFinished,
            static_cast<QLineEdit*>(m_widget),
            [this]() {
            SceneTreeWidget * parentWidget = static_cast<SceneTreeWidget*>(treeWidget());
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty())
                newName = object()->getName();
            performAction(new Gb::ChangeNameCommand(newName, parentWidget->m_engine, object()));
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
        parentWidget->setItemWidget(this, SceneTreeWidget::NUM_COLUMNS - 1, m_widget);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::removeWidget()
{
    treeWidget()->removeItemWidget(this, SceneTreeWidget::NUM_COLUMNS - 1);
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
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SceneRelatedItem::refreshText()
{
    std::vector<QString> text = getText();
    for (size_t i = 0; i < SceneTreeWidget::NUM_COLUMNS; i++) {
        setText(i, text[i]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<QString> SceneRelatedItem::getText()
{
    std::vector<QString> text;
    auto obj = object();
    switch (sceneType()) {
    case kScenario:
    case kScene:
    case kSceneObject:
    {
        text.push_back(obj->className());
        text.push_back(obj->getName());
        break;
    }
    case kComponent:
    default:
        throw("Error, this item type is not implemented");
        break;
    }

    return text;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
#include "geppetto/qt/widgets/animation/GAnimationTreeWidget.h"

#include "fortress/json/GJson.h"
#include "fortress/containers/GSortingLayer.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/actions/commands/GSceneCommand.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/animation/GAnimationClipWidget.h"
#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"

#include "enums/GAnimationStateTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

AnimationItem::AnimationItem(Int32_t itemId, const json& animationItemJson, AnimationItemType type) :
    TreeItem(itemId, (int)type, animationItemJson)
{
    initializeItem();
}

AnimationItem::~AnimationItem()
{
}

void AnimationItem::setWidget()
{
    assert(!m_widget && "Error, item already has a widget");

    // Get parent tree widget
    AnimationTreeWidgetInterface* parentWidget = static_cast<AnimationTreeWidgetInterface*>(treeWidget());

    // Create widget
    switch (AnimationItemType(type())) {
    case AnimationItemType::kState:
    case AnimationItemType::kMotion: {
        QString name = QString(m_data.getRef<const std::string&>("name").c_str());
        QString txt = text(1);

        assert (txt == name && "Error, something has gone awry. Name mismatch");

        m_widget = new QLineEdit(name);
        m_widget->setFocusPolicy(Qt::StrongFocus);

        // Set signal for widget value change
        QObject::connect(static_cast<QLineEdit*>(m_widget),
            &QLineEdit::editingFinished,
            static_cast<QLineEdit*>(m_widget),
            [this]() {
            if (!m_widget) {
                return;
            }
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty()) {
                newName = m_data.getRef<const std::string&>("name").c_str();
            }

            // Set name of motion
            AnimationTreeWidgetInterface* parentWidget = static_cast<AnimationTreeWidgetInterface*>(treeWidget());
            Uint32_t soId = parentWidget->getSceneObjectId();
            m_modifyMotionMessage.setSceneObjectId(soId);
            m_modifyMotionMessage.setMotionName(m_data.getRef<const std::string&>("name").c_str());
            m_modifyMotionMessage.setNewMotionName(newName.toStdString().c_str());
            parentWidget->m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyMotionMessage);

            // Refresh text in the tree item
            updateText();
        }
        );

        // Set signal for widget out of focus
        QObject::connect(treeWidget(),
            &QTreeWidget::itemSelectionChanged,
            m_widget,
            [this]() {
            if (m_widget) {
                removeWidget(1);
                updateText();
            }
        }
        );
    }
        break;
    case AnimationItemType::kClip:
    {
        
        AnimationClipTreeWidget* parentWidget = static_cast<AnimationClipTreeWidget*>(treeWidget());
        m_widget = new AnimClipWidget(parentWidget->m_widgetManager, m_data.m_data.m_json, parentWidget->getStateId(), parentWidget->getStateMachineId(), parentWidget->m_nodeWidget);
    }
        break;
    default:
        assert(false && "Unimplemented");
        break;
    }

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 1, m_widget);
}

void AnimationItem::removeWidget(int column)
{
    // Only ever use one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, column);
    m_widget = nullptr;
}

AnimationTreeWidgetInterface* AnimationItem::animationTreeWidget() const
{
    return static_cast<AnimationTreeWidgetInterface*>(treeWidget());
}

void AnimationItem::initializeItem()
{
    TreeItem::initializeItem();
    updateText();
}

void AnimationItem::updateText()
{
    const std::string& name = m_data.getRef<const std::string&>("name").c_str();
    switch (itemType()) {
    case AnimationItemType::kClip:
        setIcon(0, SAIcon("running"));
        break;
    case AnimationItemType::kMotion:
        setIcon(0, SAIcon("hands-helping"));
        setText(1, name.c_str());
        break;
    case AnimationItemType::kState:
    {
        GAnimationStateType stateType = (GAnimationStateType)m_data.get<Int32_t>("stateType");
        switch((EAnimationStateType)stateType){
        case EAnimationStateType::eAnimation:
            setIcon(0, SAIcon("square")); // "sitemap" "network-wired" "project-diagram"
            break;
        case EAnimationStateType::eTransition:
            setIcon(0, SAIcon("long-arrow-alt-right"));
            break;
        case EAnimationStateType::eNone:
            setIcon(0, SAIcon("circle"));
            break;
        default:
            assert(false && "Error, unimplemented");
            break;
        }
        setText(1, name.c_str());
        break;
    }
    default:
        assert(false && "Unrecognized item type");
        break;
    }
}

void AnimationItem::performAction(UndoCommand * command)
{
    animationTreeWidget()->m_widgetManager->actionManager()->performAction(command);
}





// AnimationTreeWidget

AnimationTreeWidgetInterface::AnimationTreeWidgetInterface(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, const char* widgetName, QWidget* parent) :
    TreeWidget(wm, wm->actionManager(), widgetName, parent, 2),
    m_animationComponentJson(animationComponentJson),
    m_nodeWidget(nodeWidget),
    m_sceneObjectId(sceneObjectId)
{
    initializeWidget();
}


AnimationTreeWidgetInterface::~AnimationTreeWidgetInterface()
{
}

void AnimationTreeWidgetInterface::removeItem(AnimationItem * AnimationItem)
{
    delete AnimationItem;
}

void AnimationTreeWidgetInterface::initializeWidget()
{
    TreeWidget::initializeWidget();

    setMinimumSize(150, 350);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setAlternatingRowColors(true);
}

void AnimationTreeWidgetInterface::onItemClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemClicked(item, column);

    AnimationItem* animItem = static_cast<AnimationItem*>(item);
    Uuid objectID = animItem->data().get<Uuid>("id");

    emit selectedItem(objectID, treeMode());
}

Uuid AnimationTreeWidgetInterface::getStateMachineId() const
{
    return m_animationComponentJson["animationController"]["stateMachineId"].get<Uuid>();
}

void AnimationTreeWidgetInterface::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemDoubleClicked(item, column);
}

void AnimationTreeWidgetInterface::initializeItem(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
    //static_cast<AnimationItem*>(item)->setWidget();
}


/// Clips

AnimationClipTreeWidget::AnimationClipTreeWidget(WidgetManager* wm, json& animationComponentJson, json& stateJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent) :
    AnimationTreeWidgetInterface(wm, animationComponentJson, sceneObjectId, nodeWidget,"Animation Clip Widget", parent),
    m_stateJson(stateJson)
{
    initializeWidget();
    repopulate();
}

AnimationClipTreeWidget::~AnimationClipTreeWidget()
{
}

void AnimationClipTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    assert(!m_stateJson.empty() && "Error, no state found for the tree widget");

    // Repopulate items
    Uint32_t counter = 0;
    for (auto& it: m_stateJson.items()) {
        addItem(counter, it.value());
        counter++;
    }

    // Resize columns to fit contents
    resizeColumns();
}

void AnimationClipTreeWidget::addItem(Uint32_t itemIndex, json& itemJson)
{
    // Create resource item
    AnimationItem* layerItem = new AnimationItem(itemIndex, itemJson, AnimationItem::AnimationItemType::kClip);

    // Add resource item
    TreeWidget::addItem(layerItem);

    // Set the item widget
    layerItem->setWidget();
}

Uuid AnimationClipTreeWidget::getStateId() const {
    return m_stateJson["id"].get<Uuid>();
}


void AnimationClipTreeWidget::initializeWidget()
{
    AnimationTreeWidgetInterface::initializeWidget();
    setHeaderLabels({ QStringLiteral(""), QStringLiteral("Clips") });
    connect(m_nodeWidget, &AnimationNodeWidget::repopulated, this, &AnimationClipTreeWidget::repopulate);

    // Initialize actions
    addAction(kNoItemSelected,
        "Add Animation Clip",
        "Add a clip to the animation state",
        [this] {
            // Add an empty clip to the animation state
            assert(!m_stateJson.empty() && "Error, no state found");
            m_addClipMessage.setStateId(getStateId());
            m_addClipMessage.setStateMachineId(getStateMachineId());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addClipMessage);
            m_nodeWidget->setStale();
        });

    addAction(kItemSelected,
        "Remove Animation Clip",
        "Remove clip from the animation state",
        [this] {
            assert(!m_stateJson.empty() && "Error, no state found");
            Uuid clipId = selectedAnimationItem()->data().get<Uuid>("id");
            m_removeClipMessage.setStateId(getStateId());
            m_removeClipMessage.setStateMachineId(getStateMachineId());
            m_removeClipMessage.setClipId(clipId);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeClipMessage);
            m_nodeWidget->setStale();
        });
}

void AnimationClipTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    AnimationTreeWidgetInterface::onItemDoubleClicked(item, column);
}



/// Motions

AnimationMotionTreeWidget::AnimationMotionTreeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent) :
    AnimationTreeWidgetInterface(wm, animationComponentJson, sceneObjectId, nodeWidget, "Animation Motion Widget", parent)
{
    initializeWidget();
    repopulate();
}


AnimationMotionTreeWidget::~AnimationMotionTreeWidget()
{
}

void AnimationMotionTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Repopulate items
    Uint32_t counter = 0;
    json& motionJson = m_nodeWidget->motionJson();
    for (auto& motionPair : motionJson.items()) {
        addItem(counter, motionPair.value());
        counter++;
    }

    // Resize columns to fit contents
    resizeColumns();
}

void AnimationMotionTreeWidget::addItem(Uint32_t itemIndex, json& itemJson)
{
    // Create resource item
    AnimationItem* layerItem = new AnimationItem(itemIndex, itemJson, AnimationItem::AnimationItemType::kMotion);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

void AnimationMotionTreeWidget::initializeWidget()
{
    AnimationTreeWidgetInterface::initializeWidget();
    setHeaderLabels({ QStringLiteral(""), QStringLiteral("Motions") });
    connect(m_nodeWidget, &AnimationNodeWidget::repopulated, this, &AnimationMotionTreeWidget::repopulate);

    // Initialize actions
    addAction(kItemSelected,
        "Remove Motion",
        "Remove motion from the state machine",
        [this] {
            std::string motionName = selectedAnimationItem()->data().getRef<const std::string&>("name");
            m_removeMotionMessage.setMotionName(motionName.c_str());
            m_removeMotionMessage.setSceneObjectId(m_sceneObjectId);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeMotionMessage);
            m_nodeWidget->setStale();
        });

    /// @todo Figure out a non-extremely painful way to fix this up. Just pinging every now and then would probably be neat
    //connect(m_engine->animationManager(), &AnimationManager::addedMotion, this, [this]() {
    //    repopulate();
    //    });

    //connect(m_engine->animationManager(), &AnimationManager::removedMotion, this, [this]() {
    //    repopulate();
    //    });

}

void AnimationMotionTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    AnimationTreeWidgetInterface::onItemDoubleClicked(item, column);

    auto* animationItem = static_cast<AnimationItem*>(item);
    if (!animationItem->widget()) {
        animationItem->setWidget();
    }
}



/// States


AnimationStateTreeWidget::AnimationStateTreeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent) :
    AnimationTreeWidgetInterface(wm, animationComponentJson, sceneObjectId, nodeWidget, "Animation States Widget", parent)
{
    initializeWidget();
    repopulate();
}


AnimationStateTreeWidget::~AnimationStateTreeWidget()
{
}

void AnimationStateTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Repopulate items
    Uint32_t counter = 0;
    for (const auto& stateJson : m_nodeWidget->statesJson().items()) {
        addItem(counter, stateJson.value());
        counter++;
    }
    for (const auto& transitionJson : m_nodeWidget->transitionsJson().items()) {
        addItem(counter, transitionJson.value());
        counter++;
    }

    // Resize columns to fit contents
    resizeColumns();
}

void AnimationStateTreeWidget::addItem(Uint32_t itemIndex, json& itemJson)
{
    // Create resource item
    AnimationItem* layerItem = new AnimationItem(itemIndex, itemJson, AnimationItem::AnimationItemType::kState);

    // Add resource item
    TreeWidget::addItem(layerItem);
}

void AnimationStateTreeWidget::initializeWidget()
{
    AnimationTreeWidgetInterface::initializeWidget();
    setHeaderLabels({ QStringLiteral(""), QStringLiteral("States and Transitions") });
    connect(m_nodeWidget, &AnimationNodeWidget::repopulated, this, &AnimationStateTreeWidget::repopulate);

    // Set actions
    addAction(kNoItemSelected,
        "Add Animation State",
        "Add a new animation state to the animation controller",
        [this] {
            m_addStateMessage.setStateMachineId(getStateMachineId());
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addStateMessage);
            m_nodeWidget->setStale();
        });

    addAction(kItemSelected,
        "Remove",
        "Remove animation state or transition from the animation controller",
        [this] {
            Uuid stateId = selectedAnimationItem()->data().get<Uuid>("id");
            m_removeStateMessage.setStateMachineId(getStateMachineId());
            m_removeStateMessage.setStateId(stateId);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeStateMessage);
            m_nodeWidget->setStale();
        });

}

void AnimationStateTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    AnimationTreeWidgetInterface::onItemDoubleClicked(item, column);

    auto* animationItem = static_cast<AnimationItem*>(item);
    if (!animationItem->widget()) {
        animationItem->setWidget();
    }
}


// End namespaces
}
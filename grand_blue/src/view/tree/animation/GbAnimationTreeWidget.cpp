#include "GbAnimationTreeWidget.h"

#include "../../../core/loop/GbSimLoop.h"
#include "../../../core/readers/GbJsonReader.h"
#include "../../../core/events/GbEventManager.h"
#include "../../../core/containers/GbSortingLayer.h"
#include "../../../core/processes/GbProcessManager.h"

#include "../../model_control/commands/GbActionManager.h"
#include "../../model_control/commands/commands/GbSceneCommand.h"

#include "../../../core/animation/GbAnimationController.h"
#include "../../../core/animation/GbAnimStateMachine.h"
#include "../../../core/animation/GbAnimationState.h"
#include "../../../core/animation/GbAnimTransition.h"

#include "../../GbWidgetManager.h"
#include "../../../GbMainWindow.h"
#include "../../style/GbFontIcon.h"
#include "../../parameters/animation/GbAnimationClipWidget.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AnimationItem
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationItem::AnimationItem(Object * object, AnimationItemType type) :
    TreeItem(object, (int)type)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationItem::~AnimationItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) { throw("Error, item already has a widget"); }

    // Get parent tree widget
    AnimationTreeWidget * parentWidget = static_cast<AnimationTreeWidget*>(treeWidget());

    // Create widget
    switch (AnimationItemType(type())) {
    case AnimationItemType::kState:
    case AnimationItemType::kMotion: {
        QString name = QString(object()->getName().c_str());
        QString txt = text(1);
        if (txt != name) {
            throw("Error, something has gone awry");
        }
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
            //AnimationTreeWidget * parentWidget = animationTreeWidget();
            QString newName = static_cast<QLineEdit*>(m_widget)->text();
            if (newName.isEmpty()) {
                newName = object()->getName().c_str();
            }
            // TODO: Maybe make this undoable, but will probably never happen (11/3/2020)
            //performAction(new Gb::ChangeNameCommand(newName, parentWidget->m_engine, object()));
            // Set name in scene object and in item
            object()->setName(newName);

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
        m_widget = new AnimClipWidget(itemObject<AnimationClip>(), parentWidget->m_engine);
    }
        break;
    default:
        throw("Unimplemented");
        break;
    }

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 1, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationItem::removeWidget(int column)
{
    // Only ever use one column, so don't need to worry about indexing
    treeWidget()->removeItemWidget(this, column);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::AnimationTreeWidget * AnimationItem::animationTreeWidget() const
{
    return static_cast<View::AnimationTreeWidget*>(treeWidget());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationItem::initializeItem()
{
    TreeItem<Object>::initializeItem();
    updateText();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationItem::updateText()
{
    Object* obj = object();
    switch (itemType()) {
    case AnimationItemType::kClip:
        setIcon(0, SAIcon("running"));
        break;
    case AnimationItemType::kMotion:
        setIcon(0, SAIcon("hands-helping"));
        setText(1, obj->getName());
        break;
    case AnimationItemType::kState:
    {
        BaseAnimationState* state = itemObject<BaseAnimationState>();
        switch(state->stateType()){
        case AnimationStateType::kAnimation:
            setIcon(0, SAIcon("square")); // "sitemap" "network-wired" "project-diagram"
            break;
        case AnimationStateType::kTransition:
            setIcon(0, SAIcon("long-arrow-alt-right"));
            break;
        case AnimationStateType::kNull:
            setIcon(0, SAIcon("circle"));
            break;
        default:
            throw("Error");
            break;
        }
        setText(1, obj->getName());
        break;
    }
    default:
        throw("Unrecognized");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationItem::performAction(UndoCommand * command)
{
    animationTreeWidget()->m_engine->actionManager()->performAction(command);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AnimationTreeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationTreeWidget::AnimationTreeWidget(CoreEngine* engine, AnimationController* controller, AnimationTreeMode mode, QWidget* parent) :
    TreeWidget(engine, "Animation Tree Widget", parent, 2),
    m_controller(controller),
    m_state(nullptr),
    m_treeMode(mode)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationTreeWidget::AnimationTreeWidget(CoreEngine * engine, AnimationController * controller, AnimationState * state, QWidget * parent) :
    TreeWidget(engine, "Animation Clip Widget", parent, 2),
    m_controller(controller),
    m_state(state),
    m_treeMode(AnimationTreeMode::kClips)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationTreeWidget::~AnimationTreeWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::repopulate()
{
    // Clear the widget
    clear();

    // Repopulate items
    switch (m_treeMode) {
    case AnimationTreeMode::kStates:
        for (BaseAnimationState* state : m_controller->stateMachine()->states()) {
            addItem(state);
        }
        break;
    case AnimationTreeMode::kMotion:
        for (const Motion& motion : m_controller->motions()) {
            addItem(const_cast<Motion*>(&motion));
        }
        break;
    case AnimationTreeMode::kClips:
        if (!m_state) {
            throw("Error, no state found for the tree widget");
        }
        for (AnimationClip& clip : m_state->clips()) {
            addItem(&clip);
        }
        break;
    default:
        throw("Mode unrecognized");
    }

    // Resize columns to fit contents
    resizeColumns();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::addItem(Motion * motion)
{
    // Verify that this tree widget is meant to be used for animation clips
    assert(m_treeMode == AnimationTreeMode::kMotion);

    // Create resource item
    AnimationItem* layerItem = new View::AnimationItem(motion, AnimationItem::AnimationItemType::kMotion);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::addItem(AnimationClip * clip)
{
    // Verify that this tree widget is meant to be used for animation clips
    assert(m_treeMode == AnimationTreeMode::kClips);

    // Create resource item
    AnimationItem* layerItem = new View::AnimationItem(clip, AnimationItem::AnimationItemType::kClip);

    // Add resource item
    TreeWidget::addItem(layerItem);
    layerItem->setWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::addItem(BaseAnimationState * state)
{
    // Verify that this tree widget is meant to be used for animation states
    assert(m_treeMode == AnimationTreeMode::kStates);

    // Create resource item
    AnimationItem* layerItem = new View::AnimationItem(state, AnimationItem::AnimationItemType::kState);

    // Add resource item
    TreeWidget::addItem(layerItem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::removeItem(AnimationItem * AnimationItem)
{
    delete AnimationItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::removeItem(Object* itemObject)
{
    AnimationItem* item = static_cast<AnimationItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    //setColumnWidth(0, columnWidth(0) * 0.75);
    //size_t width = columnWidth(1);
    //setColumnWidth(1, width);

    setMinimumSize(150, 350);

    //initializeAsList();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setAlternatingRowColors(true);
    //enableDragAndDrop();

    // Initialize actions
    switch (m_treeMode) {
    case AnimationTreeMode::kStates:
        setHeaderLabels({ QStringLiteral(""), QStringLiteral("States and Transitions") });

        addAction(kNoItemSelected,
            "Add Animation State",
            "Add a new animation state to the animation controller",
            [this] {
            m_controller->addState(new AnimationState(Uuid().createUniqueName("state_"),
                m_controller->stateMachine()));
            repopulate();
        });
        //addAction(kNoItemSelected,
        //    "Add Transition",
        //    "Add a new transition to the animation controller",
        //    [this] {
        //    m_controller->addTransition(new AnimationTransition());
        //    repopulate();
        //});

        addAction(kItemSelected,
            "Remove",
            "Remove animation state or transition from the animation controller",
            [this] {
            BaseAnimationState* currentState = selectedItemObject<BaseAnimationState>();
            if (!currentState) {
                throw("Error, no selected state");
            }
            m_controller->stateMachine()->removeState(currentState);
            repopulate();
        });

        break;
    case AnimationTreeMode::kMotion:
        setHeaderLabels({ QStringLiteral(""), QStringLiteral("Motions") });

        addAction(kItemSelected,
            "Remove Motion",
            "Remove motion from the state machine",
            [this] {
            Motion* motion = selectedItemObject<Motion>();
            m_controller->removeMotion(motion);
            repopulate();
        });

        connect(m_engine->eventManager(), &EventManager::addedMotion, this, [this]() {
            repopulate();
        });

        connect(m_engine->eventManager(), &EventManager::removedMotion, this, [this]() {
            repopulate();
        });

        break;
    case AnimationTreeMode::kClips:
        setHeaderLabels({ QStringLiteral(""), QStringLiteral("Clips") });

        addAction(kNoItemSelected,
            "Add Animation Clip",
            "Add a clip to the animation state",
            [this] {
            // Add an empty clip to the animation state
            if (!m_state) {
                throw("Error, no state found");
            }
            m_state->addClip(nullptr);
            repopulate();
        });

        addAction(kItemSelected,
            "Remove Animation Clip",
            "Remove clip from the animation state",
            [this] {
            if (!m_state) {
                throw("Error, no state found");
            }
            AnimationClip* clip = selectedItemObject<AnimationClip>();
            m_state->removeClip(*clip);
            repopulate();
        });
        break;
    default:
        throw("unrecognized");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::onItemClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemClicked(item, column);

    AnimationItem* animItem = static_cast<AnimationItem*>(item);
    const Uuid& objectID = animItem->object()->getUuid();

    emit selectedItem(objectID, m_treeMode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemDoubleClicked(item, column);

    auto* animationItem = static_cast<AnimationItem*>(item);
    if (animationItem->m_widget) {
        //throw("Error, item should not have a widget");
        // Item was not deselected, and still has a widget
        return;
    }

    // Set widget
    switch (m_treeMode) {
    case AnimationTreeMode::kStates:
    case AnimationTreeMode::kMotion:
        animationItem->setWidget();
        break;
    case AnimationTreeMode::kClips:
        break;
    default:
        throw("unrecognized");
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationTreeWidget::initializeItem(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
    //static_cast<AnimationItem*>(item)->setWidget();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}
#include "GAnimationNodeWidget.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenu>

#include "../../../core/GCoreEngine.h"
#include "../../../core/loop/GSimLoop.h"
#include "../../../core/resource/GResourceCache.h"
#include "../../../core/rendering/models/GModel.h"
#include "../../../core/animation/GAnimTransition.h"

#include "../../../core/scene/GSceneObject.h"
#include "../../../core/components/GAnimationComponent.h"
#include "../../../core/components/GTransformComponent.h"
#include "../../style/GFontIcon.h"

#include "../GNodeViewEditor.h"
#include "../GNodeViewCanvas.h"
#include "../GNodeViewBlock.h"
#include "../GNodeViewScene.h"
#include "../GNodeViewPort.h"
#include "../GNodeViewConnection.h"

#include "../../timeline/GTimeline.h"
#include "../../timeline/GTimelineTrack.h"
#include "../../timeline/GTimelineView.h"
#include "../../timeline/GTimelineScene.h"

#include "../../tree/animation/GAnimationTreeWidget.h"
#include "../../parameters/animation/GAnimationMotionWidget.h"
#include "../../parameters/animation/GAnimationStateWidget.h"

namespace rev {
namespace View {
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// AnimationNodeWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationNodeWidget::AnimationNodeWidget(CoreEngine* core, BoneAnimationComponent* animComp, QWidget *parent) :
    ParameterWidget(core, parent),
    m_animationComponent(animComp),
    m_minBlockSpacing(50),
    m_detailWidget(nullptr)
{
    initialize();

    setWindowTitle(tr("Animation State Machine"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
AnimationNodeWidget::~AnimationNodeWidget()
{
    // Delete nodeview stuff
    delete m_view;
    delete m_scene;
    delete m_editor;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::onMouseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    switch (static_cast<qint32>(event->type()))
    {
    case QEvent::GraphicsSceneMousePress:
    {
        switch (static_cast<qint32>(mouseEvent->button()))
        {
        case Qt::LeftButton:
        {
            break;
        }

        case Qt::RightButton:
        {
            // Return if no item selected on right-click
            QGraphicsItem* item = m_editor->itemAt(mouseEvent->scenePos());
            if (!item)
                break;

            const QPoint menuPosition = mouseEvent->screenPos();

            if (item->type() == (int)NodeViewItemType::kConnection)
            {
                showConnectionMenu(menuPosition, static_cast<NodeViewConnection*>(item));
            }
            else if (item->type() == (int)NodeViewItemType::kBlock)
            {
                showBlockMenu(menuPosition, static_cast<NodeViewBlock*>(item));
            }

            break;
        }
        }
    }

    case QEvent::GraphicsSceneMouseMove:
    {
        break;
    }

    case QEvent::GraphicsSceneMouseRelease:
    {
        break;
    }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::onConnectedPorts(NodeViewConnection* connection)
{
    // Add conection in state machine when ports connected
    NodeViewPort * start = connection->startPort();
    NodeViewPort * end = connection->endPort();
    BaseAnimationState* startState = getState(start->block());
    BaseAnimationState* endState = getState(end->block());
    StateConnection* sConnection = m_animationComponent->animationController().stateMachine()->addConnection(startState, endState);

    // Set connection metadata
    connection->setData(0, sConnection->getUuid());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::onDisconnectedPorts(StateConnection * connection)
{
    m_animationComponent->animationController().stateMachine()->removeConnection(connection);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::initializeWidgets()
{
    ParameterWidget::initializeWidgets();

    m_motionTreeWidget = new AnimationTreeWidget(m_engine, &m_animationComponent->animationController(), AnimationTreeMode::kMotion, this);
    m_stateTreeWidget = new AnimationTreeWidget(m_engine, &m_animationComponent->animationController(), AnimationTreeMode::kStates, this);    
    
    // Create scene and view
    m_scene = new NodeViewScene();
    m_view = new NodeViewCanvas(m_scene, this);
    
    m_editor = new NodeViewEditor(1, this);
    m_editor->install(m_scene);

    // Initialize graphics scene
    initializeAnimationScene();

    // Initialize timeline widget
    initializeTimeline();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::initializeConnections()
{
    ParameterWidget::initializeConnections();
    //m_jsonWidget->setReloadJson(true);

    // Handle mouse events from NodeViewEditor
    connect(m_editor, &NodeViewEditor::mouseEventFiltered, this, &AnimationNodeWidget::onMouseEvent);

    // Handle port connections
    connect(m_editor, &NodeViewEditor::connectedPorts, this, &AnimationNodeWidget::onConnectedPorts);

    // Handle motion selection
    connect(m_motionTreeWidget, &AnimationTreeWidget::selectedItem, this,
        [this](const Uuid& uuid, AnimationTreeMode mode) {
        Q_UNUSED(mode);
        Motion* motion = m_animationComponent->animationController().getMotion(uuid);
        setMotionWidget(motion);
    });

    // Handle state selection
    connect(m_stateTreeWidget, &AnimationTreeWidget::selectedItem, this,
        [this](const Uuid& uuid, AnimationTreeMode mode) {
        Q_UNUSED(mode);
        BaseAnimationState* state = m_animationComponent->animationController().stateMachine()->getState(uuid);
        
        // If state is not found, then is a transition
        if (!state) {
            state = m_animationComponent->animationController().stateMachine()->getTransition(uuid);
        }

        setStateWidget(state);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    m_gridLayout = new QGridLayout();
    m_gridLayout->setMargin(0);

    //QSizePolicy spLeft(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //spLeft.setHorizontalStretch(1);
    //m_motionTreeWidget->setSizePolicy(spLeft);
    //m_stateTreeWidget->setSizePolicy(spLeft);
    m_gridLayout->addWidget(m_motionTreeWidget, 1, 0, 1, 1);
    m_gridLayout->addWidget(m_stateTreeWidget, 0, 0, 1, 1);

    //QSizePolicy spMiddle(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //spMiddle.setHorizontalStretch(3);
    //m_view->setSizePolicy(spMiddle);
    m_gridLayout->addWidget(m_view, 0, 1, 2, 1);
    //m_view->setMinimumWidth(Renderable::screenDimensions().width() * 0.66);
    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 5);
    m_gridLayout->setColumnStretch(2, 1);

    m_gridLayout->setAlignment(Qt::AlignCenter);

    layoutTimeline();

    m_mainLayout->addLayout(m_gridLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::initializeAnimationScene()
{
    // Clear the graphics scene
    m_scene->clear();

    // Add animation states
    QPointF pos(0, 0);
    bool moveHorizontal = true;
    AnimationStateMachine* stateMachine = m_animationComponent->animationController().stateMachine();
    for (BaseAnimationState* state : stateMachine->states()) {
        if (state->stateType() == AnimationStateType::kTransition) {
            continue;
        }

        // Iterate over each state to add a block, alternating spacing
        NodeViewBlock* block = addBlock(state->getUuid(), pos);
        qint32 bw = block->width();
        qint32 hm = block->horizontalMargin();
        qint32 bh = block->height();
        qint32 vm = block->verticalMargin();
        float spacing;
        if (moveHorizontal) {
            // Space state horizontally with respect to last one
            spacing = std::max(bw + hm, m_minBlockSpacing);
            pos.setX(pos.x() + spacing);
        }
        else {
            // Space state vertically with respect to last one
            spacing = std::max(bh + vm, m_minBlockSpacing);
            pos.setY(pos.y() + spacing);
        }
        moveHorizontal = !moveHorizontal;
    }

    // TODO: Add connections
    for (const StateConnection& connection : stateMachine->connections()) {
        addConnection(connection);
    }

    // TODO: Add motions
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationNodeWidget::getState(const Uuid & uuid) const
{
    return m_animationComponent->animationController().stateMachine()->getState(uuid);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<BaseAnimationState*>& AnimationNodeWidget::getStates() const
{
    return m_animationComponent->animationController().stateMachine()->states();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewBlock* AnimationNodeWidget::addBlock(const Uuid& uuid, const QPointF & position)
{
    // Create block
    NodeViewBlock* block = new NodeViewBlock();
    m_scene->addItem(block);

    // Set block name
    BaseAnimationState* state = getState(uuid);
    QString blockName = QString(state->getName().c_str());
    block->addPort(blockName, 0, (int)NodeViewPortLabelFlags::kIsName);

    // Set block type
    const char* typeName;
    switch (state->stateType()) {
    case AnimationStateType::kAnimation:
        typeName = "Animation State";
        break;
    case AnimationStateType::kTransition:
        typeName = "Transition";
        break;
    }
    block->addPort(typeName, 0, (int)NodeViewPortLabelFlags::kIsType);

    // Set block metadata
    block->setData(0, state->getUuid());

    // Set block position
    block->setPos(position);

    return block;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewPort * AnimationNodeWidget::addPort(const BaseAnimationState* state, bool isOutput)
{
    // Search for block that matches state
    NodeViewPort* port = nullptr;
    for (QGraphicsItem* item : m_scene->items()) {
        if (item->type() != (int)NodeViewItemType::kBlock) {
            // Skip if not a block
            continue;
        }

        NodeViewBlock* block = static_cast<NodeViewBlock*>(item);

        BaseAnimationState* blockState = getState(block);
        if (blockState->getUuid() == state->getUuid()) {
            port = block->addPort("", isOutput);
            break;
        }
    }

    if (!port) {
        throw("Error, port not found");
    }

    return port;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
NodeViewConnection * AnimationNodeWidget::addConnection(const StateConnection & connection)
{
    // Create connection
    NodeViewConnection* nvc = new NodeViewConnection();
    m_scene->addItem(nvc);

    // Create ports
    const BaseAnimationState* endState = connection.end(m_animationComponent->animationController().stateMachine());
    const BaseAnimationState* startState = connection.start(m_animationComponent->animationController().stateMachine());
    NodeViewPort* start = addPort(startState, true);
    NodeViewPort* end = addPort(endState, false);

    nvc->setStartPort(start);
    nvc->setEndPort(end);

    // Set connection metadata to machine index of state machine connection
    nvc->setData(0, connection.machineIndex());

    // Update the position based on ports
    nvc->updatePosition();

    return nvc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BaseAnimationState * AnimationNodeWidget::getState(NodeViewBlock * block)
{
    Uuid stateID = Uuid(block->data(0).toUuid());
    return m_animationComponent->animationController().stateMachine()->getState(stateID);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
StateConnection * AnimationNodeWidget::getConnection(NodeViewConnection * connection)
{
    Uuid connectionID = Uuid(connection->data(0).toUuid());
    return m_animationComponent->animationController().stateMachine()->getConnection(connectionID);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::showBlockMenu(const QPoint& point, NodeViewBlock* block)
{
    // Perform actions on block selection
    QMenu menu;
    if (block->inputPortsHoveredOver()) {
        QAction* inputAction = menu.addAction("Add Input Port");
        QAction* selection = menu.exec(point);
        if (selection == inputAction) {
            NodeViewPort* port = block->addPort("", false);
            Q_UNUSED(port);
        }
    }
    else if (block->outputPortsHoveredOver()) {
        QAction* outputAction = menu.addAction("Add Output Port");
        QAction* selection = menu.exec(point);
        if (selection == outputAction) {
            NodeViewPort* port = block->addPort("", true);
            Q_UNUSED(port);
        }
    }
    else {
        // Don't allow deletion of states, saves the trouble of having to implement adding them back
        //QAction* deleteAction = menu.addAction("Delete");
        QAction* addMotion = menu.addAction("Add Motion");
        QAction* selection = menu.exec(point);
        //if (selection == deleteAction)
        //{
        //    delete block;
        //}
        if (selection == addMotion) {
            BaseAnimationState* state = getState(block);
            m_animationComponent->animationController().addMotion(state);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::showConnectionMenu(const QPoint& point, NodeViewConnection* connection)
{
    QMenu menu;
    BaseAnimationState* startState = getState(connection->startPort()->block());
    BaseAnimationState* endState = getState(connection->endPort()->block());
    QAction* addTransitionAction;
    if (startState->stateType() == AnimationStateType::kAnimation &&
        endState->stateType() == AnimationStateType::kAnimation) {
        // Only add transitions when both states are of correct type
        addTransitionAction = menu.addAction("Add Transition");
    }

    QAction* splitAction = menu.addAction("Split");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete");
    QAction* selection = menu.exec(point);
    if (selection == deleteAction)
    {
        onDisconnectedPorts(getConnection(connection));
        delete connection;
    }
    else if (selection == addTransitionAction) {
        static std::atomic<int> count = 0;
        AnimationController& controller = m_animationComponent->animationController();
        //AnimationState* start = static_cast<AnimationState*>(startState);
        //AnimationState* end = static_cast<AnimationState*>(endState);
        StateConnection* conn = getConnection(connection);
        AnimationTransition* transition = new AnimationTransition(controller.stateMachine(), conn->machineIndex());
        transition->setName("transition" + GString::FromNumber((size_t)count));
        controller.addTransition(transition);
        count++;

        // Repopulate tree widget
        m_stateTreeWidget->repopulate();
    }
    else if (selection == splitAction)
    {
        NodeViewConnectionSplit* split = new NodeViewConnectionSplit(connection);
        m_scene->addItem(split);
        // GW-TODO: Temp: Need to calculate multiple points
        QPointF splitPosition = (connection->startPosition() + connection->endPosition()) / 2;
        split->setSplitPosition(splitPosition);
        split->updatePath();
        connection->splits().append(split);
        connection->updatePath();
    }
    menu.hide();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::setMotionWidget(Motion * motion)
{
    clearDetailWidget();

    m_detailWidget = new AnimMotionWidget(motion, &m_animationComponent->animationController(), this);

    layoutDetailWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::setStateWidget(BaseAnimationState * state)
{
    Q_UNUSED(state);

    clearDetailWidget();

    m_detailWidget = new AnimStateWidget(state, &m_animationComponent->animationController(), this);

    layoutDetailWidget();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::clearDetailWidget()
{
    if (m_detailWidget) {
        m_gridLayout->removeWidget(m_detailWidget);
        delete m_detailWidget;
        m_detailWidget = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::clearTimelineWidget()
{
    m_timeline->view()->scene()->clear();
    m_gridLayout->removeWidget(m_timeline->view());

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::layoutDetailWidget()
{
    //QSizePolicy spRight(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //spRight.setHorizontalStretch(1);
    //m_detailWidget->setSizePolicy(spRight);
    m_gridLayout->addWidget(m_detailWidget, 0, 2, 2, 1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::initializeTimeline()
{
    m_timeline = new Timeline(this);
    m_timeline->scene()->setTickIncrement(0.2f);
    m_timeline->setTitle("Animation Resources");

    std::shared_ptr<ResourceHandle> modelHandle = m_animationComponent->modelHandle();
    if (!modelHandle) {
        return;
    }
    
    // Iterate through animations to add 
    for (const std::shared_ptr<ResourceHandle>& child : modelHandle->children()) {
        // Skip resources that are not animations
        if (child->getResourceType() != ResourceType::kAnimation) {
            continue;
        }

        // Get animation
        Animation* animation = child->resourceAs<Animation>();
        if (!animation) {
            continue;
        }
        size_t rowIndex = m_timeline->addRow(child->getName().c_str());

        // Add animation frame times to timeline
        const std::vector<float>& times = animation->frameTimes();
        size_t numFrames = times.size();
        double ticksToSecs = 1.0 / animation->m_ticksPerSecond;
        for (size_t i = 0; i < numFrames; i++) {
            float timeInSec = times[i] * ticksToSecs;
            m_timeline->addMarker(rowIndex, timeInSec, false);
        }
        m_timeline->refreshTickLabels();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationNodeWidget::layoutTimeline()
{
    QSizePolicy spBottom(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    spBottom.setHorizontalStretch(1);
    spBottom.setVerticalStretch(0.5);
    m_timeline->view()->setSizePolicy(spBottom);
    m_gridLayout->addWidget(m_timeline->view(), 2, 0, 1, 3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
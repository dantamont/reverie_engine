#include "geppetto/qt/widgets/nodes/animation/GAnimationNodeWidget.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenu>

#include "geppetto/qt/layer/types/GQtConverter.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "geppetto/qt/widgets/GWidgetManager.h"

#include "geppetto/qt/widgets/nodes/GNodeViewEditor.h"
#include "geppetto/qt/widgets/nodes/GNodeViewCanvas.h"
#include "geppetto/qt/widgets/nodes/GNodeViewBlock.h"
#include "geppetto/qt/widgets/nodes/GNodeViewScene.h"
#include "geppetto/qt/widgets/nodes/GNodeViewPort.h"
#include "geppetto/qt/widgets/nodes/GNodeViewConnection.h"

#include "geppetto/qt/widgets/playback/timeline/GTimeline.h"
#include "geppetto/qt/widgets/playback/timeline/GTimelineTrack.h"
#include "geppetto/qt/widgets/playback/timeline/GTimelineView.h"
#include "geppetto/qt/widgets/playback/timeline/GTimelineScene.h"

#include "geppetto/qt/widgets/animation/GAnimationTreeWidget.h"
#include "geppetto/qt/widgets/animation/GAnimationMotionWidget.h"
#include "geppetto/qt/widgets/animation/GAnimationStateWidget.h"
#include "geppetto/qt/widgets/components/GAnimationComponentWidget.h"

#include "enums/GAnimationStateTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

AnimationNodeWidget::AnimationNodeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, QWidget *parent) :
    ParameterWidget(wm, parent),
    m_animationComponentJson(animationComponentJson),
    m_sceneObjectId(sceneObjectId),
    m_minBlockSpacing(50),
    m_detailWidget(nullptr)
{
    initialize();

    setWindowTitle(tr("Animation State Machine"));
}

AnimationNodeWidget::~AnimationNodeWidget()
{
    // Delete nodeview stuff
    delete m_view;
    delete m_scene;
    delete m_editor;
}

void AnimationNodeWidget::update()
{
    // Request an update this widget and all connected animation widgets if stale
    if (m_isStale) {
        requestAnimationData();
        m_isStale = false;
    }
}

void AnimationNodeWidget::update(const GAnimationComponentDataMessage& animationDataMessage)
{
    m_motions = GJson::FromBytes(animationDataMessage.getMotionsJsonBytes());
    m_states = GJson::FromBytes(animationDataMessage.getStatesJsonBytes());
    m_transitions = GJson::FromBytes(animationDataMessage.getTransitionsJsonBytes());
    m_animations = GJson::FromBytes(animationDataMessage.getAnimationsJsonBytes());
    m_connections = GJson::FromBytes(animationDataMessage.getConnectionsJsonBytes());

    // Initialize graphics scene
    initializeAnimationScene();

    // Initialize timeline widget
    initializeTimeline();

    // Repopulate associated animation widgets
    emit repopulated();
}

void AnimationNodeWidget::update(const GAnimationDataMessage& animationDataMessage)
{
    emit receivedAnimationData(animationDataMessage);
}

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

void AnimationNodeWidget::onConnectedPorts(NodeViewConnection* connection)
{
    // Add conection in state machine when ports connected
    NodeViewPort * start = connection->startPort();
    NodeViewPort * end = connection->endPort();
    Uuid startState = getStateId(start->block());
    Uuid endState = getStateId(end->block());

    Uuid connectionId;
    Uuid stateMachineId = m_animationComponentJson["animationController"]["stateMachineId"];
    m_addConnectionMessage.setConnectionId(connectionId);
    m_addConnectionMessage.setStateMachineId(stateMachineId);
    m_addConnectionMessage.setStartStateId(startState);
    m_addConnectionMessage.setEndStateId(endState);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addConnectionMessage);

    // Set connection metadata
    connection->setData(0, QConverter::ToQt(connectionId));
}

void AnimationNodeWidget::onDisconnectedPorts(const Uuid& connectionId)
{
    Uuid stateMachineId = m_animationComponentJson["animationController"]["stateMachineId"];
    m_removeConnectionMessage.setConnectionId(connectionId);
    m_removeConnectionMessage.setStateMachineId(stateMachineId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeConnectionMessage);
}

void AnimationNodeWidget::initializeWidgets()
{
    m_motionTreeWidget = new AnimationMotionTreeWidget(m_widgetManager, m_animationComponentJson, m_sceneObjectId, this, this);
    m_stateTreeWidget = new AnimationStateTreeWidget(m_widgetManager, m_animationComponentJson, m_sceneObjectId, this, this);
    
    // Create scene and view
    m_scene = new NodeViewScene();
    m_view = new NodeViewCanvas(m_scene, this);
    
    m_editor = new NodeViewEditor(1, this);
    m_editor->install(m_scene);

    // Request update animation information from the main engine
    requestAnimationData();
}

void AnimationNodeWidget::initializeConnections()
{
    // Handle mouse events from NodeViewEditor
    connect(m_editor, &NodeViewEditor::mouseEventFiltered, this, &AnimationNodeWidget::onMouseEvent);

    // Handle port connections
    connect(m_editor, &NodeViewEditor::connectedPorts, this, &AnimationNodeWidget::onConnectedPorts);

    // Handle motion selection
    connect(m_motionTreeWidget, &AnimationTreeWidgetInterface::selectedItem, this,
        [this](const Uuid& uuid, AnimationTreeMode mode) {
        Q_UNUSED(mode);
        json& motionJson = m_motions[uuid.asString().c_str()];
        setMotionWidget(motionJson);
    });

    // Handle state selection
    connect(m_stateTreeWidget, &AnimationTreeWidgetInterface::selectedItem, this,
        [this](const Uuid& uuid, AnimationTreeMode mode) {
            Q_UNUSED(mode);
            json stateJson;

            GString key = uuid.asString();
            if (m_states.contains(key.c_str())) {
                // State found!
                stateJson = m_states[key];
            }
            else {
                // If state is not found, then is a transition
                stateJson = m_transitions[key];
            }

            setStateWidget(stateJson);
        }
    );

    // Delete on close, and connect signal to clear from parent widget
    setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, &AnimationNodeWidget::destroyed, this, 
        [this]() {
            AnimationComponentWidget* parent = dynamic_cast<AnimationComponentWidget*>(parentWidget());
            assert(parent && "Parent not found");
            parent->clearNodeWidget();
        }    
    );
}

void AnimationNodeWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    m_gridLayout = new QGridLayout();
    m_gridLayout->setMargin(0);
    m_gridLayout->addWidget(m_motionTreeWidget, 1, 0, 1, 1);
    m_gridLayout->addWidget(m_stateTreeWidget, 0, 0, 1, 1);

    m_gridLayout->addWidget(m_view, 0, 1, 2, 1);
    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 5);
    m_gridLayout->setColumnStretch(2, 1);

    m_gridLayout->setAlignment(Qt::AlignCenter);

    layoutTimeline();

    m_mainLayout->addLayout(m_gridLayout);
}

void AnimationNodeWidget::initializeAnimationScene()
{
    // Clear the graphics scene
    m_scene->clear();

    // Add animation states
    QPointF pos(0, 0);
    bool moveHorizontal = true;
    for (const json& stateJson : m_states) {
        if (stateJson["stateType"].get<Int32_t>() == (size_t)EAnimationStateType::eTransition) {
            continue;
        }

        // Iterate over each state to add a block, alternating spacing
        NodeViewBlock* block = addBlock(stateJson["id"].get<Uuid>(), pos);
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

    // Add connections
    for (const json& connectionJson : m_connections) {
        addConnection(connectionJson);
    }

    // TODO: Add motions
}

NodeViewBlock* AnimationNodeWidget::addBlock(const Uuid& stateId, const QPointF & position)
{
    // Create block
    NodeViewBlock* block = new NodeViewBlock();
    m_scene->addItem(block);

    // Set block name
    const json& stateJson = m_states[stateId.asString().c_str()];
    QString blockName = QString(stateJson["name"].get_ref<const std::string&>().c_str());
    block->addPort(blockName, 0, (int)NodeViewPortLabelFlags::kIsName);

    // Set block type
    GAnimationStateType stateType = GAnimationStateType(stateJson["stateType"].get<Int32_t>());
    const char* typeName;
    switch ((EAnimationStateType)stateType) {
    case EAnimationStateType::eAnimation:
        typeName = "Animation State";
        break;
    case EAnimationStateType::eTransition:
        typeName = "Transition";
        break;
    }
    block->addPort(typeName, 0, (int)NodeViewPortLabelFlags::kIsType);

    // Set block metadata
    block->setData(0, QConverter::ToQt(stateId));

    // Set block position
    block->setPos(position);

    return block;
}

NodeViewPort * AnimationNodeWidget::addPort(const Uuid& stateId, bool isOutput)
{
    // Search for block that matches state
    NodeViewPort* port = nullptr;
    for (QGraphicsItem* item : m_scene->items()) {
        if (item->type() != (int)NodeViewItemType::kBlock) {
            // Skip if not a block
            continue;
        }

        NodeViewBlock* block = static_cast<NodeViewBlock*>(item);

        Uuid blockStateId = getStateId(block);
        if (blockStateId == stateId) {
            port = block->addPort("", isOutput);
            break;
        }
    }

    assert(port && "Error, port not found");

    return port;
}

NodeViewConnection * AnimationNodeWidget::addConnection(const json& connectionJson)
{
    // Create connection
    NodeViewConnection* nvc = new NodeViewConnection();
    m_scene->addItem(nvc);

    // Get json for start and end states from indices
    Int32_t startIndexInStateMachine = connectionJson["start"].get<Int32_t>();
    Int32_t endIndexInStateMachine = connectionJson["end"].get<Int32_t>();
    json startState;
    json endState;
    Uint32_t counter = 0;
    for (const auto& item : m_states.items())
    {
        if (counter == startIndexInStateMachine) {
            startState = item.value();
        }
        if (counter == endIndexInStateMachine) {
            endState = item.value();
        }
        counter++;
    }

    assert(!startState.empty() && "Start state not found");
    assert(!endState.empty() && "End state not found");

    // Create ports
    NodeViewPort* start = addPort(startState["id"].get<Uuid>(), true);
    NodeViewPort* end = addPort(endState["id"].get<Uuid>(), false);

    nvc->setStartPort(start);
    nvc->setEndPort(end);

    // Set connection metadata to machine index of state machine connection
    nvc->setData(0, connectionJson["machineIndex"].get<Int32_t>());

    // Update the position based on ports
    nvc->updatePosition();

    return nvc;
}

Uuid AnimationNodeWidget::getStateId(NodeViewBlock * block)
{
    return QConverter::FromQt(block->data(0).toUuid());
}

const json& AnimationNodeWidget::getStateJson(NodeViewBlock* block)
{
    return m_states[getStateId(block).asString().c_str()];
}

Uuid AnimationNodeWidget::getConnectionId(NodeViewConnection * connection)
{
    return QConverter::FromQt(connection->data(0).toUuid());
}

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
        QAction* addMotion = menu.addAction("Add Motion");
        QAction* selection = menu.exec(point);

        if (selection == addMotion) {
            Uuid stateId = getStateId(block);
            m_addMotionMessage.setAnimationStateId(stateId);
            m_addMotionMessage.setSceneObjectId(m_sceneObjectId);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addMotionMessage);
        }
    }
}

void AnimationNodeWidget::showConnectionMenu(const QPoint& point, NodeViewConnection* connection)
{
    QMenu menu;
    const json& startStateJson = getStateJson(connection->startPort()->block());
    const json& endStateJson = getStateJson(connection->endPort()->block());
    GAnimationStateType startType = (GAnimationStateType)startStateJson["stateType"].get<Int32_t>();
    GAnimationStateType endType = (GAnimationStateType)endStateJson["stateType"].get<Int32_t>();
    QAction* addTransitionAction;
    if (startType == EAnimationStateType::eAnimation &&
        endType == EAnimationStateType::eAnimation) {
        // Only add transitions when both states are of correct type
        addTransitionAction = menu.addAction("Add Transition");
    }

    QAction* splitAction = menu.addAction("Split");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete");
    QAction* selection = menu.exec(point);
    if (selection == deleteAction)
    {
        onDisconnectedPorts(getConnectionId(connection));
        delete connection;
    }
    else if (selection == addTransitionAction) {
        Uuid stateMachineId = m_animationComponentJson["animationController"]["stateMachineId"];
        m_addTransitionMessage.setConnectionId(getConnectionId(connection));
        m_addTransitionMessage.setStateMachineId(stateMachineId);

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

void AnimationNodeWidget::setMotionWidget(json& motionJson)
{
    clearDetailWidget();

    m_detailWidget = new AnimMotionWidget(m_widgetManager, motionJson, m_sceneObjectId, this);

    layoutDetailWidget();
}

void AnimationNodeWidget::setStateWidget(json& stateJson)
{
    clearDetailWidget();

    m_detailWidget = new AnimStateWidget(m_widgetManager, stateJson, m_animationComponentJson, m_sceneObjectId, this);

    layoutDetailWidget();
}

void AnimationNodeWidget::clearDetailWidget()
{
    if (m_detailWidget) {
        m_gridLayout->removeWidget(m_detailWidget);
        delete m_detailWidget;
        m_detailWidget = nullptr;
    }
}

void AnimationNodeWidget::clearTimelineWidget()
{
    m_timeline->view()->scene()->clear();
    m_gridLayout->removeWidget(m_timeline->view());

}

void AnimationNodeWidget::requestAnimationData()
{
    Uuid stateMachineId = m_animationComponentJson["animationController"]["stateMachineId"];
    m_requestAnimationDataMessage.setStateMachineId(stateMachineId);
    m_requestAnimationDataMessage.setSceneObjectId(m_sceneObjectId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestAnimationDataMessage);
}

void AnimationNodeWidget::layoutDetailWidget()
{
    m_gridLayout->addWidget(m_detailWidget, 0, 2, 2, 1);
}

void AnimationNodeWidget::initializeTimeline()
{
    if (m_timeline) {
        delete m_timeline;
    }
    m_timeline = new Timeline(false, nullptr);
    m_timeline->scene()->setTickIncrement(5.0f);
    m_timeline->setTitle("Animation Resources");

    // Iterate through animations to add 
    for (const auto& animationJsonPair : m_animations.items()) {
        // Get animation JSON
        const json& animationJson = animationJsonPair.value();

        size_t rowIndex = m_timeline->addRow(
            animationJson["name"].get_ref<const std::string&>().c_str(), 
            false);

        // Add animation frame times to timeline
        std::vector<float> times = animationJson["frameTimes"];
        size_t numFrames = times.size();
        Float64_t ticksToSecs = 1.0 / animationJson["ticksPerSecond"].get<Float64_t>();
        for (size_t i = 0; i < numFrames; i++) {
            float timeInSec = times[i] * ticksToSecs;
            m_timeline->addMarker(rowIndex, timeInSec, false);
        }
    }

    // Refresh tick labels after all animations are added
    m_timeline->refreshTickLabels();
}

void AnimationNodeWidget::layoutTimeline()
{
    QSizePolicy spBottom(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    spBottom.setHorizontalStretch(1);
    spBottom.setVerticalStretch(0.5);
    m_timeline->view()->setSizePolicy(spBottom);
    m_gridLayout->addWidget(m_timeline->view(), 2, 0, 1, 3);
}


} // rev
#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "ripple/network/messages/GAddAnimationStateMachineConnectionMessage.h"
#include "ripple/network/messages/GRemoveAnimationStateMachineConnectionMessage.h"
#include "ripple/network/messages/GRequestAnimationComponentDataMessage.h"
#include "ripple/network/messages/GAnimationComponentDataMessage.h"
#include "ripple/network/messages/GAnimationDataMessage.h"
#include "ripple/network/messages/GAddAnimationComponentMotionMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineTransitionMessage.h"


class QMenu;
class QGraphicsView;
class QSceneView;

namespace rev {

class WidgetManager;
class Timeline;
class TimelineView;
class NodeViewEditor;
class AnimationMotionTreeWidget;
class AnimationStateTreeWidget;
class NodeViewBlock;
class NodeViewConnection;
class NodeViewPort;
class NodeViewCanvas;
class NodeViewScene;

/// @class AnimationNodeWidget
/// @brief Generic component widget that uses JSON data to modify components
class AnimationNodeWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    AnimationNodeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, QWidget *parent = 0);
    ~AnimationNodeWidget();

    /// @}

    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @brief Update with data for an animation component
    void update(const GAnimationComponentDataMessage& animationDataMessage);

    /// @brief Update with data for a single animation
    void update(const GAnimationDataMessage& animationDataMessage);

    json& motionJson() { return m_motions; }

    json& statesJson() { return m_states; }

    json& transitionsJson() { return m_transitions; }

    /// @brief Setting the widget as stale will cause it to request new animation data in the next frame
    void setStale() { m_isStale = true; }

    /// @}

signals:

    /// @brief Signal that this widget was repopulated
    void repopulated();

    /// @brief Signal that a set of animation data was received
    void receivedAnimationData(const GAnimationDataMessage& animationDataMessage);

protected slots:

    /// @brief Add a block to the animation scene graph, using widget coordinates
    NodeViewBlock* addBlock(const Uuid& uuid, const QPointF& position);

    /// @brief Add a port for the specified state
    NodeViewPort* addPort(const Uuid& stateId, bool isOutput);

    /// @brief Add a connection based on a state connection JSON
    NodeViewConnection* addConnection(const json& connectionJson);

    /// @brief Add a slot for handling mouse event in graphics scene
    void onMouseEvent(QGraphicsSceneMouseEvent*);

    /// @brief Slot for handling state connections
    void onConnectedPorts(NodeViewConnection* connection);

    /// @brief Slot for when ports are disconnected
    void onDisconnectedPorts(const Uuid& connectionId);

protected:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Initialize graph scene
    void initializeAnimationScene();

    /// @brief Get the state corresponding to a block
    Uuid getStateId(NodeViewBlock* block);

    /// @brief Get the state JSON corresponding to a block
    const json& getStateJson(NodeViewBlock* block);

    /// @brief Get the state connection corresponding to a node connection
    Uuid getConnectionId(NodeViewConnection* connection);

    void showBlockMenu(const QPoint& point, NodeViewBlock* block);
    void showConnectionMenu(const QPoint& point, NodeViewConnection* connection);

    void setMotionWidget(json& motionJson);
    void setStateWidget(json& stateJson);

    void clearDetailWidget();
    void layoutDetailWidget();

    /// @brief Initialize timeline widget
    void initializeTimeline();
    void layoutTimeline();
    void clearTimelineWidget();

    /// @brief Request animation data from main application
    void requestAnimationData();

    /// @}

    /// @name Private Members
    /// @{
    
    QGridLayout* m_gridLayout{ nullptr };
    AnimationMotionTreeWidget* m_motionTreeWidget{ nullptr };
    AnimationStateTreeWidget* m_stateTreeWidget{ nullptr };
    QWidget* m_detailWidget{ nullptr };

    Uint32_t m_sceneObjectId;
    json& m_animationComponentJson;
    json m_animations;
    json m_motions;
    json m_states;
    json m_transitions;
    json m_connections;

    bool m_isStale{ false }; ///< Flag to decide whether or not to repopulate widgets on update

    NodeViewEditor* m_editor{ nullptr };
    NodeViewCanvas* m_view{ nullptr };
    NodeViewScene* m_scene{ nullptr };

    Timeline* m_timeline{ nullptr };

    qint32 m_minBlockSpacing;

    GAddAnimationStateMachineConnectionMessage m_addConnectionMessage;
    GRemoveAnimationStateMachineConnectionMessage m_removeConnectionMessage;
    GRequestAnimationComponentDataMessage m_requestAnimationDataMessage;
    GAddAnimationComponentMotionMessage m_addMotionMessage;
    GAddAnimationStateMachineTransitionMessage m_addTransitionMessage;

    /// @}
};


// End namespaces        
}

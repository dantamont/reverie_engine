#ifndef GB_ANIMATION_NODE_WIDGET_H
#define GB_ANIMATION_NODE_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../parameters/GParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
class QMenu;
class QGraphicsView;
class QSceneView;

namespace rev {

class Motion;
class BoneAnimationComponent;
class BaseAnimationState;
class CoreEngine;
class StateConnection;

namespace View {

class Timeline;
class TimelineView;
class NodeViewEditor;
class AnimationTreeWidget;
class NodeViewBlock;
class NodeViewConnection;
class NodeViewPort;
class NodeViewCanvas;
class NodeViewScene;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationNodeWidget
/// @brief Generic component widget that uses JSON data to modify components
class AnimationNodeWidget : public ParameterWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    AnimationNodeWidget(CoreEngine* core, BoneAnimationComponent* animComp, QWidget *parent = 0);
    ~AnimationNodeWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

protected slots:

    /// @brief Add a block to the animation scene graph, using widget coordinates
    NodeViewBlock* addBlock(const Uuid& uuid, const QPointF& position);

    /// @brief Add a port for the specified state
    NodeViewPort* addPort(const BaseAnimationState* state, bool isOutput);

    /// @brief Add a connection based on an actual state connection
    NodeViewConnection* addConnection(const StateConnection& connection);

    /// @brief Add a slot for handling mouse event in graphics scene
    void onMouseEvent(QGraphicsSceneMouseEvent*);

    /// @brief Slot for handling state connections
    void onConnectedPorts(NodeViewConnection* connection);

    /// @brief Slot for when ports are disconnected
    void onDisconnectedPorts(StateConnection* connection);

protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Initialize graph scene
    void initializeAnimationScene();

    /// @brief Get the state with the specified uuid
    BaseAnimationState* getState(const Uuid& uuid) const;

    /// @brief Get the states associated with the animation component
    const std::vector<BaseAnimationState*>& getStates() const;

    /// @brief Get the state corresponding to a block
    BaseAnimationState* getState(NodeViewBlock* block);

    /// @brief Get the state connection corresponding to a node connection
    StateConnection* getConnection(NodeViewConnection* connection);

    void showBlockMenu(const QPoint& point, NodeViewBlock* block);
    void showConnectionMenu(const QPoint& point, NodeViewConnection* connection);

    void setMotionWidget(Motion* motion);
    void setStateWidget(BaseAnimationState* state);

    void clearDetailWidget();
    void layoutDetailWidget();

    /// @brief Initialize timeline widget
    void initializeTimeline();
    void layoutTimeline();
    void clearTimelineWidget();

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{
    
    QGridLayout* m_gridLayout;
    AnimationTreeWidget* m_motionTreeWidget;
    AnimationTreeWidget* m_stateTreeWidget;
    QWidget* m_detailWidget;

    BoneAnimationComponent* m_animationComponent;

    //QMenu* m_menu;

    NodeViewEditor* m_editor;
    NodeViewCanvas* m_view;
    NodeViewScene* m_scene;

    Timeline* m_timeline;

    qint32 m_minBlockSpacing;

    /// @}
};




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
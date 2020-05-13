#ifndef GB_GRAPH_WIDGET_H 
#define GB_GRAPH_WIDGET_H
/** @file GbGraphWidget.h
    @brief Defines graph widget class
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QGraphicsView>

// Project
#include "../base/GbTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GraphNode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GraphWidget : public QGraphicsView, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "GraphWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::GraphWidget"; }

    /// @brief Indicates whether this Service has a UI or not.
    virtual bool hasUi() const { return true; };

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const { return false; };

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{

    GraphWidget(const QString& name, QWidget *parent = 0);

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Notify changes in the scene's node graph
    /// @details Restarts main timer if not running, stops when graph stabilizes
    void itemMoved();

    /// @}

public slots:
    void shuffle();
    void zoomIn();
    void zoomOut();


protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief handle resize event
    /// @details Overrides so that scene scales with view size
    void resizeEvent(QResizeEvent *event) override;

    /// @brief handle key presses
    void keyPressEvent(QKeyEvent *event) override;

    /// @brief handle timer event
    void timerEvent(QTimerEvent *event) override;

#if QT_CONFIG(wheelevent)
    /// @brief controls mouse zoom event
    /// @details Converts mouse wheel delta to a scale factor
    void wheelEvent(QWheelEvent *event) override;
#endif
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    /// @brief Scales view, limiting zoom amount
    void scaleView(qreal scaleFactor);

    /// @}

private:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private members
    /// @{

    /// @brief ID of the QObject timer for this widget
    int timerId;

    GraphNode* centerNode;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_GRAPH_WIDGET_H 






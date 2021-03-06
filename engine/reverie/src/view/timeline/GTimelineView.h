#ifndef GB_TIMELINEVIEW_H
#define GB_TIMELINEVIEW_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <iostream>
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {
namespace View {

class TimelineScene;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////
class TimelineView: public QGraphicsView{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{

    TimelineView(TimelineScene *scene);
    ~TimelineView();

    /// @}
    
    //---------------------------------------------------------------------------------------
    /// @name QWidget Overrides
    /// @{

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    /// @}

signals:
    void sendMousePressEventSignal(QMouseEvent *event);
    void sendMouseMoveEventSignal(QMouseEvent *event);
    void sendMouseReleaseEventSignal(QMouseEvent *event);

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Control zoom level
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}


#endif //TimelineView.h

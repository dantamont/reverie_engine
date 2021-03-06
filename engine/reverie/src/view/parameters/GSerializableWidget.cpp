#include "GSerializableWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/readers/GJsonReader.h"
#include "../style/GFontIcon.h"

namespace rev {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// SerializableWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
SerializableWidget::SerializableWidget(CoreEngine* core, Serializable* component, QWidget *parent):
    JsonWidget(core, component, parent)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SerializableWidget::~SerializableWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializableWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializableWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(350);

    JsonWidget::layoutWidgets();

    // Note, cannot call again without deleting previous layout
    // https://doc.qt.io/qt-5/qwidget.html#setLayout
    setLayout(m_mainLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializableWidget::preLoad()
{
    //// Pause scenario to edit component
    //SimulationLoop* simLoop = m_engine->simulationLoop();
    //m_wasPlaying = simLoop->isPlaying();
    //if (m_wasPlaying) {
    //    simLoop->pause();
    //}
    //m_engine->simulationLoop()->pause();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void SerializableWidget::postLoad()
{
    //// Unpause scenario
    //SimulationLoop* simLoop = m_engine->simulationLoop();
    //if (m_wasPlaying) {
    //    simLoop->play();
    //}
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
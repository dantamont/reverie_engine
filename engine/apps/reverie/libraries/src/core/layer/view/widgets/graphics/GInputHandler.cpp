#include "core/layer/view/widgets/graphics/GInputHandler.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "fortress/constants/GConstants.h"
#include "fortress/containers/GContainerExtensions.h"

#include <QCursor>

namespace rev {

void InputHandler::dispatchEvent(QInputEvent* inputEvent) const
{
    // Add event to event handler queue
    const GLWidget* myWidget = static_cast<const GLWidget*>(m_widget);
    myWidget->m_engine->eventManager()->addEvent(inputEvent);
}

} // End namespaces
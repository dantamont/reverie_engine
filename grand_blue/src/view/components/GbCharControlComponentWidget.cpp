#include "GbCharControlComponentWidget.h"

#include "../components/GbPhysicsWidgets.h"
#include "../../core/physics/GbPhysicsShape.h"
#include "../../core/physics/GbPhysicsGeometry.h"
#include "../../core/physics/GbPhysicsManager.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/components/GbPhysicsComponents.h"
#include "../../core/components/GbShaderComponent.h"
#include "../../core/scripting/GbPythonScript.h"

#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"

#include "../../core/physics/GbPhysicsActor.h"

namespace Gb {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CharControlWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CharControlWidget::CharControlWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CharControlComponent* CharControlWidget::charControlComponent() const {
    return static_cast<CharControlComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CharControlWidget::~CharControlWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();
    "walking";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CharControlWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();
}






///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb
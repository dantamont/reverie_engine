//#include "geppetto/qt/widgets/components/GCharControlComponentWidget.h"
//
//#include "../components/GPhysicsWidgets.h"
//#include "core/physics/GPhysicsShape.h"
//#include "core/physics/GPhysicsGeometry.h"
//#include "core/physics/GPhysicsManager.h"
//
//#include "core/GCoreEngine.h"
//#include "core/loop/GSimLoop.h"
//#include "geppetto/qt/widgets/components/GComponentWidget.h"
//#include "core/resource/GResourceCache.h"
//
//#include "core/scene/GSceneObject.h"
//#include "fortress/json/GJson.h"
//#include "core/components/GScriptComponent.h"
//#include <core/components/GCharControlComponent.h>
//#include <core/components/GRigidBodyComponent.h>
//#include "core/components/GShaderComponent.h"
//#include "core/scripting/GPythonScript.h"
//
//#include "geppetto/qt/style/GFontIcon.h"
//#include "fortress/containers/math/GEulerAngles.h"
//
//#include "core/physics/GPhysicsActor.h"
//
//namespace rev {
//
//
//
//
//// CharControlWidget
//
//CharControlWidget::CharControlWidget(CoreEngine* core,
//    Component* component, 
//    QWidget *parent) :
//    ComponentWidget(core, component, parent) {
//    initialize();
//}
//
//CharControlComponent* CharControlWidget::charControlComponent() const {
//    return static_cast<CharControlComponent*>(m_component);
//}
//
//CharControlWidget::~CharControlWidget()
//{
//}
//
//void CharControlWidget::initializeWidgets()
//{
//    ComponentWidget::initializeWidgets();
//    "walking";
//}
//
//void CharControlWidget::initializeConnections()
//{
//    ComponentWidget::initializeConnections();
//}
//
//void CharControlWidget::layoutWidgets()
//{
//    ComponentWidget::layoutWidgets();
//}
//
//
//
//
//
//
//} // rev
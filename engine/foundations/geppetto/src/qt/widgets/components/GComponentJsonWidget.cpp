#include "geppetto/qt/widgets/components/GComponentJsonWidget.h"

#include "fortress/json/GJson.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GUpdateJsonMessage.h"
#include "ripple/network/messages/GOnUpdateJsonMessage.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {

ComponentJsonWidget::ComponentJsonWidget(WidgetManager* wm, const json& j, GSceneObjectComponentType soComponentType, Int32_t sceneObjectId, QWidget* parent) :
    JsonWidget(wm, j, { {"isComponentItem", true}, {"sceneObjectId", sceneObjectId}, { "sceneObjectType", Int32_t(soComponentType) } }, parent, false)
{
    initialize();
}

ComponentJsonWidget::ComponentJsonWidget(WidgetManager* wm, const json& j, GSceneComponentType sceneComponentType, QWidget* parent):
    JsonWidget(wm, j, { {"isComponentItem", true}, { "sceneType", Int32_t(sceneComponentType) } }, parent, false)
{
    initialize();
}



} // rev
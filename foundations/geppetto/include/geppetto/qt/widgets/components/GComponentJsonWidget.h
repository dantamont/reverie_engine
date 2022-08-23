#pragma once

// External
#include "enums/GSceneComponentTypeEnum.h"
#include "enums/GSceneObjectComponentTypeEnum.h"

// Internal
#include "geppetto/qt/widgets/general/GJsonWidget.h"

namespace rev {
/// @class ComponentJsonWidget
/// @brief Widget for handling JSON representation of a scene or scene object component
class ComponentJsonWidget : public JsonWidget {
    //Q_OBJECT ///< Incompatible with templates, and only needed for signals/slots anyway
public:
    /// @name Constructors/Destructor
    /// @{

    ComponentJsonWidget(WidgetManager* wm, const json& j, GSceneObjectComponentType soComponentType, Int32_t soId, QWidget* parent = nullptr);
    ComponentJsonWidget(WidgetManager* wm, const json& j, GSceneComponentType soComponentType, QWidget* parent = nullptr);
    virtual ~ComponentJsonWidget() = default;

    /// @}
};


// End namespaces
}

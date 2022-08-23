#pragma once

// Third-party
#include <QtWidgets>

// External
#include "ripple/network/messages/GSelectSceneObjectShaderPresetMessage.h"

// Internal
#include "GComponentWidget.h"

namespace rev {

/// @class ShaderComponentWidget
/// @brief Widget representing a shader preset component
class ShaderComponentWidget : public SceneObjectComponentWidget {
public:

    /// @name Constructors/Destructor
    /// @{

    ShaderComponentWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget* parent = 0);
    ~ShaderComponentWidget();

    /// @}

private:

    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    QComboBox* m_presetSelectWidget{ nullptr };
    GSelectSceneObjectShaderPresetMessage m_selectedPresetMessage; ///< Message sent when selecting a shader preset

    /// @}
};



// End namespaces        
}
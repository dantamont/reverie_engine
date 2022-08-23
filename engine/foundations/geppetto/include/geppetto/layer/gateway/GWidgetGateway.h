#pragma once

#include "fortress/layer/framework/GSignalSlot.h"

#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

class CoreEngine;
class WidgetManager;

/// @class WidgetGateway
/// @brief Handles messages received by widgets
class WidgetGateway: public GMessageGateway {
public:

    WidgetGateway(WidgetManager* widgetManager);
    ~WidgetGateway() = default;

    /// @brief Process messages sent by application
    void processMessages() override;

protected:

    using GMessageGateway::processMessage;

    void processMessage(GPlaybackDataMessage* message) override;

    void processMessage(GSceneObjectAddedMessage* message) override;

    void processMessage(GSceneObjectRemovedMessage* message) override;

    void processMessage(GSceneObjectCopiedMessage* message) override;

    void processMessage(GScenarioModifiedMessage* message) override;

    void processMessage(GSceneObjectSelectedMessage* message) override;

    void processMessage(GOnSceneComponentAddedMessage* message) override;

    void processMessage(GOnUpdateJsonMessage* message) override;

    void processMessage(GScenarioJsonMessage* message) override;

    void processMessage(GTransformMessage* message) override;

    void processMessage(GRenderSettingsInfoMessage* message) override;

    void processMessage(GAnimationStateMachinesMessage* message) override;

    void processMessage(GAnimationComponentDataMessage* message) override;

    void processMessage(GAnimationDataMessage* message) override;

    void processMessage(GCubemapsDataMessage* message) override;

    void processMessage(GModelDataMessage* message) override;

    void processMessage(GCanvasComponentDataMessage* message) override;

    void processMessage(GAudioResourceDataMessage* message) override;

    void processMessage(GAudioResourcesDataMessage* message) override;

    void processMessage(GResourcesDataMessage* message) override;

    void processMessage(GResourceAddedMessage* message) override;
    
    void processMessage(GResourceModifiedMessage* message) override;

    void processMessage(GResourceRemovedMessage* message) override;

    void processMessage(GDisplayWarningMessage* message) override;

    void processMessage(GResourceDataMessage* message) override;

    void processMessage(GBlueprintsDataMessage* message) override;

    void processMessage(GAdvanceProgressWidgetMessage* message) override;

    void processMessage(GRequestAddTexturesToMaterialMessage* message) override;

private:

    WidgetManager* m_widgetManager{ nullptr }; ///< The widget manager, exists independently of engine

};

}
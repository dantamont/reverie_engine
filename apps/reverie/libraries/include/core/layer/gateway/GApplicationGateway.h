#pragma once

#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/thread/GThreadpool.h"

#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

class CoreEngine;
class WidgetManager;

/// @class ApplicationGateway
/// @brief Handles messages received by widgets
class ApplicationGateway: public GMessageGateway {
protected:

    static constexpr Uint64_t s_defaultSendTimeMicroseconds = 0;

public:

    ApplicationGateway(CoreEngine* engine);
    ~ApplicationGateway() = default;

    /// @brief Process messages sent by widgets
    void processMessages() override;

    /// @brief Send message to widget manager on scene object selection in GL view
    /// @detail Used for signaling widgets to update display
    /// @todo This should be a signal, which sends the message from somewhere else
    void sceneObjectSelected(Int32_t id);

    /// @brief Set up connections for the gateway
    void initializeConnections();

protected:

    /// @brief Send message to widget manager on scenario change
    /// @detail Used for signaling widgets to update display lists
    /// @todo This should be a signal, which sends the message from somewhere else
    void scenarioLoaded();

    /// @brief Send message to widget manager on scenario change
    /// @detail Used for signaling widgets to update display lists
    /// @todo This should be a signal, which sends the message from somewhere else
    //void scenarioChanged();

    using GMessageGateway::processMessage;

    void processMessage(GRequestPlaybackDataMessage* message) override;

    void processMessage(GTogglePlaybackMessage* message) override;

    void processMessage(GTogglePlaybackModeMessage* message) override;

    void processMessage(GCreatePolygonMeshMessage* message) override;

    void processMessage(GAddScenarioMessage* message) override;

    void processMessage(GRestorePreviousScenarioMessage* message) override;

    void processMessage(GAddSceneObjectMessage* message) override;

    void processMessage(GRemoveSceneObjectMessage* message) override;

    void processMessage(GRenameScenarioMessage* message) override;

    void processMessage(GRenameSceneMessage* message) override;

    void processMessage(GRenameSceneObjectMessage* message) override;

    void processMessage(GReparentSceneObjectMessage* message) override;

    void processMessage(GCopySceneObjectMessage* message) override;

    void processMessage(GAddSceneComponentMessage* message) override;

    void processMessage(GRemoveSceneComponentMessage* message) override;

    void processMessage(GUpdateJsonMessage* message) override;

    void processMessage(GGetScenarioJsonMessage* message) override;

    void processMessage(GToggleComponentMessage* message) override;

    void processMessage(GResetPythonScriptMessage* message) override;

    void processMessage(GRequestTransformMessage* message) override;

    void processMessage(GTransformUpdateMessage* message) override;

    void processMessage(GSelectSceneObjectShaderPresetMessage* message) override;

    void processMessage(GUpdatePhysicsShapePrefabMessage* message) override;
    
    void processMessage(GUpdateRigidBodyComponentMessage* message) override;

    void processMessage(GCreatePhysicsShapePrefabMessage* message) override;

    void processMessage(GDeletePhysicsShapePrefabMessage* message) override;

    void processMessage(GAddSceneObjectRenderLayerMessage * message) override;

    void processMessage(GRemoveSceneObjectRenderLayerMessage* message) override;
    
    void processMessage(GAddCameraRenderLayerMessage* message) override;

    void processMessage(GRemoveCameraRenderLayerMessage* message) override;

    void processMessage(GUpdateLightComponentMessage* message) override;

    void processMessage(GSetLightComponentTypeMessage* message) override;

    void processMessage(GToggleLightComponentShadowsMessage* message) override;

    void processMessage(GRequestRenderSettingsInfoMessage* message) override;

    void processMessage(GRequestAnimationStateMachinesMessage* message) override;

    void processMessage(GSetAnimationComponentStateMachineMessage* message) override;

    void processMessage(GRenameAnimationStateMachineMessage* message) override;

    void processMessage(GAddAnimationStateMachineMessage* message) override;

    void processMessage(GAddAnimationStateMachineConnectionMessage* message) override;

    void processMessage(GRemoveAnimationStateMachineConnectionMessage* message) override;

    void processMessage(GRequestAnimationComponentDataMessage* message) override;

    void processMessage(GAddAnimationComponentMotionMessage* message) override;
    
    void processMessage(GAddAnimationStateMachineTransitionMessage* message) override;

    void processMessage(GModifyAnimationStateMachineTransitionMessage* message) override;
    
    void processMessage(GModifyAnimationComponentMotionMessage* message) override;

    void processMessage(GAddAnimationClipToStateMessage* message) override;

    void processMessage(GRemoveAnimationClipFromStateMessage* message) override;
        
    void processMessage(GAddedAnimationMotionMessage* message) override;

    void processMessage(GRemovedAnimationMotionMessage* message) override;

    void processMessage(GAddAnimationStateMachineStateMessage* message) override;

    void processMessage(GRemoveAnimationStateMachineStateMessage* message) override;

    void processMessage(GModifyAnimationClipMessage* message) override;

    void processMessage(GRequestAnimationDataMessage* message) override;

    void processMessage(GModifyCameraClearColorMessage* message) override;

    void processMessage(GModifyCameraOptionFlagsMessage* message) override;

    void processMessage(GModifyCameraViewportMessage* message) override;

    void processMessage(GModifyCameraRenderProjectionMessage* message) override;

    void processMessage(GModifyCameraCubemapMessage* message) override;

    void processMessage(GRequestCubemapsDataMessage* message) override;
    
    void processMessage(GSetSceneObjectModelMessage* message) override;
    
    void processMessage(GRequestModelDataMessage* message) override;

    void processMessage(GUpdateRenderSettingsMessage* message) override;

    void processMessage(GModifyCubemapColorMessage* message) override;

    void processMessage(GModifyCubemapNameMessage* message) override;

    void processMessage(GModifyCubemapTextureMessage* message) override;

    void processMessage(GModifyDefaultCubemapMessage* message) override;

    void processMessage(GModifyCanvasBillboardFlagsMessage* message) override;

    void processMessage(GModifyCanvasGlyphModeMessage* message) override;

    void processMessage(GModifyGlyphAlignmentMessage* message) override;

    void processMessage(GModifyLabelTextMessage* message) override;

    void processMessage(GModifyLabelFontMessage* message) override;

    void processMessage(GModifyLabelSpacingMessage* message) override;

    void processMessage(GModifyLabelColorMessage* message) override;

    void processMessage(GModifyIconFontSizeMessage* message) override;

    void processMessage(GModifyIconNameMessage* message) override;

    void processMessage(GModifyIconColorMessage* message) override;

    void processMessage(GAddGlyphMessage* message) override;

    void processMessage(GReparentGlyphMessage* message) override;

    void processMessage(GRemoveGlyphMessage* message) override;

    void processMessage(GModifyAudioComponentResourceMessage* message) override;

    void processMessage(GModifyAudioComponentSourceFlagsMessage* message) override;

    void processMessage(GModifyAudioComponentSourceAttenuationMessage* message) override;

    void processMessage(GModifyAudioComponentSourceVolumeMessage* message) override;

    void processMessage(GModifyAudioComponentSettingsMessage* message) override;

    void processMessage(GRequestAudioResourceDataMessage* message) override;

    void processMessage(GRequestAudioResourcesDataMessage* message) override;

    void processMessage(GModifyListenerScriptMessage* message) override;

    void processMessage(GModifyAudioListenerMessage* message) override;

    void processMessage(GModifyResourceMessage* message) override;

    void processMessage(GLoadTextureResourceMessage* message) override;

    void processMessage(GUnloadTextureResourceMessage* message) override;

    void processMessage(GAddMaterialResourceMessage* message) override;

    void processMessage(GAddTextureToMaterialMessage* message) override;

    void processMessage(GRemoveMaterialResourceMessage* message) override;

    void processMessage(GCopyMaterialResourceMessage* message) override;

    void processMessage(GRemoveMeshResourceMessage* message) override;

    void processMessage(GAddModelResourceMessage* message) override;

    void processMessage(GRemoveModelResourceMessage* message) override;

    void processMessage(GLoadModelMessage* message) override;

    void processMessage(GCopyModelResourceMessage* message) override;

    void processMessage(GLoadAudioResourceMessage* message) override;

    void processMessage(GRemoveAudioResourceMessage* message) override;

    void processMessage(GLoadShaderResourceMessage* message) override;

    void processMessage(GAddShaderResourceMessage* message) override;

    void processMessage(GRemoveShaderResourceMessage* message) override;

    void processMessage(GReloadResourceMessage* message) override;

    void processMessage(GGetResourceDataMessage* message) override;

    void processMessage(GRequestResourcesDataMessage* message) override;

    void processMessage(GCreateBlueprintMessage* message) override;

    void processMessage(GRequestBlueprintsDataMessage* message) override;

    void processMessage(GModifyBlueprintMessage* message) override;

    void processMessage(GAddShaderPresetMessage* message) override;
    
    void processMessage(GRemoveShaderPresetMessage* message) override;

    void processMessage(GReorderRenderLayersMessage* message) override;

    void processMessage(GAddRenderLayerMessage* message) override;

    void processMessage(GRemoveRenderLayerMessage* message) override;

    void processMessage(GReorderScriptProcessesMessage* message) override;

    void processMessage(GAddScriptProcessLayerMessage* message) override;

    void processMessage(GRemoveScriptProcessLayerMessage* message) override;

private:

    CoreEngine* m_engine{ nullptr };

    /// @brief Scenes
    json m_previousLoadedScenario;
    json m_previousScenarioPath;
    
    static constexpr Uint32_t s_workerThreadCount = 1;
    ThreadPool m_threadPool{ s_workerThreadCount }; ///< The thread pool for offloading any asynchronous operations
};

}
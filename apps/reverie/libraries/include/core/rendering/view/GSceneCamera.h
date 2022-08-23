#pragma once

// Internal
#include "GCamera.h"

namespace rev {

/// @brief Flags for toggling camera settings
enum class CameraOption {
    kFrustumCulling = (1 << 0), // Flag to toggle frustum culling
    kOcclusionCulling = (1 << 1), // Flag to toggle occlusion culling
    kShowAllRenderLayers = (1 << 2), // Flag to force rendering of all render layers, even those unassociated with the camera
    kEnablePostProcessing = (1 << 3), // Flag to enable post-processing effects
    kUseZBufferPass = (1 << 4) // Flag to make a zbuffer pass before rendering scene
};
typedef Flags<CameraOption> CameraOptions;

/// @class SceneCamera
/// @brief Camera to be attached to a scene object via a Camera Component
class SceneCamera : public Camera {
public:
    /// @name Static
    /// @{

    enum ColorAttachment {
        kFragColor = 0,
        kNormals = 1,
        kObjectIDs = 2
    };

    /// @}

    /// @name Constructors/Destructor
    /// @{

    SceneCamera(CameraComponent* component);
    ~SceneCamera();

    /// @}

    /// @name Properties
    /// @{

    FrameBuffer& ssaoFrameBuffer() {
        return m_ssaoFrameBuffer;
    }

    FrameBuffer& ssaoBlurFrameBuffer() {
        return m_ssaoBlurFrameBuffer;
    }

    virtual CameraType cameraType() const {
        return CameraType::kSceneCamera;
    }

    /// @brief Chain of post-processing effects for this camera
    std::shared_ptr<PostProcessingChain>& postProcessingChain() {
        return m_postProcessingChain;
    }

    /// @brief The grid used for light culling
    LightClusterGrid& lightClusterGrid() { return m_lightClusterGrid; }

    /// @brief Camera options (flags)
    CameraOptions& cameraOptions() { return m_cameraOptions; }
    const CameraOptions& cameraOptions() const { return m_cameraOptions; }
    CameraComponent* cameraComponent() { return m_component; }

    std::vector<SortingLayer> renderLayers() const;
    const std::vector<Uint32_t>& renderLayerIds() const;

    void addRenderLayer(Uint32_t layerId);
    void removeRenderLayer(Uint32_t layerId);

    /// @}

    /// @name Public Methods
    /// @{

    virtual void bindTextures() override;

    /// @brief Render a framebuffer quad to screen
    virtual void drawFrameBufferQuad(CoreEngine* core) override;

    /// @brief Resize the camera's framebuffer to accomodate the given renderer
    virtual void resizeFrame(uint32_t width, uint32_t height) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() override;

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    virtual void updateFrustum(CameraUpdateFlags flags) override;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SceneCamera& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SceneCamera& orObject);


    /// @}

protected:

    /// @name Friends
    /// @{

    friend class CameraComponent;
    friend class LightClusterGrid;
    friend class PostProcessingChain;
    //friend class rev::SceneObject;

    /// @}

    /// @name Protected Methods
    /// @{

    void setDefaultRenderLayers();

    void onRenderLayerAdded(Uint32_t layer);
    void onRenderLayerRemoved(Uint32_t layer);

    /// @}

    /// @name Protected Members
    /// @{

    FrameBuffer m_ssaoFrameBuffer;
    FrameBuffer m_ssaoBlurFrameBuffer;
    CameraOptions m_cameraOptions; ///< Miscellaneous options for the camera
    CameraComponent* m_component{ nullptr }; ///< The camera component that owns this camera
    std::shared_ptr<PostProcessingChain> m_postProcessingChain{ nullptr }; ///< The post-processing chain for this camera 
    LightClusterGrid m_lightClusterGrid; ///< The grid used for light culling

    /// @details IDs match those found in scenario settings
    std::vector<Uint32_t> m_renderLayers; ///<  Vector of rendering layers drawn by this camera

    /// @}
};


    
// End namespaces
}

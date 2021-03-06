//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCENE_CAMERA_H
#define GB_SCENE_CAMERA_H

// QT

// Internal
#include "GCamera.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SceneCamera
/// @brief Camera to be attached to a scene object via a Camera Component
class SceneCamera : public Camera {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ColorAttachment {
        kFragColor = 0,
        kNormals = 1,
        kObjectIDs = 2
    };

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    SceneCamera(CameraComponent* component);
    ~SceneCamera();

    /// @}

    //---------------------------------------------------------------------------------------
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

    std::vector<SortingLayer*> renderLayers() const;

    std::vector<size_t>& renderLayerIds() { return m_renderLayers; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void bindTextures() override;

    /// @brief Render a framebuffer quad to screen
    virtual void drawFrameBufferQuad(CoreEngine* core) override;

    /// @brief Resize the camera's framebuffer to accomodate the given renderer
    virtual void resizeFrame(size_t width, size_t height) override;

    /// @brief Bind any buffers associated with the camera
    virtual void bindBuffers() override;

    /// @brief Updates the frustum with the latest view/projection matrices from the camera
    virtual void updateFrustum(CameraUpdateFlags flags) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "SceneCamera"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::SceneCamera"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class CameraComponent;
    friend class LightClusterGrid;
    friend class PostProcessingChain;
    //friend class rev::SceneObject;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    void setDefaultRenderLayers();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    FrameBuffer m_ssaoFrameBuffer;
    FrameBuffer m_ssaoBlurFrameBuffer;

    /// @brief Miscellaneous options for the camera
    CameraOptions m_cameraOptions;

    /// @brief The camera component that owns this camera
    CameraComponent* m_component;

    /// @brief The post-processing chain for this camera
    std::shared_ptr<PostProcessingChain> m_postProcessingChain = nullptr;

    /// @brief The grid used for light culling
    LightClusterGrid m_lightClusterGrid;

    /// @brief Vector of rendering layers drawn by this camera
    std::vector<size_t> m_renderLayers;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
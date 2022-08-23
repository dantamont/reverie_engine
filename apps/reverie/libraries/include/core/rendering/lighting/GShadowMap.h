#pragma once

// Internal
#include "fortress/containers/math/GMatrix.h"
#include "core/rendering/view/GCamera.h"

#define LIGHT_SPACE_MATRIX_UNIFORM "lightSpaceMatrix"
#define SHADOW_MAP_UNIFORM "shadowMap"
#define SHADOW_BUFFER_NAME QStringLiteral("ShadowBuffer")

namespace rev {  

class OpenGlRenderer;
class LightingSettings;
class LightComponent;
class Scene;
class SceneCamera;
class RenderContext;

/// @struct ShadowInfo
/// @brief Struct containing data associating a light with a shadow map
/// @details For use in shadows SSBO
struct ShadowInfo {

    /// @brief Get far clip plane for point light
    float farClipPlane() const { return m_mapIndexBiasesFarClip[3]; }
    void setFarClipPlane(float clip) { m_mapIndexBiasesFarClip[3] = clip; }

    /// @brief Get near clip plane for point light
    float nearClipPlane() const { return m_attributesOrLightMatrix(0, 0); }
    void setNearClipPlane(float clip) { m_attributesOrLightMatrix(0, 0) = clip; }

    /// @brief the Index of the shadow map to use for shadow rendering
    /// @details Map index, biases for fixing aliasing, and far clip of light camera
    Vector4 m_mapIndexBiasesFarClip = Vector4(-1, 0, 0, 0);

    Vector4 __pad0__;
    Vector4 __pad1__;
    Vector4 __pad2__;

    /// @brief The light space matrix of the light corresponding to this set of Shadow Info
    Matrix4x4g m_attributesOrLightMatrix;
};

/// @class ShadowMap
/// @see https://community.khronos.org/t/cascaded-shadow-mapping-and-gl-texture-2d-array/71482
/// @see https://www.gamedev.net/forums/topic/657728-standard-approach-to-shadow-mapping-multiple-light-sources/
/// @see https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
/// @see On FBOS with texture arrays https://stackoverflow.com/questions/31793466/using-gl-texture-2d-array-as-a-draw-target
/// @see https://community.khronos.org/t/depth-only-draw-into-a-texture-array/62085
class ShadowMap: public IdentifiableInterface {
public:
    /// @name Constructors/Destructor
    /// @{
    ShadowMap(RenderContext& context, LightComponent* lightComponent, size_t mapIndex);
    ~ShadowMap();

    /// @}

    /// @name Properties

    /// @brief The camera representing the view from the light's perspective
    AbstractCamera* camera() { return m_camera.get(); }
    const AbstractCamera* camera() const { return m_camera.get(); }

    const LightComponent* lightComponent() const { return m_lightComponent; }


    /// @}

    /// @name Public Methods

    /// @brief Initialize camera based on light type
    void initializeCamera(RenderContext& context, size_t mapIndex);

    /// @brief Resize the shadow map to the specified resolution
    void resizeShadowMap(size_t width, size_t height);

    /// @brief Update light matrix
    void updateShadowAttributes(const Scene& sc);

    /// @brief Set view matrix, given a scene
    void setView(const Scene& sc);

    /// @brief Set projection matrix, given a scene's camera
    void setProjection(const Scene& sc);

    /// @}

protected:
    /// @name Protected members
    /// @{

    LightComponent* m_lightComponent; ///< The light component for the shadow map
    std::unique_ptr<AbstractCamera> m_camera = nullptr; ///< The camera for the shadow map

    /// @}
};

    
// End namespaces
}

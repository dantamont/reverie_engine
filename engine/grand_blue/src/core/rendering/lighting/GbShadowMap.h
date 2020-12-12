/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SHADOW_MAP_H
#define GB_SHADOW_MAP_H

// standard

// Internal
#include "../../geometry/GbMatrix.h"
#include "../view/GbCamera.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
#define LIGHT_SPACE_MATRIX_UNIFORM "lightSpaceMatrix"
#define SHADOW_MAP_UNIFORM "shadowMap"
#define SHADOW_BUFFER_NAME QStringLiteral("ShadowBuffer")

namespace Gb {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class MainRenderer;
class LightingSettings;
class LightComponent;
class Scene;
class SceneCamera;
class RenderContext;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
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
    Vector4 m_mapIndexBiasesFarClip;

    Vector4 __pad0__;
    Vector4 __pad1__;
    Vector4 __pad2__;

    /// @brief The light space matrix of the light corresponding to this set of Shadow Info
    Matrix4x4g m_attributesOrLightMatrix;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ShadowMap
// See: https://community.khronos.org/t/cascaded-shadow-mapping-and-gl-texture-2d-array/71482
// https://www.gamedev.net/forums/topic/657728-standard-approach-to-shadow-mapping-multiple-light-sources/
// https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus
// On FBOS with texture arrays https://stackoverflow.com/questions/31793466/using-gl-texture-2d-array-as-a-draw-target
// https://community.khronos.org/t/depth-only-draw-into-a-texture-array/62085
class ShadowMap: public Object {
public:
    //---------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ShadowMap(RenderContext& context, LightComponent* lightComponent, size_t mapIndex);
    ~ShadowMap();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties

    /// @brief The camera representing the view from the light's perspective
    AbstractCamera* camera() { return m_camera; }
    const AbstractCamera* camera() const { return m_camera; }

    const LightComponent* lightComponent() const { return m_lightComponent; }


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods

    void createDrawCommands(Scene& scene, MainRenderer& renderer);

    /// @brief Resize the shadow map to the specified resolution
    void resize(size_t width, size_t height);

    /// @brief Update light matrix
    void updateShadowAttributes(const Scene& sc);

    /// @brief Set view matrix, given a scene
    void setView(const Scene& sc);

    /// @brief Set projection matrix, given a scene's camera
    void setProjection(const Scene& sc);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB Object Properties
    /// @{

    /// @property className
    const char* className() const override { return "ShadowMap"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ShadowMap"; }

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Initialize camera based on light type
    void initializeCamera(RenderContext& context, size_t mapIndex);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The light component for the shadow map
    LightComponent* m_lightComponent;

    /// @brief The camera for the shadow map
    AbstractCamera* m_camera;

    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif
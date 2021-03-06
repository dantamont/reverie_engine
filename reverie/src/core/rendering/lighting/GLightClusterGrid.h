/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_CLUSTER_GRID_H
#define GB_LIGHT_CLUSTER_GRID_H

// standard

// Internal
#include "../../geometry/GMatrix.h"
#include "../GGLFunctions.h"
#include "../../GObject.h"
#include "../buffers/GShaderStorageBuffer.h"
#include "../../geometry/GCollisions.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////////////////
#define CLUSTER_BUFFER_NAME QStringLiteral("ClusterBoundsBuffer")
#define SCREEN_TO_VIEW_BUFFER_NAME QStringLiteral("ScreenToViewBuffer")
#define LIGHT_INDEX_BUFFER_NAME QStringLiteral("LightIndexBuffer")
#define LIGHT_GRID_BUFFER_NAME QStringLiteral("LightGridBuffer")
#define GLOBAL_INDEX_COUNT_BUFFER_NAME QStringLiteral("GlobalIndexCountBuffer")

namespace rev {  

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class MainRenderer;
class LightingSettings;
class SceneCamera;
class RenderContext;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightClusterGrid
class LightClusterGrid : public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    struct ScreenToView {

        bool operator==(const ScreenToView& other) const;

        operator QString() const;

        Matrix4x4g m_inverseProjection;
        Vector<unsigned int, 4> m_tileGridDimensions; // grid size x, grid size y, grid size z, null
        Vector<unsigned int, 4> m_tilePixelSizes; // x pixel width, y pixel height, null, null
        Vector<unsigned int, 2> m_widgetDimensions;

        // solve Z = nearZ(farZ/NearZ)^(slice/numSlices) for slice
        // Result: slice = |log(Z) * numSlices/log(farZ/nearZ) - numSlices*log(nearZ)/log(farZ/nearZ)|
        float m_sliceScalingFactor;   // numSlices/log(Farz/Nearz)
        float m_sliceBiasFactor; // -numSlices*log(nearZ)/log(farZ/nearZ)
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    LightClusterGrid(SceneCamera* camera);
    ~LightClusterGrid();

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::shared_ptr<ShaderStorageBuffer>& screenToViewBuffer() const {
        return m_screenToViewBuffer;
    }

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Bind buffers to binding points
    void bindBufferPoints();

    /// @brief Initialize buffers
    void initializeBuffers(RenderContext& rc);

    /// @brief Cull lights for a render pass
    void cullLights(MainRenderer& renderer);

    /// @brief Callback for screen resizing
    void onResize();

    /// @brief Callback for resizing screen to view
    void updateScreenToView(unsigned int widgetWidth, unsigned int widgetHeight);

    /// @brief Dispatch compute shader to reconstruct light cluster grid
    void reconstructGrid();

    /// @brief Get underlying screen-to-view struct
    ScreenToView getScreenToView() const;

    /// @brief Get grid boxes
    void getGridBoxes(std::vector<AABBData>& outBoxes) const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "LightClusterGrid"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::LightClusterGrid"; }
    /// @}


protected:
    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static methods
    /// @{


    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    SceneCamera* m_camera;

    /// @brief Buffer containing cluster AABBs
    std::shared_ptr<ShaderStorageBuffer> m_lightClusterBuffer = nullptr;

    /// @brief Buffer containing screen-to-view information
    std::shared_ptr<ShaderStorageBuffer> m_screenToViewBuffer = nullptr;

    /// @brief Buffer of indices to lights that are active and intersect with a cluster
    std::shared_ptr<ShaderStorageBuffer> m_activeLightIndexBuffer = nullptr;

    /// @brief Buffer for the light cluster grid
    /// @details Every tile takes two unsigned ints one to represent the number of lights in that grid
    /// Another to represent the offset to the light index list from where to begin reading light indexes from
    /// This implementation is straight up from Olsson paper
    std::shared_ptr<ShaderStorageBuffer> m_lightClusterGridBuffer = nullptr;

    /// @brief Buffer with a single unsigned int of the count of active lights
    std::shared_ptr<ShaderStorageBuffer> m_globalLightCountBuffer = nullptr;

    /// @brief Buffer of active clusters for more efficient light rendering
    std::shared_ptr<ShaderStorageBuffer> m_activeClusterBuffer = nullptr;

    /// @brief Shader program for the cluster grid
    ShaderProgram* m_clusterGridShaderProgram = nullptr;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Static members
    /// @{

    /// @brief Cluster grid dimensions
    // FIXME: Don't hard-code this
    const unsigned int m_gridSizeX = 16;
    const unsigned int m_gridSizeY = 9;
    const unsigned int m_gridSizeZ = 24;

    /// @brief Max number of lights per tile
    const unsigned int m_maxLightsPerTile;

    /// @}
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}

#endif
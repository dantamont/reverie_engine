//////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LIGHT_SETTINGS_H
#define GB_LIGHT_SETTINGS_H

// QT

// Internal
#include "GbLight.h"
#include "../materials/GbTexture.h"
#include "../view/GbFrameBuffer.h"
#include "../buffers/GbBufferQueue.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer for clustered light rendering
#define LIGHT_SETTINGS_BUFFER_NAME QStringLiteral("LightSettingsBuffer")
#define NUM_SHADOWS_PER_LIGHT_TYPE 5
#define NUM_SHADOW_MAP_TEXTURES 3 // One for each light type

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class Transform;
struct ShadowInfo;
class ShaderProgram;
class RenderProjection;
class ShadowMap;
//class RenderContext;
class LightingSettings;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightSettings
class LightingSettings {
public:

    enum LightingModel {
        kPhong, // Most basic lighting model
        kBlinnPhong // More realistic than Phong
    };

    LightingSettings(RenderContext& context, size_t maxNumLights = 1024, size_t numKernelSamples = 64);
    ~LightingSettings();


    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Shadow textures
    std::array<std::shared_ptr<Texture>, 3>& shadowTextures() { return m_shadowMapTextures; }

    /// @brief Maximum number of lights allowed in scenario
    const unsigned int& maxNumLights() const { return m_maxLights; }

    /// @brief Light buffer
    ShaderStorageBufferQueue& lightBuffers() { return m_lightBuffers; }

    /// @brief Shadow buffer
    ShaderStorageBufferQueue& shadowBuffers() { return m_shadows; }

    /// @brief SSAO buffers
    ShaderStorageBuffer& ssaoBuffer() { return m_ssaoBuffer; }

    const std::shared_ptr<Texture>& noiseTexture() const {
        return m_noiseTexture;
    }
    int noiseSize() const {
        return m_noiseTexture->width();
    }
    int numKernelSamples() const {
        return m_numKernalSamples;
    }
    float ssaoBias() const {
        return m_ssaoBias;
    }
    float ssaoRadius() const {
        return m_ssaoRadius;
    }

    ShaderStorageBufferQueue& pointLightMatrixBuffers() { return m_pointLightMatrices; }

    FrameBuffer& pointLightFrameBuffer() { return m_pointLightFrameBuffer; }

    /// @brief Shadow maps
    std::vector<ShadowMap*>& shadowMaps() { return m_shadowMaps; }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Whether or not more shadows can be added for the specified light type
    bool canAddShadow(Light::LightType lightType, int* availableIndex = nullptr);
    void addShadow(ShadowMap* sm, size_t shadowIndex, size_t lightIndex);
    void removeShadow(ShadowMap* sm, size_t index);

    /// @brief Reserve an index for a light to be added, returning the index
    int reserveLightIndex();

    /// @brief Check that there aren't too many lights
    void checkLights() const;

    /// @brief Clear all light data
    void clearLights();

    /// @}

protected:

    friend class Light;
    friend class LightComponent;

    //---------------------------------------------------------------------------------------
    /// @name SSAO
    /// @{

    /// @brief Randomly generate a sample kernal for SSAO's unit hemisphere
    void generateSampleKernel();

    /// @brief Initialize noise texture
    void initializeNoiseTexture();

    /// @brief The number of samples to take per SSAO kernel
    size_t m_numKernalSamples;

    // Parameters for tweaking SSAO
    float m_ssaoRadius;
    float m_ssaoBias;

    /// @brief Repeating texture of random rotations (about view normal's z) for tiling over screen (to improve SSAO results)
    std::shared_ptr<Texture> m_noiseTexture;

    /// @brief Buffer containing SSAO data
    /// @details Contains kernel samples
    ShaderStorageBuffer m_ssaoBuffer;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Lights
    /// @{

    /// @brief Count of lights
    unsigned int m_lightCount = 0;

    /// @brief Indices of deleted lights in attribute lists
    std::vector<int> m_deletedIndices;

    /// @brief The lighting model to use
    LightingModel m_lightingModel = LightingModel::kBlinnPhong;;

    /// @brief Buffer of light data for reference in shaders
    ShaderStorageBufferQueue m_lightBuffers;

    /// @brief The max number of lights allowed
    const unsigned int m_maxLights;

    /// @ brief An empty, disabled light to cap of the lights list
    Light m_disabledLight;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Shadows
    /// @{

    /// @brief Indices of lights corresponding to each available shadow slot
    /// @details A negative index indicates no assigned light
    std::array<int, NUM_SHADOWS_PER_LIGHT_TYPE * NUM_SHADOW_MAP_TEXTURES> m_shadowIndices;

    /// @brief Buffer of shadow data for reference in shaders
    ShaderStorageBufferQueue m_shadows;

    /// @brief Buffer of point shadow map light (world to clip) matrices for reference in shaders
    ShaderStorageBufferQueue m_pointLightMatrices;

    /// @brief Framebuffer for point-light rendering
    FrameBuffer m_pointLightFrameBuffer;

    /// @brief Vector of all shadow maps in the settings
    std::vector<ShadowMap*> m_shadowMaps;

    /// @brief Shadow map textures
    std::array<std::shared_ptr<Texture>, 3> m_shadowMapTextures;

    /// @}

    RenderContext& m_context;
};


//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
}


#endif
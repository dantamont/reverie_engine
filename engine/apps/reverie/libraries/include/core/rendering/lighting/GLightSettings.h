#pragma once
// Internal
#include "GLight.h"
#include "core/rendering/materials/GTexture.h"
#include "core/rendering/shaders/GUniform.h"
#include "core/rendering/view/GFrameBuffer.h"
#include "core/rendering/buffers/GBufferQueue.h"

// Buffer for clustered light rendering
#define NUM_SHADOWS_PER_LIGHT_TYPE 5
#define NUM_SHADOW_MAP_TEXTURES 3 // One for each light type

namespace rev {

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

class CoreEngine;
struct ShadowInfo;
class ShaderProgram;
class RenderProjection;
class ShadowMap;
class LightingSettings;

/// @class LightSettings
class LightingSettings {
public:

    enum LightingModel {
        kPhong, // Most basic lighting model
        kBlinnPhong // More realistic than Phong
    };


    /// @brief Uniforms to be used in lighting UBO
    struct LightBufferUniforms {
        StrongTypeUniformData<Int32_t> m_lightCount;
        StrongTypeUniformData<Int32_t> m_lightingModel;
    };

    LightingSettings(RenderContext& context, uint32_t maxNumLights = 1024, uint32_t numKernelSamples = 64);
    ~LightingSettings();


    /// @name Public methods
    /// @{

    /// @brief Uniforms used to set valeus in the lighting UBO
    const LightBufferUniforms& lightBufferUniforms() const {
        return m_bufferUniforms;
    }

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
    uint32_t noiseSize() const {
        return m_noiseTexture->width();
    }
    uint32_t numKernelSamples() const {
        return m_numKernalSamples;
    }
    float ssaoBias() const {
        return m_ssaoBias;
    }
    float ssaoRadius() const {
        return m_ssaoRadius;
    }

    ShaderStorageBufferQueue& pointLightMatrixBuffers() { return m_pointLightMatrices; }
    ShaderStorageBufferQueue& pointLightAttributeBuffers() { return m_pointLightAttributes; }


    FrameBuffer& pointLightFrameBuffer() { return m_pointLightFrameBuffer; }

    /// @brief Shadow maps
    std::vector<ShadowMap*>& shadowMaps() { return m_shadowMaps; }

    /// @brief Whether or not more shadows can be added for the specified light type
    bool canAddShadow(Light::LightType lightType, int* availableIndex = nullptr);
    void addShadow(ShadowMap* sm, uint32_t shadowIndex, uint32_t lightIndex);
    void removeShadow(ShadowMap* sm, uint32_t index);

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

    /// @name SSAO
    /// @{

    /// @brief Randomly generate a sample kernal for SSAO's unit hemisphere
    void generateSampleKernel();

    /// @brief Initialize noise texture
    void initializeNoiseTexture();

    uint32_t m_numKernalSamples; ///< The number of samples to take per SSAO kernel
    float m_ssaoRadius; ///< Parameters for tweaking SSAO
    float m_ssaoBias; ///< Parameters for tweaking SSAO
    std::shared_ptr<Texture> m_noiseTexture; ///< Repeating texture of random rotations (about view normal's z) for tiling over screen (to improve SSAO results)
    ShaderStorageBuffer m_ssaoBuffer; ///< Buffer containing SSAO data. Contains kernel samples

    /// @}

    /// @name Lights
    /// @{

    /// @brief Update the uniforms in the lighting UBO
    void updateLightBufferUniforms();

    LightBufferUniforms m_bufferUniforms; ///< Uniforms to set in the lighting UBO
    unsigned int m_lightCount = 0; ///< Count of lights
    std::vector<int> m_deletedIndices; ///< @todo Replace with strict grow container. Indices of deleted lights in attribute lists
    LightingModel m_lightingModel = LightingModel::kBlinnPhong; ///< The lighting model to use
    ShaderStorageBufferQueue m_lightBuffers; ///< Buffer of light data for reference in shaders
    const unsigned int m_maxLights; ///< The max number of lights allowed
    Light m_disabledLight; ///< An empty, disabled light to cap of the lights list

    /// @}

    /// @name Shadows
    /// @{

    /// @details A negative index indicates no assigned light
    std::array<int, NUM_SHADOWS_PER_LIGHT_TYPE * NUM_SHADOW_MAP_TEXTURES> m_shadowIndices; ///< Indices of lights corresponding to each available shadow slot
    ShaderStorageBufferQueue m_shadows; ///< Buffer of shadow data for reference in shaders
    ShaderStorageBufferQueue m_pointLightMatrices; ///< Buffer of point shadow map light (world to clip) matrices for reference in shaders
    ShaderStorageBufferQueue m_pointLightAttributes; ///< Buffer of point shadow map light attributes for reference in shaders
    FrameBuffer m_pointLightFrameBuffer; ///< Framebuffer for point-light rendering
    std::vector<ShadowMap*> m_shadowMaps; ///< Vector of all shadow maps in the settings
    std::array<std::shared_ptr<Texture>, 3> m_shadowMapTextures; ///< Shadow map textures
    static ShadowInfo s_emptyShadowInfo; ///< Empty shadow info

    /// @}

    RenderContext& m_context;
};


// End namespaces
}

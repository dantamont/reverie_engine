#include "GLightSettings.h"
#include "GShadowMap.h"

#include "../materials/GCubeTexture.h"
#include "../../geometry/GMatrix.h"
#include "../../components/GTransformComponent.h"
#include "../../scene/GSceneObject.h"
#include "../../rendering/shaders/GShaderProgram.h"
#include "../../rendering/buffers/GUniformBufferObject.h"
#include "../../rendering/renderer/GRenderContext.h"

#include "../../utils/GRandom.h"
#include "../../utils/GInterpolation.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev{

LightingSettings::LightingSettings(RenderContext& context, size_t maxNumLights, size_t numKernelSamples) :
    m_context(context),
    m_maxLights(maxNumLights),
    m_numKernalSamples(numKernelSamples),
    m_lightBuffers(context, sizeof(Light) * maxNumLights),
    m_shadows(context, sizeof(ShadowInfo) * maxNumLights),
    m_ssaoBuffer(context, sizeof(Vector4) * numKernelSamples),
    m_ssaoBias(0.025f),
    m_ssaoRadius(0.5f),
    m_pointLightMatrices(context, sizeof(Matrix4x4g) * 6 * NUM_SHADOWS_PER_LIGHT_TYPE),
    m_pointLightAttributes(context, sizeof(BoundingSphereData) * NUM_SHADOWS_PER_LIGHT_TYPE),
    m_pointLightFrameBuffer(context.context(),
        AliasingType::kDefault,
        FrameBuffer::BufferAttachmentType::kTexture,
        FBO_FLOATING_POINT_TEX_FORMAT,
        4, // Not using MSAA, so doesn't matter
        0) // No color attachments needed
{
    // Disable cached disabled light
    m_disabledLight.disable();

    // Set bind points of light and shadow buffers for access in shader programs
    m_lightBuffers.setBindPoints(0);
    m_lightBuffers.setNames("LightBuffer");

    m_shadows.setBindPoints(8);
    m_shadows.setNames("ShadowBuffer");

    m_pointLightMatrices.setBindPoints(0);
    m_pointLightMatrices.setNames("PointLightMatrices");

    m_pointLightAttributes.setBindPoints(1);
    m_pointLightAttributes.setNames("PointLightAttributes");

    // Initialize shadow buffer values to -1 (no shadow map), and matrices to identity
    static ShadowInfo emptyInfo; // TODO: Needs to be static, but shouldn't be local
    emptyInfo.m_mapIndexBiasesFarClip = Vector4(-1, 0, 0, 0);

    for (size_t i = 0; i < maxNumLights; i++) {
        m_shadows.queueUpdate<ShadowInfo>(emptyInfo, i);
    }

    // Initialize array of shadow indices
    for (size_t i = 0; i < m_shadowIndices.size(); i++) {
        m_shadowIndices[i] = -1;
    }

    // Initialize shadow map textures, using fairly large texture map
    // TODO: Add widget for shadow map texture settings
    size_t maxSize = Texture::MaxDivisibleTextureSize();
    maxSize = std::min(maxSize, (size_t)4096);

    size_t numLayers = std::min(Texture::MaxNumTextureLayers(), size_t(NUM_SHADOWS_PER_LIGHT_TYPE));
    
    // FIXME: Either investigate why the geometry shader only updates the first point light,
    // or create a framebuffer/cubetexture for EACH point light, allowing for more than one. 

    // Create a texture array for each light type
    // TODO: Optimize with sparse textures, or with settings via widget
    // Point lights
    size_t pointMapSize = std::min((size_t)1024, maxSize);
    auto pointTexture = std::make_shared<CubeTexture>(
        pointMapSize,
        pointMapSize,
        4/*NUM_SHADOWS_PER_LIGHT_TYPE*/, // TODO: Dynamically check this hardware limit, see prepass_shadowmap.geom
        true,
        FBO_DEFAULT_DEPTH_PRECISION);
    pointTexture->postConstruction();
    m_shadowMapTextures[(int)Light::LightType::kPoint] = pointTexture;

    for (size_t i = 1; i < 3; i++) {
        // For directional (1) and spot lights (2)
        auto mapTexture = std::make_shared<Texture>(
            maxSize,
            maxSize,
            TextureTargetType::k2DArray,
            TextureUsageType::kNone,
            TextureFilter::kLinear, // Need nearest or linear for depth attachment
            TextureFilter::kLinear, // Need nearest or linear for depth attachment
            TextureWrapMode::kClampToBorder, // Want all coordinates outside of depth range to have a value of 1.0 (never in shadow)
            FBO_DEFAULT_DEPTH_PRECISION, // Could use kDepth32FStencil8X24, but don't need that precision
            numLayers); 
        mapTexture->setBorderColor(Color(std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f}));
        mapTexture->postConstruction();
        m_shadowMapTextures[i] = mapTexture;
    }

    // Initialize SSAO
    generateSampleKernel();
    initializeNoiseTexture();
    m_ssaoBuffer.setBindPoint(0);
}

LightingSettings::~LightingSettings()
{
    for (size_t i = 0; i < m_shadowMapTextures.size(); i++) {
        m_shadowMapTextures[i] = nullptr;
    }
}

void LightingSettings::clearLights()
{
    // Clear lights
    m_lightCount = 0;
    //m_lights.clear();
    m_lightBuffers.clear();
    m_deletedIndices.clear();


    // Clear shadows
    m_shadows.clear();
    m_shadowMaps.clear();
}

void LightingSettings::generateSampleKernel()
{
    // See "Normal-oriented hemisphere": https://learnopengl.com/Advanced-Lighting/SSAO
    std::vector<Vector4> kernelSamples; // extra entry for alignment
    kernelSamples.resize(m_numKernalSamples);
    std::array<Vector2, 4> ranges{ {
        {-1.0, 1.0}, { -1.0, 1.0 }, { 0.0, 1.0 }, {0.0, 0.0} } };
    Random::GetRandomVectors(m_numKernalSamples, 
        ranges,
        kernelSamples.data(),
        [](size_t index, size_t numSamples) {
        float scale = (float)index / numSamples;
        scale = Interpolation::lerp(0.1f, 1.0f, scale * scale);
        return scale;
    });
    m_ssaoBuffer.subData(kernelSamples.data(), 0, m_numKernalSamples);

    for (size_t i = 0; i < 10; i++) {
        Object().logInfo(QString(m_ssaoBuffer.data<Vector4>()[i]));
        m_ssaoBuffer.unmap();
    }
    //for (auto& vec : kernelSamples) {
    //    Object().logInfo(QString(vec));
    //}
}

void LightingSettings::initializeNoiseTexture()
{
    std::vector<Vector3> ssaoNoise;
    std::array<Vector2, 3> ranges{ {
        {-1.0, 1.0}, { -1.0, 1.0 }, { 0.0, 0.0 } } };
    Random::GetRandomVectors(16,
        ranges,
        ssaoNoise,
        [](size_t, size_t) {
        return 1;
    });

    //for (auto& vec : ssaoNoise) {
    //    Object().logInfo(QString(vec));
    //}

    m_noiseTexture = std::make_shared<Texture>(
        4,
        4,
        TextureTargetType::k2D,
        TextureUsageType::kNone,
        TextureFilter::kNearest, 
        TextureFilter::kNearest, 
        TextureWrapMode::kRepeat, // Want to repeat for tiling
        TextureFormat::kRGB16F);
    m_noiseTexture->postConstruction();
    m_noiseTexture->setData(ssaoNoise.data(), PixelFormat::kRGB, PixelType::kFloat32);
    
    // Testing data retrieval
    //std::array<Vector3, 16> outNoise;
    //m_noiseTexture->getData(outNoise.data(), PixelFormat::kRGB, PixelType::kFloat32);
}

bool LightingSettings::canAddShadow(Light::LightType lightType, int * availableIndex)
{
    size_t numLightsOfType = m_shadowMapTextures[(int)lightType]->depth();
    size_t startIdx = lightType * NUM_SHADOWS_PER_LIGHT_TYPE;
    size_t endIdx = startIdx + numLightsOfType;
    size_t idx;
    for (idx = startIdx; idx < endIdx; idx++) {
        if (m_shadowIndices[idx] < 0) {
            // Index is unassigned, so is available
            break;
        }
    }
    if (availableIndex) {
        *availableIndex = idx;
    }

    return idx < endIdx;
}

void LightingSettings::addShadow(ShadowMap* sm, size_t index, size_t lightIndex)
{
    m_shadowIndices[index] = lightIndex;
    m_shadowMaps.push_back(sm);
}

void LightingSettings::removeShadow(ShadowMap* sm, size_t index)
{
    m_shadowIndices[index] = -1;

    if (!sm) {
        throw("Error, shadow map does not exist");
    }

    auto it = std::find_if(m_shadowMaps.begin(), m_shadowMaps.end(),
        [&](ShadowMap* map) {
        return map->getUuid() == sm->getUuid();
    });

    if (it == m_shadowMaps.end()) {
        throw("Error, shadow map not found");
    }

    m_shadowMaps.erase(it);
}

int LightingSettings::reserveLightIndex()
{
    int index;
    if (m_deletedIndices.size() > 0) {
        index = m_deletedIndices.back();
        m_deletedIndices.pop_back();
    }
    else {
        index = m_lightCount;
        m_lightCount++;
        if (m_lightCount < m_maxLights) {
            // Disabled last light is ESSENTIAL for light culling shader
            m_lightBuffers.queueUpdate<Light>(m_disabledLight, m_lightCount);
        }
    }

    return index;
}

void LightingSettings::checkLights() const
{
    if (m_lightCount > m_maxLights) {
        throw("Error, exceeded max number of allowable lights");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////  
// End namespaces
}
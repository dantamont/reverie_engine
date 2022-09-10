#include "core/canvas/GSprite.h"

#include "core/GCoreEngine.h"
#include <core/processes/GProcessManager.h>
#include <core/processes/GSpriteAnimationProcess.h>
#include "core/resource/GResourceHandle.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "fortress/json/GJson.h"
#include "core/components/GCanvasComponent.h"
#include "core/rendering/renderer/GRenderSettings.h"
#include "core/scene/GSceneObject.h"
#include "core/scene/GScene.h"
#include "core/rendering/shaders/GUniformContainer.h"

#include "fortress/image/GTexturePacker.h"

namespace rev {

void SpriteAnimation::updateFrame(Sprite* sprite, float timeInSec)
{
    bool done;
    uint32_t frame = getAnimationFrame(sprite, timeInSec, m_playbackMode, done);
    if (!done) {
        m_currentFrame.store(frame + m_offset);
    }
}

void to_json(json& orJson, const SpriteAnimation& korObject)
{
    orJson["frameSec"] = korObject.m_frameSec;
    orJson["playbackMode"] = (int)korObject.m_playbackMode;
    orJson["frameCount"] = (int)korObject.m_frameCount;
    orJson["maxPlays"] = korObject.m_maxPlays;
    orJson["frameOffset"] = korObject.m_offset;
}

void from_json(const json& korJson, SpriteAnimation& orObject)
{
    korJson["frameSec"].get_to(orObject.m_frameSec);
    orObject.m_playbackMode = (SpritePlaybackMode)korJson["playbackMode"];
    korJson["frameCount"].get_to(orObject.m_frameCount);
    korJson["maxPlays"].get_to(orObject.m_maxPlays);
    orObject.m_offset = korJson.value("frameOffset", 0);
}

uint32_t SpriteAnimation::getAnimationFrame(Sprite* sprite, float timeInSec, SpritePlaybackMode mode, bool& donePlaying)
{
    // Get timing info
    float animationTime = getAnimationTime(sprite, timeInSec, mode, donePlaying);

    // Get frame
    double animationTickTime = animationTime / m_frameSec;
    uint32_t frame = (uint32_t)animationTickTime;

    return frame;
}

float SpriteAnimation::getAnimationTime(Sprite* sprite, float timeInSec, SpritePlaybackMode mode, bool& donePlaying) const
{
    // Get timing info
    if (!sprite->materialHandle()) {
        return 0;
    }
    else if (!sprite->material()) {
        return 0;
    }

    const SpriteSheetInfo& info = sprite->material()->spriteInfo();
    uint32_t frameCount = (uint32_t)info.m_packedTextures.size();
    if (m_frameCount >= 0) {
        frameCount = std::min(frameCount, (uint32_t)m_frameCount);
    }
    float durationSec = m_frameSec * frameCount;
    float animationTime = fmod(timeInSec, durationSec);
    int playCount = int(timeInSec / durationSec);

    // Determine whether or not play count has been reached
    if (m_maxPlays > 0) {
        if (playCount >= m_maxPlays) {
            // If played more than max num allowable times, stop animating
            donePlaying = true;
            return animationTime;
        }
    }

    // Perform playback mode-specific functionality
    switch (mode) {
    case SpritePlaybackMode::kSingleShot: {
        if (playCount > 1) {
            donePlaying = true;
            return animationTime;
        }
        break;
    }
    case SpritePlaybackMode::kPingPong:
        if (playCount % 2 == 1) {
            // If is an odd play count, ping-pong the animation
            animationTime = durationSec - animationTime;
        }
        break;
    case SpritePlaybackMode::kLoop:
    default:
        break;
    }

    donePlaying = false;
    return animationTime;
}



// Sprite

Sprite::Sprite(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex) :
    Glyph(canvas, worldMatrixVec, worldMatrixIndex)
{
    // Give the sprite a default name
    static std::atomic<uint32_t> count(0);
    m_name = GString::Format("Sprite_%d", count.load()).c_str();
    count.fetch_add(1);

    // Obtain quad
    ResourceCache* cache = &ResourceCache::Instance();
    initializeMesh(cache);

    // Initialize render settings to enable blending and disable face culling
    m_renderSettings.addDefaultBlend();
    m_renderSettings.addSetting<CullFaceSetting>(false);
}

Sprite::~Sprite()
{
    // Abort the process for this animation
    if (m_animationProcess) {
        std::unique_lock lock(m_animationProcess->mutex());
        if (!m_animationProcess->isAborted()) {
            m_animationProcess->abort();
        }
    }
}

void to_json(json& orJson, const Sprite& korObject)
{
    ToJson<Glyph>(orJson, korObject);

    // Save members
    orJson["name"] = korObject.m_name.c_str();
    orJson["texCoords"] = korObject.m_texCoords;
    orJson["dims"] = korObject.m_texDimensions;
    orJson["spriteFlags"] = (int)korObject.m_flags;

    orJson["spriteAnimation"] = korObject.m_animation;

    if (korObject.m_matHandle) {
        orJson["material"] = korObject.m_matHandle->getName().c_str();
    }
}

void from_json(const json& korJson, Sprite& orObject)
{
    FromJson<Glyph>(korJson, orObject);

    if (orObject.m_animationProcess) {
        // Clear any current animation process
        orObject.removeAnimation();
    }

    // Load members
    orObject.m_name = korJson[JsonKeys::s_name].get_ref<const std::string&>().c_str();
    korJson["texCoords"].get_to(orObject.m_texCoords);
    korJson["dims"].get_to(orObject.m_texDimensions);
    orObject.initializeMesh(&ResourceCache::Instance());

    if (korJson.contains("spriteFlags")) {
        orObject.m_flags = SpriteFlags(korJson["spriteFlags"].get<Int32_t>());
    }

    if (korJson.contains("spriteAnimation")) {
        korJson["spriteAnimation"].get_to(orObject.m_animation);
        if (orObject.m_flags.testFlag(SpriteFlag::kAnimated)) {
            orObject.addAnimation();
        }
    }

    if (korJson.contains("material")) {
        // Set material resource handle
        GString materialName = korJson["material"].get_ref<const std::string&>().c_str();
        ResourceCache& cache = ResourceCache::Instance();
        orObject.m_matHandle = cache.getHandleWithName(materialName, EResourceType::eMaterial);
        if (!orObject.m_matHandle) {
            orObject.m_materialName = materialName;
        }
    }
}

void Sprite::addAnimation()
{
    m_flags.setFlag(SpriteFlag::kAnimated, true);
    CoreEngine* core = m_canvas->sceneObject()->scene()->engine();
    m_animationProcess = std::make_shared<SpriteAnimationProcess>(core, this);

    // Add process for this animation to the process manager queue
    core->processManager()->animationThread().attachProcess(m_animationProcess);
}

void Sprite::removeAnimation()
{
    m_flags.setFlag(SpriteFlag::kAnimated, false);
    m_animationProcess->abort();
    m_animationProcess = nullptr;
}

void Sprite::initializeMesh(ResourceCache* cache)
{
    Mesh* quad = cache->polygonCache()->getSquare();

    // Set mesh to be rectangle
    m_meshHandle = cache->getHandle(quad->handle()->getUuid());

    // Make sure that mesh has bounds
    assert(quad->objectBounds().boxData().isInitialized() && "Quad bounds uninitialized");
}

void Sprite::bindUniforms(ShaderProgram& shaderProgram)
{
    Glyph::bindUniforms(shaderProgram);
    RenderContext& context = m_canvas->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();

    /// @todo Better document what this uniform is actually doing. "textColor"
    // Set texture uniform
    //shaderProgram.setUniformValue(shaderProgram.uniformMappings().m_textColor, Vector3::Ones(), uc);
    UniformData& textColorUniformData = m_canvas->m_glyphUniforms.m_textColor[m_canvasIndices.m_textColor];
    shaderProgram.setUniformValue(
        shaderProgram.uniformMappings().m_textColor,
        textColorUniformData);

    // Make sure material is loaded
    if (!m_matHandle) {
        return;
    }

    if (Material* material = m_matHandle->resourceAs<Material>()) {

        bool isAnimated = m_flags.testFlag(SpriteFlag::kAnimated);
        if (isAnimated) {
            uint32_t currentFrame = m_animation.m_currentFrame.load();
            const PackedTextureInfo& spriteTex = material->spriteInfo().m_packedTextures[currentFrame];
            //Logger::LogDebug(std::to_string(currentFrame) + "\nOrigin: " + GString(spriteTex.m_origin).c_str());
            if (material->handle()->children().size()) {
                Texture* tex = material->handle()->children()[0]->resourceAs<Texture>();

                UniformData& textureOffsetUniform = m_canvas->m_glyphUniforms.m_textureOffset[m_canvasIndices.m_textureOffset];
                UniformData& textureScaleUniform = m_canvas->m_glyphUniforms.m_textureScale[m_canvasIndices.m_textureScale];

                /// @todo Update these values only when the sprite texture is loaded or changes
                // Since origin is given as pixels from top-left corner of each sprite, and size is in pixels, eed to convert
                Float32_t inverseWidth = 1.0F / (float)tex->width();
                Float32_t inverseHeight = 1.0F / (float)tex->height();
                Vector2 sampleOrigin((float)spriteTex.m_origin.x() * inverseWidth,
                    ((-(float)spriteTex.m_origin.y() - (float)spriteTex.m_size.y()) * inverseHeight));
                Vector2 sampleSize((float)spriteTex.m_size.x() * inverseWidth,
                    (float)spriteTex.m_size.y() * inverseHeight);
                
                textureOffsetUniform.setValue(sampleOrigin, uc);
                textureScaleUniform.setValue(sampleSize, uc);

                shaderProgram.setUniformValue(
                    shaderProgram.uniformMappings().m_texOffset,
                    textureOffsetUniform
                );
                shaderProgram.setUniformValue(
                    shaderProgram.uniformMappings().m_texScale,
                    textureScaleUniform
                );
            }
        }
    }
}

void Sprite::bindTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    // TODO: Check this in a signal
    if (!m_matHandle) {
        if (!m_materialName.isEmpty()) {
            ResourceCache* cache = &ResourceCache::Instance();
            m_matHandle = cache->getHandleWithName(m_materialName, EResourceType::eMaterial);
        }
    }

    if (m_matHandle) {
        m_matHandle->resourceAs<Material>()->bind(*shaderProgram, *context);
    }
}

void Sprite::releaseTextures(ShaderProgram* shaderProgram, RenderContext* context)
{
    G_UNUSED(shaderProgram);
    if (m_matHandle) {
        m_matHandle->resourceAs<Material>()->release(*context);
    }
}

void Sprite::drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings)
{
    G_UNUSED(settings);
    G_UNUSED(shaderProgram);


    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw quads for text
    mesh()->vertexData().drawGeometry(m_renderSettings.shapeMode(), 1);
}




} // End namespaces

#include "core/mixins/GRenderable.h"

#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "fortress/layer/framework/GFlags.h"
#include "core/GCoreEngine.h"
#include "core/scene/GScenario.h"

#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/materials/GMaterial.h"

#include "fortress/system/memory/GPointerTypes.h"

namespace rev {


// Shadable

void to_json(json& orJson, const Shadable& korObject)
{
    orJson["renderSettings"] = korObject.m_renderSettings;
}

void from_json(const json& korJson, Shadable& orObject)
{
    if (korJson.contains("renderSettings")) {
        korJson["renderSettings"].get_to(orObject.m_renderSettings);
    }

    if (korJson.contains("transparency")) {
        // Deprecated, now stored in render settings
        orObject.m_renderSettings.setTransparencyType((TransparencyRenderMode)korJson.at("transparency").get<Int32_t>());
    }
}

void Shadable::addUniform(const Uniform & uniform)
{
#ifdef DEBUG_MODE
    if (uniform.getId() == -1) {
        Logger::Throw("Error, uniform is invalid");
    }
#endif
    int idx = -1;
    if (hasUniform(uniform, &idx)) {
        m_uniforms[idx] = uniform;
    }
    else {
        m_uniforms.push_back(uniform);
    }
}

bool Shadable::hasUniform(Uint32_t uniformId, int * outIndex)
{
    auto iter = std::find_if(m_uniforms.begin(), m_uniforms.end(),
        [&](const Uniform& u) {
        return u.getId() == uniformId;
    });

    if (iter == m_uniforms.end()) {
        return false;
    }
    else {
        // Replace uniform if already set
        *outIndex = iter - m_uniforms.begin();
        return true;
    }
}

bool Shadable::hasUniform(const Uniform & uniform, int * outIndex)
{
    return hasUniform(uniform.getId(), outIndex);
}





Renderable::Renderable():
    m_meshHandle(nullptr)
{
}

Renderable::~Renderable()
{
    // Added this to make sure that mesh handles were removed, but results in a crash on scenario switch
    // I'm thinking it's not actually necessary, since the resource cache will clear itself
    //if (m_meshHandle->isRuntimeGenerated()) {
    //    // Make sure to delete mesh if unique to renderable. Won't delete if is a core resource
    //    ResourceCache::Instance().remove(m_meshHandle);
    //}
}
Mesh * Renderable::mesh() const
{
    if (m_meshHandle->isConstructed()) {
        return m_meshHandle->resourceAs<Mesh>();
    }
    else {
        return nullptr;
    }
}
Material * Renderable::material() const
{
    if (m_matHandle->isConstructed()) {
        return m_matHandle->resourceAs<Material>();
    }
    else {
        return nullptr;
    }
}

void Renderable::setUniforms(DrawCommand & drawCommand) const
{
    G_UNUSED(drawCommand);
}

void Renderable::draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings* settings, size_t drawFlags)
{    
    if (!shaderProgram.handle()->isConstructed()) return;

    preDraw();

#ifdef DEBUG_MODE
    printError("Error in predraw");
#endif

    // Apply render settings
    Flags<RenderableIgnoreFlag> flags = Flags<RenderableIgnoreFlag>(drawFlags);
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreSettings)) {
        if (settings) {
            settings->bind(*context);
        }
        else {
            m_renderSettings.bind(*context);
        }
    }
#ifdef DEBUG_MODE
    else {
        int test = 0;
        test;
    }
#endif

#ifdef DEBUG_MODE
    printError("Error initializing render settings for renderable");
#endif

    // Bind shader
    shaderProgram.bind();

#ifdef DEBUG_MODE
    printError("Error binding shader for renderable");
#endif

    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreTextures)) {
        // Bind texture (note that this doesn't need to come before uniforms are set)
        /// \see https://computergraphics.stackexchange.com/questions/5063/send-texture-to-shader
        bindTextures(&shaderProgram, context);
    }

#ifdef DEBUG_MODE
    printError("Error binding textures for renderable");
#endif

    // Set uniforms
    bool ignoreMismatch = flags.testFlag(RenderableIgnoreFlag::kIgnoreUniformMismatch);
    const UniformContainer& uc = context->uniformContainer();
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreUniforms)) {
        bindUniforms(shaderProgram);
        shaderProgram.updateUniforms(uc, ignoreMismatch);
    }

#ifdef DEBUG_MODE
    printError("Error setting uniforms for renderable");
#endif

    // Draw primitives for text
    // If additional settings are specified, these may override the base settings
    drawGeometry(shaderProgram, settings? settings: &m_renderSettings);

#ifdef DEBUG_MODE
    printError("Error drawing renderable");
#endif

    // Release textures
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreTextures)) {
        releaseTextures(&shaderProgram, context);
    }

#ifdef DEBUG_MODE
    printError("Error releasing renderable textures");
#endif

    // (optionally) Restore uniform values
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreUniforms)) {
        releaseUniforms(shaderProgram);
    }

#ifdef DEBUG_MODE
    printError("Error restoring renderable uniforms");
#endif

    // Restore render settings
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreSettings)) {
        if (settings) {
            settings->release(*context);
        }
        else {
            m_renderSettings.release(*context);
        }
    }

#ifdef DEBUG_MODE
    printError("Error drawing renderable");
#endif
}

void Renderable::reload()
{
    //verify();
}
//
//void Renderable::verify() const
//{
//    // Ensure that object has mesh and bounds on reload
//    if (!m_meshHandle) {
//        Logger::Throw("Error, renderable has no geometry associated with it");
//    }
//
//    if (!m_meshHandle->resourceAs<Mesh>()->objectBounds().count()) {
//        Logger::Throw("Error, renderable geometry needs bounds");
//    }
//}

void Renderable::addWorldBounds(const TransformInterface& world, BoundingBoxes& outBounds) const
{
    /// @todo Move this check to the actual resourceAs routine
    if (m_meshHandle->isLoading()) {
        return;
    }

    auto mesh = m_meshHandle->resourceAs<Mesh>();
    if (!mesh) {
#ifdef DEBUG_MODE
        Logger::LogWarning("Warning, no mesh found for renderable");
#endif
        return;
    }

    outBounds.geometry().emplace_back();
    if (m_meshHandle->getName().contains("sponza_326")) {
        mesh->objectBounds().recalculateBounds(world, outBounds.geometry().back());
    }
    else {
        mesh->objectBounds().recalculateBounds(world, outBounds.geometry().back());
    }
}

void to_json(json& orJson, const Renderable& korObject)
{
    ToJson<Shadable>(orJson, korObject);
}

void from_json(const json& korJson, Renderable& orObject)
{
    FromJson<Shadable>(korJson, orObject);
}


void Renderable::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const Uniform& uniform : m_uniforms) {
        shaderProgram.setUniformValue(uniform);
    }
}

void Renderable::releaseUniforms(ShaderProgram& shaderProgram)
{
    G_UNUSED(shaderProgram);
}

void Renderable::bindTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    G_UNUSED(shaderProgram);
    G_UNUSED(context);
}

void Renderable::releaseTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    G_UNUSED(shaderProgram);
    G_UNUSED(context);
}

void Renderable::drawGeometry(ShaderProgram&, RenderSettings*)
{
}

void Renderable::printError(const GStringView & errorStr)
{
    bool error = gl::OpenGLFunctions::printGLError(errorStr);
    if (error) {
        qDebug() << QStringLiteral("DO NOT IGNORE ERROR");
#ifdef DEBUG_MODE
        Logger::Throw("Bad error time");
#endif
    }
}

void Renderable::initializeEmptyMesh(ResourceCache& cache, ResourceBehaviorFlags flags)
{
    // If a handle already exists, throw error
    if (m_meshHandle) {
        Logger::Throw("Error, renderable already has a mesh.");
    }

    // Create handle for material
    static size_t count = 0;
    m_meshHandle = ResourceHandle::Create(cache.engine(),
        EResourceType::eMesh,
        flags);
    m_meshHandle->setName("renderable_" + GString::FromNumber(count));
    count++;

    // Create mesh
    m_meshHandle->setRuntimeGenerated(true);
    m_meshHandle->setUnsaved(true); // Don't want this getting reloaded
    m_meshHandle->setIsLoading(true);
    auto mesh = std::make_unique<Mesh>();
    m_meshHandle->setResource(std::move(mesh), false);
    m_meshHandle->setConstructed(true);

    // Update any widgets
    if (!flags.testFlag(ResourceBehaviorFlag::kHidden)) {
        cache.m_resourceAdded.emitForAll(m_meshHandle->getUuid());
    }
}




} // End namespaces

#include "GRenderable.h"

#include "../GCoreEngine.h"
#include "../resource/GResourceCache.h"
#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../rendering/renderer/GMainRenderer.h"
#include "../containers/GFlags.h"
#include "../GCoreEngine.h"
#include "../scene/GScenario.h"

#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/renderer/GRenderCommand.h"
#include "../rendering/renderer/GRenderContext.h"
#include "../rendering/geometry/GMesh.h"
#include "../rendering/materials/GMaterial.h"

#include "../utils/GMemoryManager.h"

namespace rev {

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shadable
////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shadable::asJson(const SerializationContext& context) const
{
    QJsonObject object;

    object.insert("renderSettings", m_renderSettings.asJson());

    //object.insert("tranparency", (int)m_transparencyType);

    return object;
}

void Shadable::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();

    if (object.contains("renderSettings"))
        m_renderSettings.loadFromJson(object["renderSettings"]);

    if (object.contains("transparency")) {
        // Deprecated, now stored in render settings
        m_renderSettings.setTransparencyType((TransparencyRenderMode)object["transparency"].toInt());
    }
}

void Shadable::addUniform(const Uniform & uniform)
{
#ifdef DEBUG_MODE
    if (uniform.getName().isEmpty()) {
        throw("Error, uniform has no name");
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

bool Shadable::hasUniform(const GStringView & uniformName, int * outIndex)
{
    auto iter = std::find_if(m_uniforms.begin(), m_uniforms.end(),
        [&](const Uniform& u) {
        return u.getName() == uniformName;
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
    return hasUniform(uniform.getName(), outIndex);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderable
////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    //    m_meshHandle->engine()->resourceCache()->remove(m_meshHandle);
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
    Q_UNUSED(drawCommand);
}

void Renderable::draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings* settings, size_t drawFlags)
{    
    if (!shaderProgram.handle()->isConstructed()) return;

    preDraw();

#ifdef DEBUG_MODE
    printError("Error in predraw");
#endif

    // Apply render settings
    Flags<RenderableIgnoreFlag> flags = Flags<RenderableIgnoreFlag>::toQFlags(drawFlags);
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
        // See: https://computergraphics.stackexchange.com/questions/5063/send-texture-to-shader
        bindTextures(&shaderProgram, context);
    }

#ifdef DEBUG_MODE
    printError("Error binding textures for renderable");
#endif

    // Set uniforms
    bool ignoreMismatch = flags.testFlag(RenderableIgnoreFlag::kIgnoreUniformMismatch);
    if (!flags.testFlag(RenderableIgnoreFlag::kIgnoreUniforms)) {
        bindUniforms(shaderProgram);
        shaderProgram.updateUniforms(ignoreMismatch);
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
//        throw("Error, renderable has no geometry associated with it");
//    }
//
//    if (!m_meshHandle->resourceAs<Mesh>()->objectBounds().count()) {
//        throw("Error, renderable geometry needs bounds");
//    }
//}

void Renderable::addWorldBounds(const Transform & world, BoundingBoxes& outBounds) const
{
    auto mesh = m_meshHandle->resourceAs<Mesh>();
    if (!mesh) {
#ifdef DEBUG_MODE
        Logger::LogWarning("Warning, no mesh found for renderable");
#endif
        return;
    }

    outBounds.geometry().emplace_back();
    mesh->objectBounds().recalculateBounds(world, outBounds.geometry().back());
}

QJsonValue Renderable::asJson(const SerializationContext& context) const
{
    return Shadable::asJson(context);
}

void Renderable::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Shadable::loadFromJson(json, context);
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
    Q_UNUSED(shaderProgram)
}

void Renderable::bindTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);
}

void Renderable::releaseTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);
}

void Renderable::printError(const GStringView & errorStr)
{
    bool error = GL::OpenGLFunctions::printGLError(errorStr);
    if (error) {
        qDebug() << QStringLiteral("DO NOT IGNORE ERROR");
#ifdef DEBUG_MODE
        throw("Bad error time");
#endif
    }
}

void Renderable::initializeEmptyMesh(ResourceCache& cache, ResourceBehaviorFlags flags)
{
    // If a handle already exists, throw error
    if (m_meshHandle) {
        throw("Error, renderable already has a mesh.");
    }

    // Create handle for material
    static size_t count = 0;
    m_meshHandle = ResourceHandle::create(cache.engine(),
        ResourceType::kMesh,
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
    emit cache.resourceAdded(m_meshHandle->getUuid());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

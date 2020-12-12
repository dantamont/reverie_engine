#include "GbRenderable.h"

#include "../GbCoreEngine.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../rendering/renderer/GbMainRenderer.h"
#include "../containers/GbFlags.h"
#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"

#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"
#include "../rendering/renderer/GbRenderContext.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Shadable
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shadable::asJson() const
{
    QJsonObject object;

    object.insert("renderSettings", m_renderSettings.asJson());

    object.insert("tranparency", (int)m_transparencyType);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shadable::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    QJsonObject object = json.toObject();

    if (object.contains("renderSettings"))
        m_renderSettings.loadFromJson(object["renderSettings"]);

    if (object.contains("transparency")) {
        m_transparencyType = (TransparencyType)object["transparency"].toInt();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
bool Shadable::hasUniform(const Uniform & uniform, int * outIndex)
{
    return hasUniform(uniform.getName(), outIndex);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderable
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSize Renderable::screenDimensions()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    return screenGeometry.size();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector2 Renderable::screenDimensionsVec()
{
    QSize size = screenDimensions();
    return Vector2(size.width(), size.height());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Renderable::screenDPI()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInch();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Renderable::screenDPIX()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchX();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Renderable::screenDPIY()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    return screen->logicalDotsPerInchY();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Renderable::Renderable()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::draw(ShaderProgram& shaderProgram, RenderContext* context, RenderSettings* settings, size_t drawFlags)
{    
    if (!shaderProgram.handle()->isConstructed()) return;

    preDraw();

#ifdef DEBUG_MODE
    printError("Error in predraw");
#endif

    // Apply render settings
    QFlags<RenderPassFlag> flags = Flags::toFlags<RenderPassFlag>(drawFlags);
    if (!flags.testFlag(RenderPassFlag::kIgnoreSettings)) {
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

    if (!flags.testFlag(RenderPassFlag::kIgnoreTextures)) {
        // Bind texture (note that this doesn't need to come before uniforms are set)
        // See: https://computergraphics.stackexchange.com/questions/5063/send-texture-to-shader
        bindTextures(&shaderProgram, context);
    }

#ifdef DEBUG_MODE
    printError("Error binding textures for renderable");
#endif

    // Set uniforms
    bool ignoreMismatch = flags.testFlag(RenderPassFlag::kIgnoreUniformMismatch);
    if (!flags.testFlag(RenderPassFlag::kIgnoreUniforms)) {
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
    if (!flags.testFlag(RenderPassFlag::kIgnoreTextures)) {
        releaseTextures(&shaderProgram, context);
    }

#ifdef DEBUG_MODE
    printError("Error releasing renderable textures");
#endif

    // (optionally) Restore uniform values
    if (!flags.testFlag(RenderPassFlag::kIgnoreUniforms)) {
        releaseUniforms(shaderProgram);
    }

#ifdef DEBUG_MODE
    printError("Error restoring renderable uniforms");
#endif

    // Restore render settings
    if (!flags.testFlag(RenderPassFlag::kIgnoreSettings)) {
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
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Renderable::asJson() const
{
    return Shadable::asJson();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Shadable::loadFromJson(json, context);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const Uniform& uniform : m_uniforms) {
        shaderProgram.setUniformValue(uniform);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::bindTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::releaseTextures(ShaderProgram * shaderProgram, RenderContext * context)
{
    Q_UNUSED(shaderProgram);
    Q_UNUSED(context);
}
/////////////////////////////////////////////////////////////////////////////////////////////
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



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

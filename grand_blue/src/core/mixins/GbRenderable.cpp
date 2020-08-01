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

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
// Shadable
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Shadable::asJson() const
{
    QJsonObject object;

    // Cache uniforms used by the renderable
    QJsonObject uniforms;
    for (const auto& uniformPair : m_uniforms) {
        uniforms.insert(uniformPair.first, uniformPair.second.asJson());
    }
    object.insert("uniforms", uniforms);

    object.insert("renderSettings", m_renderSettings.asJson());

    object.insert("tranparency", (int)m_transparencyType);

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shadable::loadFromJson(const QJsonValue & json)
{
    // Load uniforms used by the renderable
    QJsonObject object = json.toObject();
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const QString& key : uniforms.keys()) {
        QJsonObject uniformObject = uniforms.value(key).toObject();
        if (!Map::HasKey(m_uniforms, key))
            Map::Emplace(m_uniforms, key, uniformObject);
        else
            m_uniforms[key] = std::move(Uniform(uniformObject));
    }

    if (object.contains("renderSettings"))
        m_renderSettings.loadFromJson(object["renderSettings"]);

    if (object.contains("transparency")) {
        m_transparencyType = (TransparencyType)object["transparency"].toInt();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Shadable::addUniform(const Uniform & uniform)
{
    m_uniforms[uniform.getName()] = uniform;
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
Vector2g Renderable::screenDimensionsVec()
{
    QSize size = screenDimensions();
    return Vector2g(size.width(), size.height());
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
void Renderable::draw(ShaderProgram& shaderProgram, RenderSettings* settings, size_t drawFlags)
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
            settings->bind();
        }
        m_renderSettings.bind();
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
        bindTextures(&shaderProgram);
    }

#ifdef DEBUG_MODE
    printError("Error binding textures for renderable");
#endif

    // Set uniforms
    if (!flags.testFlag(RenderPassFlag::kIgnoreUniforms)) {
        bindUniforms(shaderProgram);
        shaderProgram.updateUniforms();
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
        releaseTextures(&shaderProgram);
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
            settings->release();
        }
        m_renderSettings.release();
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
void Renderable::loadFromJson(const QJsonValue & json)
{
    Shadable::loadFromJson(json);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::bindUniforms(ShaderProgram& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const std::pair<QString, Uniform>& uniformPair : m_uniforms) {
        shaderProgram.setUniformValue(uniformPair.second);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::printError(const QString & errorStr)
{
    bool error = GL::OpenGLFunctions::printGLError(errorStr);
    if (error) {
        qDebug() << QStringLiteral("DO NOT IGNORE ERROR");
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

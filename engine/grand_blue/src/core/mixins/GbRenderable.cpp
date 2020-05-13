#include "GbRenderable.h"

#include "../GbCoreEngine.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../rendering/renderer/GbMainRenderer.h"

#include "../rendering/shaders/GbShaders.h"

namespace Gb {
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
void Renderable::addUniform(const Uniform & uniform)
{
    m_uniforms[uniform.getName()] = uniform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::draw(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings)
{    
    // Apply render settings
    if(settings){
        settings->bind();
    }
    m_renderSettings.bind();

#ifdef DEBUG_MODE
    //printError("Error initializing render settings for renderable");
#endif

    // Bind shader
    shaderProgram->bind();

#ifdef DEBUG_MODE
    //printError("Error binding shader for renderable");
#endif

    // Bind texture (note that this doesn't need to come before uniforms are set)
    // See: https://computergraphics.stackexchange.com/questions/5063/send-texture-to-shader
    bindTextures();

#ifdef DEBUG_MODE
    //printError("Error binding textures for renderable");
#endif

    // Set uniforms
    bindUniforms(shaderProgram);
    shaderProgram->updateUniforms();

#ifdef DEBUG_MODE
    //printError("Error setting uniforms for renderable");
#endif

    // Draw primitives for text
    // If additional settings are specified, these may override the base settings
    drawGeometry(shaderProgram, settings? settings: &m_renderSettings);

#ifdef DEBUG_MODE
    //printError("Error drawing renderable");
#endif

    // Release textures
    releaseTextures();

#ifdef DEBUG_MODE
    //printError("Error releasing renderable textures");
#endif

    // (optionally) Restore uniform values
    releaseUniforms(shaderProgram);

#ifdef DEBUG_MODE
    //printError("Error restoring renderable uniforms");
#endif

    // Restore render settings
    if (settings) {
        settings->release();
    }
    m_renderSettings.release();

#ifdef DEBUG_MODE
    printError("Error drawing renderable");
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Renderable::asJson() const
{
    QJsonObject object;

    // Cache uniforms used by the renderable
    QJsonObject uniforms;
    for (const auto& uniformPair : m_uniforms) {
        uniforms.insert(uniformPair.first, uniformPair.second.asJson());
    }
    object.insert("uniforms", uniforms);

    object.insert("renderSettings", m_renderSettings.asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::loadFromJson(const QJsonValue & json)
{
    // Load uniforms used by the renderable
    QJsonObject object = json.toObject();
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const QString& key : uniforms.keys()) {
        QJsonObject uniformObject = uniforms.value(key).toObject();
        Map::Emplace(m_uniforms, key, uniformObject);
    }

    if(object.contains("renderSettings"))
        m_renderSettings.loadFromJson(object["renderSettings"]);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    // Iterate through uniforms to update in shader program class
    for (const std::pair<QString, Uniform>& uniformPair : m_uniforms) {
        shaderProgram->setUniformValue(uniformPair.second);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderable::releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
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

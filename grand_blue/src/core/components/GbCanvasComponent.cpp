#include "GbCanvasComponent.h"

#include "../GbCoreEngine.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../rendering/renderer/GbMainRenderer.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../rendering/shaders/GbShaders.h"

#include "../canvas/GbIcon.h"
#include "../canvas/GbLabel.h"

#include "GbCamera.h"

namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent() :
    Component(kCanvas)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent(CoreEngine * core) :
    Component(core, kCanvas),
    m_renderProjection(core->widgetManager()->mainGLWidget())
{
    m_renderProjection.setProjectionType(RenderProjection::kOrthographic);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, kCanvas),
    m_renderProjection(object->engine()->widgetManager()->mainGLWidget())
{
    m_renderProjection.setProjectionType(RenderProjection::kOrthographic);
    //setSceneObject(sceneObject()); // performed by parent class
    sceneObject()->addComponent(this);

    sceneObject()->scene()->addCanvas(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Gb::CanvasComponent::~CanvasComponent()
{
    clear();
    if (sceneObject()) {
        if(scene())
            sceneObject()->scene()->removeCanvas(this);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::addGlyph(std::shared_ptr<Glyph> glyph)
{
    Vec::EmplaceBack(m_glyphs, glyph);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::removeGlyph(std::shared_ptr<Glyph> glyph)
{
    auto it = std::find_if(m_glyphs.begin(), m_glyphs.end(),
        [&](std::shared_ptr<Glyph> g) {
        return g->getUuid() == glyph->getUuid();
    });

    if (it != m_glyphs.end()) {
        m_glyphs.erase(it);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSize CanvasComponent::mainGLWidgetDimensions() const
{
    return m_engine->widgetManager()->mainGLWidget()->size();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::clear()
{
    m_glyphs.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::draw(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings)
{
    if (!m_isEnabled) return;
    Renderable::draw(shaderProgram, settings);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::enable()
{
    Component::enable();
}
//////////////// ///////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::disable()
{
    Component::disable();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CanvasComponent::asJson() const
{
    QJsonObject object = Component::asJson().toObject();
    QJsonValue renderable = Renderable::asJson();
    object.insert("renderable", renderable);

    object.insert("viewport", m_viewport.asJson());
    object.insert("renderProjection", m_renderProjection.asJson());
    //object.insert("viewMatrix", m_viewMatrix.asJson())

    // Cache uniforms used by the canvas
    QJsonObject uniforms;
    for (const auto& uniformPair : m_uniforms) {
        uniforms.insert(uniformPair.first, uniformPair.second.asJson());
    }
    object.insert("uniforms", uniforms);

    // Cache glyphs
    QJsonArray glyphs;
    for (const std::shared_ptr<Glyph>& glyph: m_glyphs) {
        glyphs.append(glyph->asJson());
    }
    object.insert("glyphs", glyphs);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::loadFromJson(const QJsonValue & json)
{
    // Delete any glyphs
    clear();

    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load renderable attributes
    if (object.contains("renderable")) {
        Renderable::loadFromJson(object["renderable"]);
    }

    // Load view settings    
    m_viewport.loadFromJson(object.value("viewport"));
    m_renderProjection.loadFromJson(object.value("renderProjection"));

    // Load uniforms used by the canvas
    const QJsonObject& uniforms = object["uniforms"].toObject();
    for (const QString& key : uniforms.keys()) {
        QJsonObject uniformObject = uniforms.value(key).toObject();
        Map::Emplace(m_uniforms, key, uniformObject);
    }

    // Load glyphs
    const QJsonArray& glyphs = object["glyphs"].toArray();
    for (const QJsonValue& glyphJson : glyphs) {
        QJsonObject glyphObject = glyphJson.toObject();
        Vec::EmplaceBack(m_glyphs, Glyph::createGlyph(this, glyphObject));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{    
    // Reload data if screen-size changed
    QSize dims = mainGLWidgetDimensions();
    if (dims != m_dimensions) {
        resize();
    }

    // TODO: Move this somewhere else
    // Set the viewport size for canvas rendering
    // TODO: Pass widget instead of assuming main widget
    View::GLWidget* widget = m_engine->widgetManager()->mainGLWidget();
    m_viewport.setGLViewport(*widget->renderer());

    Renderable::bindUniforms(shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::resize()
{
    m_dimensions = mainGLWidgetDimensions();
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        glyph->reload();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings * settings)
{
    Q_UNUSED(settings)
    // Draw each glyph
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        glyph->draw(shaderProgram);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g CanvasComponent::VIEW_MATRIX = Matrix4x4g();



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
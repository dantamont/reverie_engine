#include "GbCanvasComponent.h"

#include "../GbCoreEngine.h"
#include "../events/GbEventManager.h"
#include "../resource/GbResourceCache.h"
#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../rendering/renderer/GbMainRenderer.h"

#include "../scene/GbScene.h"
#include "../scene/GbScenario.h"
#include "../scene/GbSceneObject.h"

#include "../rendering/shaders/GbShaders.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../canvas/GbIcon.h"
#include "../canvas/GbLabel.h"

#include "GbShaderComponent.h"


namespace Gb {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent() :
    Component(ComponentType::kCanvas)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent(CoreEngine * core) :
    Component(core, ComponentType::kCanvas)
{
    core->widgetManager()->mainGLWidget()->addRenderProjection(&m_renderProjection);
    m_renderProjection.setProjectionType(RenderProjection::kOrthographic);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCanvas)
{
    object->engine()->widgetManager()->mainGLWidget()->addRenderProjection(&m_renderProjection);

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
        if (sceneObject()->scene()) {
            sceneObject()->scene()->removeCanvas(this);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::createDrawCommands(
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
    SceneCamera & camera, 
    ShaderProgram & shaderProgram,
    ShaderProgram* prepassProgram)
{
    if (!m_isEnabled) { return; }

    // Iterate through glyphs to generate draw commands
    // TODO: Perform frustum culling for world-text
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        auto command = std::make_shared<DrawCommand>(*glyph, shaderProgram, camera, prepassProgram);
        //bindUniforms(*command);
        command->setUniform(
            Uniform("worldMatrix", glyph->transform()->worldMatrix())); // for depth calculation
        command->renderSettings().overrideSettings(m_renderSettings); // may not be necessary
        command->renderSettings().overrideSettings(glyph->renderSettings());

        outDrawCommands.push_back(command);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::addGlyph(const std::shared_ptr<Glyph>& glyph)
{
    Vec::EmplaceBack(m_glyphs, glyph);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::removeGlyph(const Glyph & glyph)
{
    auto it = std::find_if(m_glyphs.begin(), m_glyphs.end(),
        [&](std::shared_ptr<Glyph> g) {
        return g->getUuid() == glyph.getUuid();
    });

    if (it != m_glyphs.end()) {
        m_glyphs.erase(it);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::removeGlyph(const std::shared_ptr<Glyph>& glyph)
{
    removeGlyph(*glyph);
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CanvasComponent::draw(ShaderProgram& shaderProgram, RenderSettings* settings)
//{
//    if (!m_isEnabled) return;
//    Renderable::draw(shaderProgram, settings);
//}
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
    QJsonValue renderable = Shadable::asJson();
    object.insert("renderable", renderable);

    object.insert("viewport", m_viewport.asJson());
    object.insert("renderProjection", m_renderProjection.asJson());
    //object.insert("viewMatrix", m_viewMatrix.asJson())

    // Cache uniforms used by the canvas
    QJsonObject uniforms;
    for (const Uniform& uniform : m_uniforms) {
        uniforms.insert(uniform.getName(), uniform.asJson());
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
void CanvasComponent::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Delete any glyphs
    clear();

    Component::loadFromJson(json);
    const QJsonObject& object = json.toObject();

    // Load renderable attributes
    if (object.contains("renderable")) {
        Shadable::loadFromJson(object["renderable"]);
    }

    // Load view settings    
    m_viewport.loadFromJson(object.value("viewport"));
    m_renderProjection.loadFromJson(object.value("renderProjection"));

    //// Load uniforms used by the canvas
    //const QJsonObject& uniforms = object["uniforms"].toObject();
    //for (const QString& key : uniforms.keys()) {
    //    QJsonObject uniformObject = uniforms.value(key).toObject();
    //    Map::Emplace(m_uniforms, key, uniformObject);
    //}

    // Load glyphs
    const QJsonArray& glyphs = object["glyphs"].toArray();
    for (const QJsonValue& glyphJson : glyphs) {
        QJsonObject glyphObject = glyphJson.toObject();
        Vec::EmplaceBack(m_glyphs, Glyph::createGlyph(this, glyphObject));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::resizeViewport()
{    
    // Reload data if screen-size changed
    // TODO: Move somewhere else, should only be called on resize
    QSize dims = mainGLWidgetDimensions();
    if (dims != m_dimensions) {
        resize();
    }

    // TODO: Associate a camera with the canvas and use that viewport instead
    // TODO: Move this somewhere else
    // Set the viewport size for canvas rendering
    // TODO: Pass widget instead of assuming main widget
    //View::GLWidget* widget = m_engine->widgetManager()->mainGLWidget();
    //m_viewport.setGLViewport(*widget->renderer());

    //Renderable::bindUniforms(shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::resize()
{
    m_dimensions = mainGLWidgetDimensions();
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        glyph->reload();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CanvasComponent::drawGeometry(ShaderProgram& shaderProgram, RenderSettings * settings)
//{
//    Q_UNUSED(settings)
//    // Draw each glyph
//    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
//        glyph->draw(shaderProgram);
//    }
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::addRequiredComponents()
{
    if (!sceneObject()->hasComponent(Component::ComponentType::kShader)) {
        // Add shader component to the scene object if not already a member
        auto shaderComp = new ShaderComponent(sceneObject());
        shaderComp->addRequiredComponents();

        // Create text preset and add to shader component
        bool created;
        std::shared_ptr<ShaderPreset> textPreset = 
            m_engine->scenario()->settings().getShaderPreset(TEXT_SHADER_NAME, created);
        
        if (created) {
            auto textShader = m_engine->resourceCache()->getHandleWithName(
                TEXT_SHADER_NAME, Resource::kShaderProgram)->resourceAs<ShaderProgram>();
            textPreset->setShaderProgram(textShader);
        }

        // Set text preset for shader component
        shaderComp->setShaderPreset(textPreset);

        // Emit signal to display widget for shader component
        emit m_engine->eventManager()->addedComponent(shaderComp->getUuid(),
            (int)Component::ComponentType::kShader, 
            sceneObject()->getUuid());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Matrix4x4g CanvasComponent::VIEW_MATRIX = Matrix4x4g();



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
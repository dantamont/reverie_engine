#include "GCanvasComponent.h"

#include "../GCoreEngine.h"
#include "../events/GEventManager.h"
#include "../resource/GResourceCache.h"
#include "../../view/GWidgetManager.h"
#include "../../view/GL/GGLWidget.h"
#include "../rendering/renderer/GMainRenderer.h"

#include "../scene/GScene.h"
#include "../scene/GScenario.h"
#include "../scene/GSceneObject.h"

#include "../rendering/shaders/GShaderProgram.h"
#include "../rendering/renderer/GRenderCommand.h"

#include "../canvas/GIcon.h"
#include "../canvas/GLabel.h"

#include "GShaderComponent.h"


namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent() :
    Component(ComponentType::kCanvas)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent::CanvasComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCanvas),
    m_glyphMode(GlyphMode::kGUI),
    m_flags(0)
{
    //object->engine()->widgetManager()->mainGLWidget()->addRenderProjection(&m_renderProjection);
    //m_renderProjection.setProjectionType(RenderProjection::kOrthographic);

    sceneObject()->addComponent(this);
    sceneObject()->scene()->addCanvas(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::CanvasComponent::~CanvasComponent()
{
    clear();
    if (!scene()) {
        logError("Error, no scene for canvas component");
    }
    scene()->removeCanvas(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::setGlyphMode(GlyphMode mode)
{
    // Set preset based on new mode
    m_glyphMode = mode;
    bool created;
    GString presetName;
    std::shared_ptr<ShaderPreset> presetShader;
    switch (m_glyphMode) {
    case GlyphMode::kGUI:
        presetName = "canvas_gui";
        break;
    case GlyphMode::kBillboard:
        presetName = "canvas_billboard";
        break;
    default:
        throw("Glyph mode unrecognized");
        break;
    }

    std::shared_ptr<ShaderPreset> canvasPreset =
        sceneObject()->scene()->engine()->scenario()->settings().getShaderPreset(presetName, created);

    if (created) {
        throw("Error, preset should already exist");
    }


    // Set preset for shader component
    sceneObject()->hasComponent<ShaderComponent>(ComponentType::kShader)->setShaderPreset(canvasPreset);

    // TODO: Lock shader component to the user using ComponentFlag::kLocked
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::createDrawCommands(
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
    SceneCamera & camera, 
    ShaderProgram & shaderProgram,
    ShaderProgram* prepassProgram)
{
    if (!isEnabled()) { return; }

    // Iterate through glyphs to generate draw commands
    // TODO: Perform frustum culling for world-text
    int soId = sceneObject() ? sceneObject()->id() : (int)RenderObjectId::kDebug;
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        auto command = std::make_shared<DrawCommand>(*glyph, shaderProgram, camera, soId, prepassProgram);
        //bindUniforms(*command);
        command->addUniform(
            Uniform("worldMatrix", glyph->transform().worldMatrix())); // for depth calculation
        command->renderSettings().overrideSettings(glyph->renderSettings());

        // All glyphs have deferred geometry, since in GUI mode, their positioning is entirely shader-determined,
        // and in billboard mode, their size and orientation may be determined by the camera rendering them
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry);
        outDrawCommands.push_back(command);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::addGlyph(const std::shared_ptr<Glyph>& glyph, bool setParent)
{
    if (!m_deletedGlyphIndices.size()) {
        // Append to end of glyphs vector
        Vec::EmplaceBack(m_glyphs, glyph);
    }
    else {
        // Add at the index of a previously deleted glyph
        m_glyphs[m_deletedGlyphIndices.back()] = glyph;
        m_deletedGlyphIndices.pop_back();
    }

    if (setParent) {
        // Set canvas transform as parent
        glyph->setParent(this);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::addGlyph(const std::shared_ptr<Glyph>& glyph, const std::shared_ptr<Glyph>& parent)
{
    addGlyph(glyph, false);
    glyph->setParent(getIndex(*parent));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::removeGlyph(const Glyph & glyph)
{
    size_t idx = getIndex(glyph);
    m_glyphs[idx] = nullptr;
    m_deletedGlyphIndices.push_back(idx);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::removeGlyph(const std::shared_ptr<Glyph>& glyph)
{
    removeGlyph(*glyph);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QSize CanvasComponent::mainGLWidgetDimensions() const
{
    return sceneObject()->scene()->engine()->widgetManager()->mainGLWidget()->size();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponent::clear()
{
    // Need to clear transform parents because otherwise it's a deletion mess
    for (const std::shared_ptr<Glyph> glyph : m_glyphs) {
        glyph->transform().clearParent(false);
    }
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
QJsonValue CanvasComponent::asJson(const SerializationContext& context) const
{
    QJsonObject object = Component::asJson(context).toObject();
    //object.insert("renderProjection", m_renderProjection.asJson());

    // Get flags
    if (m_glyphMode == GlyphMode::kBillboard) {
        object.insert("faceCamera", facesCamera());
        object.insert("scaleWithDistance", scalesWithDistance());
        object.insert("onTop", alwaysOnTop());
    }

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

    // Load glyph mode
    m_glyphMode = GlyphMode(object.value("mode").toInt(0));

    // Load view settings    
    //m_renderProjection.loadFromJson(object.value("renderProjection"));

    // Load flags
    if (m_glyphMode == GlyphMode::kBillboard) {
        setFaceCamera(object["faceCamera"].toBool(false));
        setScaleWithDistance(object["scaleWithDistance"].toBool(true));
        setOnTop(object["onTop"].toBool(false));
    }

    // Load glyphs
    const QJsonArray& glyphs = object["glyphs"].toArray();
    for (const QJsonValue& glyphJson : glyphs) {
        QJsonObject glyphObject = glyphJson.toObject();
        int parentIndex = glyphObject["parent"].toInt(-1);
        bool hasParent = parentIndex >= 0;
        if (hasParent) {
            // Parent CANNOT be a reference to a pointer, it becomes invalidated on addGlyph
            std::shared_ptr<Glyph> parent = m_glyphs[parentIndex];
            addGlyph(Glyph::createGlyph(this, glyphObject), parent);
        }
        else {
            addGlyph(Glyph::createGlyph(this, glyphObject));
        }
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
    //View::GLWidget* widget = sceneObject()->scene()->engine()->widgetManager()->mainGLWidget();
    //m_viewport.setGLViewport(*widget->renderer());

    //Renderable::bindUniforms(shaderProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t CanvasComponent::getIndex(const Glyph & glyph) const
{
    auto it = std::find_if(m_glyphs.begin(), m_glyphs.end(),
        [&](const std::shared_ptr<Glyph>& g) {
        return g->getUuid() == glyph.getUuid();
    });

    if (it == m_glyphs.end()) {
        throw("Error, glyph not in vector");
    }

    return it - m_glyphs.begin();
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
    if (!sceneObject()->hasComponent(ComponentType::kShader)) {
        // Add shader component to the scene object if not already a member
        auto shaderComp = new ShaderComponent(sceneObject());
        shaderComp->addRequiredComponents();

        // Create text preset and add to shader component
        std::shared_ptr<ShaderPreset> canvasPreset = ShaderPreset::GetBuiltin("canvas_gui");
        
        // Set canvas preset for shader component
        shaderComp->setShaderPreset(canvasPreset);

        // Emit signal to display widget for shader component
        emit sceneObject()->scene()->engine()->eventManager()->addedComponent(shaderComp->getUuid(),
            (int)ComponentType::kShader, 
            sceneObject()->id());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing
#include "core/components/GCanvasComponent.h"

#include "core/GCoreEngine.h"
#include "core/events/GEventManager.h"
#include "core/resource/GResourceCache.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "core/layer/view/widgets/graphics/GGLWidget.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"

#include "core/scene/GScene.h"
#include "core/scene/GScenario.h"
#include "core/scene/GSceneObject.h"

#include "core/rendering/shaders/GUniformContainer.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/renderer/GRenderCommand.h"

#include "core/canvas/GIcon.h"
#include "core/canvas/GLabel.h"

#include "core/components/GShaderComponent.h"


namespace rev {

CanvasComponent::CanvasComponent() :
    Component(ComponentType::kCanvas)
{
}

CanvasComponent::CanvasComponent(const std::shared_ptr<SceneObject>& object) :
    Component(object, ComponentType::kCanvas),
    m_glyphMode(GlyphMode::kGUI),
    m_flags(0)
{
    // Initialize buffer uniforms
    static Matrix4x4 s_orthoProjection = RenderProjection::Orthographic(-1.0f, 1.0f, -1.0f, 1.0f);
    static Matrix4x4 s_identityViewMatrix = Matrix4x4::Identity();
    RenderContext& context = object->scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    m_orthoProjectionBufferUniformData.setValue(s_orthoProjection, uc);
    m_viewMatrixBufferUniformData.setValue(s_identityViewMatrix, uc);
    
    // Initialize uniforms
    const std::shared_ptr<CameraUbo>& cameraBuffer = Ubo::GetCameraBuffer();
    const Matrix4x4g& projectionMatrix = cameraBuffer->getBufferUniformValue<Matrix4x4>("projectionMatrix");
    Matrix3x3 inversedPerspectiveScale = Matrix3x3(projectionMatrix).inversed();
    m_glyphUniforms.m_perspectiveInverseScale.setValue(inversedPerspectiveScale, uc);
    m_glyphUniforms.m_onTop.setValue(alwaysOnTop(), uc);
    m_glyphUniforms.m_faceCamera.setValue(facesCamera(), uc);
    m_glyphUniforms.m_scaleWithDistance.setValue(scalesWithDistance(), uc);

    for (Int32_t i = 0; i < m_glyphUniforms.m_textureUnits.size(); i++) {
        // Simply use texture unit (0) for everything until other texture types are implemented
        m_glyphUniforms.m_textureUnits[i].setValue(0, uc);
    }

    sceneObject()->setComponent(this);
    sceneObject()->scene()->addCanvas(this);
}

rev::CanvasComponent::~CanvasComponent()
{
    clear();
    if (!scene()) {
        Logger::LogError("Error, no scene for canvas component");
    }
    scene()->removeCanvas(this);
}

Glyph* CanvasComponent::getGlyph(const Uuid& uuid) const
{
    auto iter = std::find_if(m_glyphs.begin(), m_glyphs.end(),
        [&](const std::shared_ptr<Glyph>& glyph) {
            return glyph->getUuid() == uuid;
        }
    );

    if (iter != m_glyphs.end()) {
        return iter->get();
    }
    else {
        return nullptr;
    }
}

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
        Logger::Throw("Glyph mode unrecognized");
        break;
    }

    std::shared_ptr<ShaderPreset> canvasPreset =
        sceneObject()->scene()->engine()->scenario()->settings().getShaderPreset(presetName, created);

    if (created) {
        Logger::Throw("Error, preset should already exist");
    }


    // Set preset for shader component
    sceneObject()->getComponent<ShaderComponent>(ComponentType::kShader)->setShaderPreset(canvasPreset);

    // TODO: Lock shader component to the user using ComponentFlag::kLocked
}

void CanvasComponent::createDrawCommands(
    std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
    SceneCamera & camera, 
    ShaderProgram & shaderProgram,
    ShaderProgram* prepassProgram)
{
    if (!isEnabled()) { return; }

    // Iterate through glyphs to generate draw commands
    // TODO: Perform frustum culling for world-text
    RenderContext& context = scene()->engine()->openGlRenderer()->renderContext();
    UniformContainer& uc = context.uniformContainer();
    int soId = sceneObject() ? sceneObject()->id() : (int)RenderObjectId::kDebug;
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        if (!glyph) {
            // Need to check for deleted glyphs
            continue;
        }

        auto command = std::make_shared<DrawCommand>(*glyph, shaderProgram, uc, camera, soId, prepassProgram);
        const UniformData& uniformData = m_glyphUniforms.m_worldMatrix[glyph->canvasUniformIndices().m_worldMatrix];
        command->addUniform(
            uniformData,
            shaderProgram.uniformMappings().m_worldMatrix,
            prepassProgram ? prepassProgram->uniformMappings().m_worldMatrix : -1
        ); // for depth calculation
        command->renderSettings().overrideSettings(glyph->renderSettings());

        // All glyphs have deferred geometry, since in GUI mode, their positioning is entirely shader-determined,
        // and in billboard mode, their size and orientation may be determined by the camera rendering them
        command->setPassFlags(RenderablePassFlag::kDeferredGeometry);
        outDrawCommands.push_back(command);
    }
}

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

void CanvasComponent::addGlyph(const std::shared_ptr<Glyph>& glyph, const std::shared_ptr<Glyph>& parent)
{
    addGlyph(glyph, false);
    glyph->setParent(getIndex(*parent));
}

void CanvasComponent::removeGlyph(const Glyph & glyph)
{
    size_t idx = getIndex(glyph);
    m_glyphs[idx] = nullptr;
    m_deletedGlyphIndices.push_back(idx);
}

void CanvasComponent::removeGlyph(const std::shared_ptr<Glyph>& glyph)
{
    removeGlyph(*glyph);
}

QSize CanvasComponent::mainGLWidgetDimensions() const
{
    return sceneObject()->scene()->engine()->widgetManager()->mainGLWidget()->size();
}

void CanvasComponent::clear()
{
    // Need to clear transform parents because otherwise it's a deletion mess
    for (const std::shared_ptr<Glyph> glyph : m_glyphs) {
        glyph->transform().clearParent(false);
    }
    m_glyphs.clear();
}

//void CanvasComponent::draw(ShaderProgram& shaderProgram, RenderSettings* settings)
//{
//    if (!m_isEnabled) return;
//    Renderable::draw(shaderProgram, settings);
//}

void CanvasComponent::enable()
{
    Component::enable();
}
 
void CanvasComponent::disable()
{
    Component::disable();
}

void to_json(json& orJson, const CanvasComponent& korObject)
{
    ToJson<Component>(orJson, korObject);

    orJson["mode"] = static_cast<Int32_t>(korObject.m_glyphMode);

    // Get flags
    if (korObject.m_glyphMode == GlyphMode::kBillboard) {
        orJson["faceCamera"] = korObject.facesCamera();
        orJson["scaleWithDistance"] = korObject.scalesWithDistance();
        orJson["onTop"] = korObject.alwaysOnTop();
    }

    // Cache glyphs
    json glyphs = json::array();
    for (const std::shared_ptr<Glyph>& glyph: korObject.m_glyphs) {
        glyphs.push_back(glyph->asJson());
    }
    orJson["glyphs"] = glyphs;
}

void from_json(const json& korJson, CanvasComponent& orObject)
{
    // Delete any glyphs
    orObject.clear();

    FromJson<Component>(korJson, orObject);

    // Load glyph mode
    orObject.m_glyphMode = GlyphMode(korJson.value("mode", 0));

    // Load view settings    
    //m_renderProjection.loadFromJson(korJson.at("renderProjection"));

    // Load flags
    if (orObject.m_glyphMode == GlyphMode::kBillboard) {
        orObject.setFaceCamera(korJson.value("faceCamera", false));
        orObject.setScaleWithDistance(korJson.value("scaleWithDistance", true));
        orObject.setOnTop(korJson.value("onTop", false));
    }

    // Load glyphs
    const json& glyphs = korJson["glyphs"];
    for (const json& glyphJson : glyphs) {
        const json& glyphObject = glyphJson;
        int parentIndex = glyphObject.value("parent", -1);
        bool hasParent = parentIndex >= 0;
        if (hasParent) {
            // Parent CANNOT be a reference to a pointer, it becomes invalidated on addGlyph
            std::shared_ptr<Glyph> parent = orObject.m_glyphs[parentIndex];
            orObject.addGlyph(Glyph::CreateGlyph(&orObject, glyphObject), parent);
        }
        else {
            orObject.addGlyph(Glyph::CreateGlyph(&orObject, glyphObject));
        }
    }
}

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
    //GLWidget* widget = sceneObject()->scene()->engine()->widgetManager()->mainGLWidget();
    //m_viewport.setGLViewport(*widget->renderer());

    //Renderable::bindUniforms(shaderProgram);
}

uint32_t CanvasComponent::getIndex(const Glyph & glyph) const
{
    auto it = std::find_if(m_glyphs.begin(), m_glyphs.end(),
        [&](const std::shared_ptr<Glyph>& g) {
        return g->getUuid() == glyph.getUuid();
    });

    if (it == m_glyphs.end()) {
        Logger::Throw("Error, glyph not in vector");
    }

    return uint32_t(it - m_glyphs.begin());
}

void CanvasComponent::resize()
{
    m_dimensions = mainGLWidgetDimensions();
    for (std::shared_ptr<Glyph>& glyph : m_glyphs) {
        glyph->reload();
    }
}

void CanvasComponent::addRequiredComponents(std::vector<Uuid>& outDependencyIds, std::vector<json>& outDependencies)
{
    if (!sceneObject()->getComponent(ComponentType::kShader)) {
        // Add shader component to the scene object if not already a member
        auto shaderComp = new ShaderComponent(sceneObject());
        shaderComp->addRequiredComponents(outDependencyIds, outDependencies);

        // Create text preset and add to shader component
        std::shared_ptr<ShaderPreset> canvasPreset = ShaderPreset::GetBuiltin("canvas_gui");
        
        // Set canvas preset for shader component
        shaderComp->setShaderPreset(canvasPreset);

        // Add shader component to output list of created dependencies
        outDependencyIds.push_back(shaderComp->getUuid());
        outDependencies.push_back(shaderComp->toJson());
    }
}


} // end namespacing
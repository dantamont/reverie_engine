#include "GbDebugManager.h"

#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"
#include "../scene/GbScene.h"
#include "../scene/GbSceneObject.h"
#include "../components/GbCameraComponent.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbModelComponent.h"
#include "../components/GbTransformComponent.h"
#include "../canvas/GbLabel.h"

#include "../rendering/models/GbModel.h"
#include "../resource/GbResourceCache.h"
#include "../rendering/geometry/GbPolygon.h"
#include "../rendering/geometry/GbLines.h"
#include "../rendering/geometry/GbPoints.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/buffers/GbUniformBufferObject.h"

#include "../rendering/renderer/GbMainRenderer.h"
#include "../rendering/renderer/GbRenderCommand.h"

#include "../../view/GbWidgetManager.h"
#include "../../view/GL/GbGLWidget.h"
#include "../../view/tree/GbSceneTreeWidget.h"
#include "../input/GbInputHandler.h"

#include "../physics/GbPhysicsActor.h"
#include "../physics/GbPhysicsShape.h"
#include "../physics/GbPhysicsQuery.h"
#include "../physics/GbPhysicsScene.h"
#include "../physics/GbPhysicsGeometry.h"
#include "../physics/GbCharacterController.h"
#include "../components/GbPhysicsComponents.h"
#include "../components/GbLightComponent.h"

#include "../components/GbAnimationComponent.h"
#include "../processes/GbAnimationProcess.h"
#include "../animation/GbAnimationController.h"
#include "../physics/GbInverseKinematics.h"

#include "../../view/GL/GbGLWidget.h"
#include "../../view/GbWidgetManager.h"

#include "../utils/GbMemoryManager.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
DebugCoordinateAxes::DebugCoordinateAxes(CoreEngine* engine) :
    m_coneHeight(1.0),
    m_columnHeight(3.0),
    m_xConeTransform(std::make_shared<Transform>()),
    m_yConeTransform(std::make_shared<Transform>()),
    m_zConeTransform(std::make_shared<Transform>()),
    m_xCylinderTransform(std::make_shared<Transform>()),
    m_yCylinderTransform(std::make_shared<Transform>()),
    m_zCylinderTransform(std::make_shared<Transform>()),
    m_setUniforms(false)
{
    // Get resources, and set them as core resources so they don't get removed
    m_cylinder = engine->resourceCache()->polygonCache()->getCylinder(0.075f, 0.075f, m_columnHeight, 36, 1);
    m_cylinder->handle()->setCore(true);

    m_cone = engine->resourceCache()->polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1);
    m_cone->handle()->setCore(true);

    // Initialize GL draw settings
    m_renderSettings.setShapeMode(GL_TRIANGLES);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    //m_renderSettings.addDefaultBlend();
    setName("Coordinate Axes");

    // Set up transforms
    double halfColumnHeight = m_columnHeight / 2.0;
    Vector3 dirX = Vector3(1.0f, 0.0f, 0.0f);
    Vector3 dirY = Vector3(0.0f, 1.0f, 0.0f);
    Vector3 dirZ = Vector3(0.0f, 0.0f, 1.0f);
    m_xConeTransform->setParent(m_xCylinderTransform.get());
    m_xConeTransform->translation().setTranslation(halfColumnHeight * dirZ);
    m_xCylinderTransform->rotateAboutAxis({ 0, 1, 0 }, Constants::PI_2);
    m_xCylinderTransform->translation().setTranslation(halfColumnHeight * dirX);

    m_yConeTransform->setParent(m_yCylinderTransform.get());
    m_yConeTransform->translation().setTranslation(halfColumnHeight * dirZ);
    m_yCylinderTransform->rotateAboutAxis({ 1, 0, 0 }, -Constants::PI_2);
    m_yCylinderTransform->translation().setTranslation(halfColumnHeight * dirY);

    m_zConeTransform->setParent(m_zCylinderTransform.get());
    m_zConeTransform->translation().setTranslation(halfColumnHeight * dirZ);
    m_zCylinderTransform->translation().setTranslation(halfColumnHeight * dirZ);
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugCoordinateAxes::~DebugCoordinateAxes()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::setTransform(const std::shared_ptr<Transform>& transform)
{
    m_transform = transform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::clear()
{
    m_transform = std::make_shared<Transform>();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::bindUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
    if (!m_transform) return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::drawGeometry(ShaderProgram& shaderProgram,
    RenderSettings * settings)
{
    VertexArrayData& cylinderData = m_cylinder->vertexData();
    VertexArrayData& coneData = m_cone->vertexData();

    // Initialize uniforms
    if (!m_setUniforms) {
        shaderProgram.setUniformValue("xConeTransform", m_xConeTransform->worldMatrix());
        shaderProgram.setUniformValue("xCylTransform", m_xCylinderTransform->worldMatrix());
        shaderProgram.setUniformValue("yConeTransform", m_yConeTransform->worldMatrix());
        shaderProgram.setUniformValue("yCylTransform", m_yCylinderTransform->worldMatrix());
        shaderProgram.setUniformValue("zConeTransform", m_zConeTransform->worldMatrix());
        shaderProgram.setUniformValue("zCylTransform", m_zCylinderTransform->worldMatrix());
        m_setUniforms = true;
    }
    // Set world matrix to rotation and translation of scene object
    Matrix4x4g worldMatrix;
    if (m_transform) {
        worldMatrix = m_transform->rotationTranslationMatrix();
    }
    shaderProgram.setUniformValue("worldMatrix", worldMatrix);

    // X-axis
    Vector4 color = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    for (size_t i = 0; i < 3; i++) {
        color[i] = 1.0f;
        if (i != 0) color[i - 1] = 0.0f;
        shaderProgram.setUniformValue("axis", (int)i);
        shaderProgram.setUniformValue("color", color);
        shaderProgram.setUniformValue("isCone", true, true);
        coneData.drawGeometry(settings->shapeMode());

        shaderProgram.setUniformValue("isCone", false, true);
        cylinderData.drawGeometry(settings->shapeMode());
    }

    //shaderProgram->release();
}




/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::DebugGrid() :
    m_grid(Lines::getPlane(1.0, 100))
{
    // Set the grid as a core resource
    std::shared_ptr<Mesh> grid = 
        CoreEngine::engines().begin()->second->resourceCache()->polygonCache()->getGridPlane(1.0, 100);
    grid->handle()->setCore(true);

    // Set GL draw settings
    m_renderSettings.setShapeMode(GL_TRIANGLES);
    m_renderSettings.addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_renderSettings.addDefaultBlend();
    setName(m_grid->getName());
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::~DebugGrid()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::bindUniforms(ShaderProgram& shaderProgram)
{
    shaderProgram.setUniformValue("lineColor", Vector4(1.0, 1.0, 1.0, 1.0));

	if (!UBO::getCameraBuffer()) return;
    auto cameraBuffer = UBO::getCameraBuffer();

    // Set world matrix uniform and update uniforms in shader queue
    Matrix3x3g worldMatrix3x3;
    const Matrix4x4g& viewMatrix = cameraBuffer->getUniformValue("viewMatrix").get<Matrix4x4g>();
    Vector3 eye = viewMatrix.getColumn(3);
    //real_g distance = eye.length();
    //float scale = (float)pow(4.0f, ceil(log10(distance) / log10(4.0)));
    float scale = 100;
    worldMatrix3x3 *= scale;
    Matrix4x4g worldMatrix(worldMatrix3x3);

    m_grid->setLineColor(Vector4(0.9f, 0.9f, 0.9f, 0.5f));
    m_grid->setLineThickness(1.25f);
    m_grid->setConstantScreenThickness(false);
    m_grid->setFadeWithDistance(true);
    m_grid->transform().updateWorldMatrix(worldMatrix);
    m_grid->setUseMiter(false);

    // Add rotation to transform
    m_grid->transform().rotateAboutAxis({ 1.0f, 0.0f, 0.0f }, Constants::PI_2);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::releaseUniforms(Gb::ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::drawGeometry(ShaderProgram& shaderProgram, RenderSettings * settings)
{
    // Draw outer grid
    Q_UNUSED(settings);

    RenderContext& context = CoreEngine::engines().begin()->second->mainRenderer()->renderContext();

    m_grid->draw(shaderProgram, &context);

    // Draw inner grid
    Matrix3x3g worldMatrix3x3 = m_grid->transform().worldMatrix();
    worldMatrix3x3 *= (real_g)(1.0 / 10.0);
    Matrix4x4g worldMatrix(worldMatrix3x3);
    worldMatrix.setTranslation({ 0.0f, -0.05f, 0.0f });
    m_grid->setLineColor(Vector4((real_g)0.9, (real_g)0.9, (real_g)0.9, (real_g)0.5));
    m_grid->setLineThickness(0.5f);
    m_grid->transform().updateWorldMatrix(worldMatrix);

    m_grid->draw(shaderProgram, &context);

}




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// DebugDrawSettings
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DebugDrawSettings::asJson() const
{
    QJsonObject object;
    object.insert("drawAxes", m_drawAxes);
    object.insert("drawGrid", m_drawGrid);
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugDrawSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    QJsonObject object = json.toObject();
    m_drawAxes = object["drawAxes"].toBool(true);
    m_drawGrid = object["drawGrid"].toBool(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// DebugManager
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::DebugManager(CoreEngine* engine) :
    Manager(engine, "Debug Manager"),
    m_raycast(std::make_unique<Raycast>()),
    m_fpsCounter(nullptr),
    m_debugRenderLayer(std::make_shared<SortingLayer>(QStringLiteral("Debug"), 100))
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::~DebugManager()
{
    // Remove scene objects
    SceneObject::EraseFromNodeMap(m_cameraObject->getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent * DebugManager::camera()
{
    return m_cameraObject->camera();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::draw(const std::shared_ptr<Scene>& scene)
{
    // Make sure to bind camera buffer uniforms!
    camera()->camera().bindUniforms(nullptr);
    drawStatic();
    if (scene != m_scene) {
        drawDynamic(scene);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDynamic(const std::shared_ptr<Scene>& scene)
{
    // Render component-specific renderables
    // FIXME: Draw child scene objects
    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {

        drawDynamic(so);
    }

    // Draw raycast hit
    drawRaycastContact();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDynamic(const std::shared_ptr<SceneObject>& so) 
{
    // Check if in bounds
    // TODO: Maybe ascribe bounds to scene objects at runtime from component AABBs
    BoundingSphere sphere = BoundingSphere();
    sphere.recalculateBounds(*so->transform(), sphere);
    sphere.setRadius(4);

    if (!camera()->camera().frustum().intersects(sphere)) {
        return;
    }

    const std::vector<std::vector<Component*>>& components = so->components();
    size_t count = 0;
    for (const auto& componentVec : components) {
        // Iterate through scene object components
        Component::ComponentType componentType = Component::ComponentType(count);
        for (const auto& component : componentVec) {

            // Draw debug renderable based on component type
            DebugRenderable* renderable = nullptr;
            switch (componentType) {
            case Component::ComponentType::kTransform: {
                // Handled separately
                break;
            }
            case Component::ComponentType::kRigidBody: {
                RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(component);
                // TODO: Draw if disabled, just grey
                for (const PhysicsShape* shape : rigidBodyComp->body()->shapes()) {
                    const PhysicsShapePrefab& prefab = shape->prefab();
                    drawPhysicsShape(prefab, *so->transform(), rigidBodyComp->isEnabled());
                }
                break;
            }
            case Component::ComponentType::kBoneAnimation: {
                BoneAnimationComponent* comp = static_cast<BoneAnimationComponent*>(component);
                drawAnimation(comp);
                break;
            }
            case Component::ComponentType::kCharacterController: {
                CharControlComponent* comp = static_cast<CharControlComponent*>(component);
                drawCharacterController(comp);
                break;
            }
            case Component::ComponentType::kLight: {
                LightComponent* comp = static_cast<LightComponent*>(component);
                drawLight(comp);
                break;
            }
            default:
                continue;
            }
            if (renderable) {
                renderable->draw();
            }
        }

        count++;
    }

    if (m_settings.m_drawAxes) {
        // Render transform component
        DebugRenderable* transformRenderable = &m_dynamicRenderables["coordinateAxes"];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            transformRenderable->m_renderable);

        // Set uniforms
        transformRenderable->m_shaderProgram->bind();
        debugAxes->setTransform(so->transform());
        transformRenderable->m_shaderProgram->updateUniforms();
        //for (const auto& uniform : transformRenderable->m_shaderProgram->uniforms()) {
        //    logInfo(QString(uniform));
        //}
        //logInfo(UBO::getCameraBuffer()->getUniformValue("viewMatrix"));
        //logWarning("------------");
        transformRenderable->draw();
    }

    for (const std::shared_ptr<SceneObject>& child : so->children()) {
        drawDynamic(child);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawStatic()
{
    // Render label and icon for scene object location
    //m_textCanvas.m_shaderProgram->bind();
    //m_textCanvas.draw();

    for (auto it = m_staticRenderables.begin(); it != m_staticRenderables.end();) {

        if (!it->second.object())
        {
            // Erase debug renderable if the object is gone
            m_staticRenderables.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            if (it->first == QStringLiteral("grid")) {
                // If draw grid is set to false, disable
                it.value().m_enabled = m_settings.m_drawGrid;
                //continue;
            }

            // Set camera uniforms
            it->second.m_shaderProgram->bind();
            it->second.m_shaderProgram->updateUniforms();

            // Draw debug renderable
            it.value().draw();
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDoodads()
{
    for (auto it = m_doodads.begin(); it != m_doodads.end();) {

        if (!it->object() || it->isDone())
        {
            // Erase debug renderable if the object is gone
            m_doodads.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            // Set camera uniforms
            it->m_shaderProgram->bind();
            it->m_shaderProgram->updateUniforms();

            // Draw debug renderable
            it->draw();
            ++it;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawCommands(Scene& scene, MainRenderer & renderer)
{
    // Text commands
    std::shared_ptr<ShaderProgram> textProgram = m_engine->resourceCache()->getHandleWithName("text",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();
    std::vector<std::shared_ptr<DrawCommand>> drawCommands;
    m_textCanvas->createDrawCommands(drawCommands,
        camera()->camera(),
        *textProgram);

    // Draw camera bounding boxes:
    //{
    //    std::vector<AABBData> aabbs;
    //    camera()->camera().lightClusterGrid().getGridBoxes(aabbs);

    //    Matrix4x4g inverseView = camera()->camera().getViewMatrix().inversed();
    //    size_t numRows = 1;

    //    std::shared_ptr<Lines> lineCube = S_CAST<Lines>(m_dynamicRenderables["cube"].m_renderable);

    //    // FIXME: Set this during draw call
    //    lineCube->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, true);
    //    lineCube->setLineColor(Vector4g(1.0f, 0.0f, 0.5, 1.0f));
    //    lineCube->setLineThickness(0.005f);
    //    lineCube->setConstantScreenThickness(true);
    //    lineCube->setFadeWithDistance(false);
    //    lineCube->setUseMiter(false);

    //    for (size_t j = 0; j < numRows; j++) {

    //        for (size_t i = 0; i < 16 * 9; i++) {
    //            const AABBData& box = aabbs[i + j * 16 * 9];

    //            // Convert from view space to world space);
    //            Matrix4x4g worldMatrix;
    //            std::vector<Vector3g> points;
    //            box.getPoints(points);

    //            Vector3g scale = box.getDimensions();
    //            worldMatrix(0, 0) = scale.x();
    //            worldMatrix(1, 1) = scale.y();
    //            worldMatrix(2, 2) = scale.z();

    //            Vector3g origin = box.getOrigin();
    //            worldMatrix.setTranslation(origin);
    //            lineCube->transform().updateWorldMatrix(worldMatrix);

    //            // box is in view space, so revert
    //            worldMatrix = inverseView * worldMatrix;

    //            auto command = std::make_shared<DrawCommand>(*lineCube,
    //                *m_lineShaderProgram,
    //                camera()->camera());
    //            command->setUniform(Uniform("worldMatrix", worldMatrix));
    //            //command->setUniform(Uniform("lineColor", Vector4g(0.5f, 0.0f, 1.0, 1.0f)));
    //            drawCommands.push_back(command);
    //        }
    //    }
    //}


    // For frustum culling, create bounding boxes
    //std::vector<std::shared_ptr<RenderCommand>>& sceneCommands = renderer.receivedCommands();
    //for (const auto& command : sceneCommands) {
    //    auto drawCommand = S_CAST<DrawCommand>(command);
    //    BoundingBoxes& boxes = drawCommand->renderable()->boundingBoxes();
    //    for (const auto& box : boxes.geometry()) {
    //        createDrawCommand(box, drawCommands);
    //    }
    //}


    for (const std::shared_ptr<SceneObject>& so : scene.topLevelSceneObjects()) {
        createDrawCommand(*so, drawCommands);
    }


    // Add commands to debug layer
    for (const auto& command : drawCommands) {
        command->setRenderLayer(m_debugRenderLayer.get());
        renderer.addRenderCommand(command);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::step(unsigned long deltaMs)
{
    if (!m_engine->isConstructed()) return;
    if (!m_cameraObject) {
        return;
    }

    // Move camera
    camera()->controller().step(deltaMs);

    processRaycasts();

    // Set OpenGL context to main context to avoid VAO struggles
    m_engine->setGLContext();

    // Set FPS counter
    if (m_fpsCounter) {
        if (m_frameDeltas.size() > 15) {
            m_frameDeltas.pop_front();
        }
        m_frameDeltas.push_back(1000.0 / double(deltaMs));
        float fps = getFps();
        size_t numDigits = floor(log10(fps)) + 1; // One if less than 10, Two digits if less than 100, three if greater, etc.
        m_fpsCounter->setText(QString::number(fps, 'g', numDigits));
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::postConstruction()
{
    // FIXME: Load and set cubemap for skybox
    // Problematic, can't access via non-Qt load, 
    // see: https://stackoverflow.com/questions/43880820/how-to-convert-qt-file-path-from-resource-to-absolute-path
    //std::shared_ptr<ResourceHandle> cubemapHandle =
    //    m_engine->resourceCache()->guaranteeHandleWithPath(
    //        ":/textures/debug_skybox/debug.cubemap",
    //        Resource::kCubeTexture
    //    );
    //cubemapHandle->setCore(true);

    // Set up scene
    m_scene = Scene::create(m_engine);
    m_scene->setName("Debug Objects");

    // Set up canvas
    m_textCanvas = std::make_shared<CanvasComponent>(m_engine);
    m_textCanvas->setName("Debug Canvas");

    std::shared_ptr<Label> mainLabel = std::make_shared<Label>(m_textCanvas.get(), "Debug View",
        "arial", 30);
    mainLabel->setToGui();
    mainLabel->setFaceCamera(true);
    mainLabel->setOnTop(true);
    mainLabel->setCoordinates({ 0.0f, 0.95f });
    mainLabel->setAlignment(Glyph::kMiddle, Glyph::kCenter);
    m_textCanvas->addGlyph(mainLabel);

    m_fpsCounter = std::make_shared<Label>(m_textCanvas.get(), "0",
        "courier", 30);
    m_fpsCounter->setToGui();
    m_fpsCounter->setFaceCamera(true);
    m_fpsCounter->setOnTop(true);
    m_fpsCounter->setCoordinates({ -0.97f, 0.95f });
    m_fpsCounter->setColor(Color(Vector3(0.4f, 0.2f, 0.7f)));
    m_fpsCounter->setAlignment(Glyph::kMiddle, Glyph::kLeft);
    m_textCanvas->addGlyph(m_fpsCounter);

    // Set up camera
    initializeCamera();

    // Set up grid
    // TODO: Use actual infinite ground plane
    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
    m_lineShaderProgram = m_engine->resourceCache()->getHandleWithName("lines",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();
    auto grid = std::make_shared<DebugGrid>();
    m_staticRenderables["grid"] = { grid, m_lineShaderProgram, grid };

    // Set up a line plane
    std::shared_ptr<Lines> plane = Lines::getPlane(1.0, 1000, int(ResourceHandle::kCore));
    m_dynamicRenderables["plane"] = { plane, m_lineShaderProgram, plane };

    // Set up line and point box
    std::shared_ptr<Lines> box = Lines::getCube(int(ResourceHandle::kCore));
    m_dynamicRenderables["cube"] = { box, m_lineShaderProgram, box };

    std::shared_ptr<Points> boxPoints = std::make_shared<Points>(*box);
    m_dynamicRenderables["pointCube"] = { boxPoints , m_pointShaderProgram, boxPoints };

    // Set up line and point sphere
    std::shared_ptr<Lines> sphere = Lines::getSphere(20, 30, int(ResourceHandle::kCore));
    m_dynamicRenderables["sphere"] = { sphere, m_lineShaderProgram, sphere };

    std::shared_ptr<Points> spherePoints = std::make_shared<Points>(*sphere);
    m_dynamicRenderables["pointSphere"] = { spherePoints , m_pointShaderProgram, spherePoints };

    // Set up simple shader program
    m_simpleShaderProgram = m_engine->resourceCache()->getHandleWithName("simple",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();

    // Set up point shader program
    m_pointShaderProgram = m_engine->resourceCache()->getHandleWithName("points",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();

    // Set up coordinate axes
    m_axisShaderProgram = m_engine->resourceCache()->getHandleWithName("axes",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();
    auto coordinateAxes = std::make_shared<DebugCoordinateAxes>(m_engine);
    m_dynamicRenderables["coordinateAxes"] = { coordinateAxes, m_axisShaderProgram , coordinateAxes };
    m_staticRenderables["coordinateAxes"] = m_dynamicRenderables["coordinateAxes"];

    // Create debug skeleton program
    m_debugSkeletonProgram = m_engine->resourceCache()->getHandleWithName("debug_skeleton",
        Resource::kShaderProgram)->resourceAs<ShaderProgram>();

    auto skeletonPoints = std::make_shared<Points>(100);
    skeletonPoints->renderSettings().addSetting<DepthSetting>(false, DepthPassMode::kLessEqual, false);
    m_dynamicRenderables["skeletonPoints"] = { skeletonPoints, m_debugSkeletonProgram, skeletonPoints};

    // Set renderable for raycast hit
    auto raycastHit = Lines::getPrism(0.25f, 0.0f, 2.0f, 6, 1, int(ResourceHandle::kCore));
    m_dynamicRenderables["raycastHit"] = {raycastHit, m_lineShaderProgram, raycastHit};

    // Set GL widget
    m_glWidget = m_engine->widgetManager()->mainGLWidget();

    Manager::postConstruction();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DebugManager::asJson() const
{
    QJsonObject object;
    object.insert("debugScene", m_scene->asJson());
    object.insert("debugSettings", m_settings.asJson());

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    const QJsonObject& object = json.toObject();
    if (object.contains("debugScene")) {
        m_scene->loadFromJson(json["debugScene"]);
        m_cameraObject = nullptr; // delete old camera object        
        m_cameraObject = m_scene->getSceneObjectByName("Debug Camera");
    }
    else {
        m_cameraObject->loadFromJson(object.value("cameraObject"));
    } 

    // TODO: Add debug cubemap

    // Add object to debug render layer so that skybox renders
    m_cameraObject->addRenderLayer(m_debugRenderLayer);

    m_cameraObject->camera()->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);

    if (object.contains("debugSettings")) {
        m_settings.loadFromJson(object["debugSettings"]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::processRaycasts() {
    // Generate raycasts
    bool hit;

    if (!m_glWidget->underMouse()) return;

    if (!m_engine->scenario()) return;

    for (const std::shared_ptr<Scene>& scene : m_engine->scenario()->getScenes()) {
        // Skip if the scene has no physics
        if (!scene->physics()) continue;
    
        // Generate raycast
        *m_raycast = Raycast();
        m_raycast->m_origin = cameraObject()->transform()->getPosition().asReal();
        camera()->camera().widgetToRayDirection(m_glWidget->widgetMousePosition(),
            m_raycast->m_direction, *m_glWidget->renderer());
        hit = scene->physics()->raycast(*m_raycast);
        Q_UNUSED(hit)
        //if (hit) {
        //    RaycastHit hit = raycast->m_hits.getHit(0);
        //    HitFlags flags = hit.flags();
        //    bool hasPos = flags.testFlag(HitEnum::PxHitFlags::kPOSITION);
        //    bool hasNormal = flags.testFlag(HitEnum::PxHitFlags::kPOSITION);
        //    bool hasUV = flags.testFlag(HitEnum::PxHitFlags::kUV);
        //    Vector3g position = hit.position();
        //    Vector3g normal = hit.normal();
        //    //logInfo("HIT HIT HIT: Position: " + QString(position) +", Normal: " + QString(normal));
        //}
    }

    // TODO: Add in detection of transforms and scene objects
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::initializeCamera()
{
    m_cameraObject = SceneObject::create(m_scene);
    m_cameraObject->setName("Debug Camera");
    m_cameraObject->transform()->translation().setPosition({ 0.0, 0.0, 1.0 });

    new CameraComponent(m_cameraObject);
    CameraController& controller = camera()->controller();
    controller.profile().m_movementTypes = tsl::robin_map<CameraController::MovementType, bool>{
        {CameraController::kZoom, true},
        {CameraController::kPan, true},
        {CameraController::kTilt, true},
        {CameraController::kTranslate, true},
        {CameraController::kRotate, true},
        {CameraController::kTilt, true}
    };
    camera()->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, true);
}
/////////////////////////////////////////////////////////////////////////////////////////////
InputHandler& DebugManager::inputHandler() const
{
    return m_engine->widgetManager()->mainGLWidget()->inputHandler();
}
/////////////////////////////////////////////////////////////////////////////////////////////
float DebugManager::getFps() const
{
    return std::accumulate(m_frameDeltas.begin(), m_frameDeltas.end(), 0.0) / m_frameDeltas.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawPhysicsShape(const PhysicsShapePrefab& shape, 
    const TransformComponent& transform, bool isEnabled)
{
    if (!m_lineShaderProgram) return;
    if (!m_pointShaderProgram) return;

    bool wasHit = false;
    if (m_raycast->hadHit()) {
        RaycastHit hit = m_raycast->firstHit();
        std::shared_ptr<SceneObject> hitObject = hit.sceneObject();
        if (hitObject)
            wasHit = hitObject->getUuid() == transform.sceneObject()->getUuid();
        else {
            logError("Error, scene object for hit not found");
        }
    }

    // Set point/line color based on enabled status
    Vector4 lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    Vector4 pointColor(0.9f, 0.25f, 0.2f, 0.9f);
    float pointSize = 0.006f;
    float lineThickness = 0.0075f;
    if (!isEnabled) {
        lineThickness = 0.005f;
        lineColor = Vector4(0.5f, 0.5f, 0.5f, 0.9f);
        pointColor = lineColor;
        pointSize = 0.004f;
    }
    else {
        if (wasHit) {
            lineThickness *= 1.25f;
            lineColor = Vector4(0.8f, 0.9f, 1.0f, 1.0f);
            pointColor = lineColor;
            pointSize *= 1.25f;
        }
    }

    // Set camera uniforms
    m_lineShaderProgram->bind();

    //m_cone = std::static_pointer_cast<Mesh>(
    //    engine->resourceCache()->polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1)->resource(false));

    Matrix4x4g worldMatrix = transform.rotationTranslationMatrix();
    QString worldStr(worldMatrix);

    switch (shape.geometry()->getType()) {
    case PhysicsGeometry::kBox: {
        auto boxGeometry = std::static_pointer_cast<BoxGeometry>(shape.geometry());
        Vector3 scale = {boxGeometry->hx() * 2.0f, boxGeometry->hy()* 2.0f,  boxGeometry->hz() * 2.0f };
        worldMatrix.addScale(scale);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["cube"].object());
        lines->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, false);
        lines->setLineColor(lineColor);
        lines->setLineThickness(lineThickness);
        lines->setConstantScreenThickness(true);
        lines->setFadeWithDistance(false);
        lines->setUseMiter(false);
        lines->transform().updateWorldMatrix(worldMatrix);
        m_dynamicRenderables["cube"].draw();

        // Draw points
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(
            m_dynamicRenderables["pointCube"].object());
        points->setPointColor(pointColor);
        points->setPointSize(pointSize);
        points->transform().updateWorldMatrix(worldMatrix);
        points->draw(*m_pointShaderProgram, &mainRenderContext());
        break;
    }
    case PhysicsGeometry::kSphere: {
        auto sphereGeometry = std::static_pointer_cast<SphereGeometry>(shape.geometry());
        Vector3 scale = { sphereGeometry->radius(), sphereGeometry->radius(),  sphereGeometry->radius()};
        worldMatrix.addScale(scale);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["sphere"].object());
        lines->setLineColor(lineColor);
        lines->setLineThickness(lineThickness);
        lines->setConstantScreenThickness(true);
        lines->setFadeWithDistance(false);
        lines->setUseMiter(false);
        lines->transform().updateWorldMatrix(worldMatrix);
        m_dynamicRenderables["sphere"].draw();

        // Draw points
        std::shared_ptr<Points> points = std::static_pointer_cast<Points>(
            m_dynamicRenderables["pointSphere"].object());
        points->setPointColor(pointColor);
        points->setPointSize(pointSize);
        points->transform().updateWorldMatrix(worldMatrix);
        points->draw(*m_pointShaderProgram, &mainRenderContext());
        break;
    }
    case PhysicsGeometry::kPlane: {
        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["plane"].object());
        //lines->setLineColor(Vector4g(0.20f, 0.3f, 0.70f, 0.2f));
        lines->setLineColor(lineColor);
        lines->setLineThickness(0.1f);
        lines->setConstantScreenThickness(false);
        lines->setFadeWithDistance(true);
        lines->setUseMiter(false);

        // TODO: Use actual infinite ground plane
        // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
        // Add rotation to transform
        Matrix4x4g identity;
        lines->transform().updateWorldMatrix(identity);
        lines->transform().rotateAboutAxis({ 0.0f, 1.0f, 0.0f }, Constants::PI_2);

        lines->transform().updateWorldMatrix(worldMatrix * lines->transform().worldMatrix());

        //const RenderProjection& rp = m_cameraObject->camera()->camera().renderProjection();
        //float nearPlane = rp.nearClipPlane();
        //Matrix4x4g projectionMatrix(m_cameraObject->camera()->camera().renderProjection().projectionMatrix());
        //projectionMatrix(2, 2) = 0;
        //projectionMatrix(2, 3) = -2 * nearPlane;
        //m_lineShaderProgram->setUniformValue("projectionMatrix", projectionMatrix);

        m_dynamicRenderables["plane"].draw();
        break; 
    }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawRaycastContact()
{
    if (!m_lineShaderProgram) return;
    if (!m_raycast->hadHit()) return;

    RaycastHit hit = m_raycast->firstHit();

    // Set point/line color based on enabled status
    Vector4 lineColor(0.75f, 0.9f, 1.0f, 0.9f);
    float lineThickness = 0.1f;

    // Set camera uniforms
    m_lineShaderProgram->bind();
    
    // Set world matrix uniform
    float prismLength = 1.0;
    Quaternion rotation = Quaternion::rotationTo(Vector3(0.0f, 0.0f, 1.0f), hit.normal());
    Matrix4x4g worldMatrix;
    worldMatrix.setTranslation(hit.position() + hit.normal() * prismLength);
    worldMatrix *= rotation.toRotationMatrix4x4();
    //worldMatrix.addScale(Vector3g(1.0f, 1.0f, 1.0f));

    // Set line and world space uniforms
    std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
        m_dynamicRenderables["raycastHit"].object());
    lines->setLineColor(lineColor);
    lines->setLineThickness(lineThickness);
    lines->setConstantScreenThickness(false);
    lines->setFadeWithDistance(false);
    lines->setUseMiter(false);
    lines->transform().updateWorldMatrix(worldMatrix);
    m_dynamicRenderables["raycastHit"].draw();

}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::clear()
{
    // Delete coordinate axes to avoid crash
    QString coordAxesStr = QStringLiteral("coordinateAxes");
    if (Map::HasKey(m_dynamicRenderables, coordAxesStr)) {
        DebugRenderable* coordAxes = &m_dynamicRenderables[coordAxesStr];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            coordAxes->m_renderable);
        debugAxes->clear();
    }

    // 
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawAnimation(BoneAnimationComponent * animComp)
{
    // FIXME: Get rid of unused uniform warnings in ShaderProgram::updateUniforms from:
    // color, etc.
    // Bind shader program
    if (!m_debugSkeletonProgram) return;
    m_debugSkeletonProgram->bind();

    auto model = animComp->animationController().getModel();
    if (!model) return;

    if (!model->skeleton()->handle()) {
        return;
    }

    if (model->skeleton()->handle()->isLoading()) {
        return;
    }

    // Get renderable
    Points& points = *std::static_pointer_cast<Points>(
        m_dynamicRenderables["skeletonPoints"].m_renderable);

    // Bind uniforms
    animComp->bindUniforms(*m_debugSkeletonProgram);
    points.setPointColor({1.0f, 0.0f, 1.0f, 1.0f});
    points.setPointSize(0.005f);
    points.transform().updateWorldMatrix(animComp->sceneObject()->transform()->worldMatrix());
    
    // Get colors
    Skeleton skeleton = model->skeleton()->prunedBoneSkeleton();
    IKChain chain(skeleton);
    std::vector<Vector4> colors(points.numPoints());
    size_t idx;
    Vector4 color;
    for (const size_t& nodeIndex : skeleton.boneNodes()) {
        SkeletonJoint& node = skeleton.getNode(nodeIndex);
        idx = node.bone().m_index;
       
        IKNode* ikNode = chain.getNode(node.getName());
        if (ikNode->isEndEffector()) {
            color = Vector4(0.85f, 0.5f, 0.1f, 1.0f);
        }
        else if (ikNode->isRoot()) {
            color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (ikNode->isSubBase()) {
            color = Vector4(1.0f, 1.0f, 0.2f, 1.0f);
        }
        else {
            color = Vector4(0.75f, 0.5f, 1.0f, 1.0f);
        }
        colors[idx] = color;
    }

    m_debugSkeletonProgram->setUniformValue("colors", colors);

    // Draw skeleton points
    points.draw(*m_debugSkeletonProgram, &mainRenderContext());

    //auto boneTransforms = m_debugSkeletonProgram->getUniformValue("boneTransforms")->get<std::vector<Matrix4x4g>>();
    //for (const auto& t : boneTransforms) {
    //    auto point = animComp->sceneObject()->transform()->worldMatrix() * t * Vector4g(0.0f, 0.0f, 0.0f, 1.0f);
    //    logInfo(QString(point));
    //}
    //logWarning("-----------");

}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawCharacterController(CharControlComponent * comp)
{
    if (!m_lineShaderProgram) return;
    if (!m_pointShaderProgram) return;

    //bool wasHit = false;
    //if (m_raycast->hadHit()) {
    //    RaycastHit hit = m_raycast->firstHit();
    //    std::shared_ptr<SceneObject> hitObject = hit.sceneObject();
    //    if (hitObject)
    //        wasHit = hitObject->getUuid() == comp->sceneObject()->getUuid();
    //    else
    //        logError("Error, scene object for hit not found");
    //}

    // Set line color based on enabled status
    Vector4 lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    float lineThickness = 0.0075f;
    if (!comp->isEnabled()) {
        lineThickness = 0.005f;
        lineColor = Vector4(0.5f, 0.5f, 0.5f, 0.9f);
    }
    //else {
    //    if (wasHit) {
    //        lineThickness *= 1.25f;
    //        lineColor = Vector4g(0.8f, 0.9f, 1.0f, 1.0f);
    //    }
    //}

    // Set camera uniforms
    m_lineShaderProgram->bind();
    switch(comp->controller()->getType()){
        case CharacterController::kCapsule:
        {
            auto controller = std::static_pointer_cast<CapsuleController>(comp->controller());
            float radius = controller->getRadius();
            float halfHeight = controller->getHeight() / 2.0;
            QString name = PolygonCache::getCapsuleName(radius, halfHeight);

            Matrix4x4g worldMatrix;
            worldMatrix.setTranslation(controller->getPosition().asReal());

            // Set line and world space uniforms
            if (!Map::HasKey(m_dynamicRenderables, name)) {
                std::shared_ptr<Lines> lines = Lines::getCapsule(radius, halfHeight);
                auto capsule = m_engine->resourceCache()->polygonCache()->getCylinder(radius, halfHeight);
                m_dynamicRenderables[name] = {lines, m_lineShaderProgram, comp->sceneObject()};
            }
            std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(m_dynamicRenderables[name].m_renderable);
            lines->setLineColor(lineColor);
            lines->setLineThickness(0.05f);
            lines->setConstantScreenThickness(false);
            lines->setFadeWithDistance(true);
            lines->setUseMiter(false);
            lines->transform().updateWorldMatrix(worldMatrix);

            // Draw outer capsule (with contact offset)
            lines->draw(*m_lineShaderProgram, &mainRenderContext());

            //// Draw inner capsule
            //float offset = comp->controller()->getContactOffset();
            //float scale = radius / (radius + offset);
            //lineColor[0] = 1.0;
            //lineColor[1] = 0.4;
            //lineColor[2] = 0.1;
            //lines->setLineColor(lineColor);
            //lines->setLineThickness(0.1f);
            //Matrix3x3g scaled;
            //scaled *= scale;
            //Matrix4x4g scaleMat(scaled);
            //lines->transform().updateWorldMatrix(worldMatrix * scaleMat);
            //lines->draw(m_lineShaderProgram, &mainRenderContext());
            break;
        }
        default:
            break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawLight(LightComponent * light)
{
    if (!m_simpleShaderProgram) return;

    // Get color and world matrix uniforms
    Vector4 color = light->getDiffuseColor();
    //light->cacheLight();
    if (!light->isEnabled()) {
        color = Vector4(0.5f, 0.5f, 0.5f, 0.75f);
    }

    // Slow, and only necessary if you really want to ensure lights are all the same size
    //Matrix4x4g worldMatrix = light->sceneObject()->transform()->rotationTranslationMatrix();
    const Matrix4x4g& worldMatrix = light->sceneObject()->transform()->worldMatrix();

    // Set camera uniforms
    m_simpleShaderProgram->bind();
    
    // Get geometry and set uniforms
    if (!m_sphereMesh) {
        m_sphereMesh = m_engine->resourceCache()->polygonCache()->getSphere(20, 30);
    }
    
    m_simpleShaderProgram->setUniformValue("worldMatrix", worldMatrix);
    m_simpleShaderProgram->setUniformValue("color", color, true);

    // Draw outer capsule (with contact offset)
    m_sphereMesh->vertexData().drawGeometry(GL_TRIANGLES);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands)
{
    Q_UNUSED(outDrawCommands)
    Q_UNUSED(so)
    // Draw bounding boxes (slow, need to only draw for visible geometry)
    //if (so.hasComponent(Component::ComponentType::kModel)) {
    //    ModelComponent* modelComp = so.modelComponent();

    //    for (const BoundingBoxes& boxes : modelComp->bounds()) {
    //        for (const AABB& box : boxes.geometry()) {
    //            std::shared_ptr<Lines> lineCube = S_CAST<Lines>(m_dynamicRenderables["cube"].m_renderable);
    //            
    //            // FIXME: Set this during draw call
    //            lineCube->drawFlags().setFlag(Lines::DrawFlag::kIgnoreWorldMatrix, true);
    //            lineCube->setLineColor(Vector4g(1.0f, 0.0f, 1.0f, 1.0f));
    //            lineCube->setLineThickness(0.1f);
    //            lineCube->setConstantScreenThickness(false);
    //            lineCube->setFadeWithDistance(false);
    //            lineCube->setUseMiter(false);

    //            Matrix4x4g worldMatrix;
    //            std::vector<Vector3g> points;
    //            box.getPoints(points);

    //            Vector3g scale = box.getDimensions();
    //            worldMatrix(0, 0) = scale.x();
    //            worldMatrix(1, 1) = scale.y();
    //            worldMatrix(2, 2) = scale.z();

    //            Vector3g origin = box.getOrigin();
    //            worldMatrix.setTranslation(origin);
    //            lineCube->transform().updateWorldMatrix(worldMatrix);

    //            auto command = std::make_shared<DrawCommand>(*lineCube,
    //                *m_lineShaderProgram,
    //                camera()->camera());
    //            command->setUniform(Uniform("worldMatrix", worldMatrix));
    //            outDrawCommands.push_back(command);
    //        }
    //    }
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
RenderContext& DebugManager::mainRenderContext() {
    return m_engine->mainRenderer()->renderContext();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::DebugRenderable::draw()
{
    if (m_enabled) {
        RenderContext& context = CoreEngine::engines().begin()->second->mainRenderer()->renderContext();
        m_renderable->draw(*m_shaderProgram,
            &context, 
            nullptr
            //Renderable::kIgnoreSettings
        );
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

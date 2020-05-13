#include "GbDebugManager.h"

#include "../GbCoreEngine.h"
#include "../scene/GbScenario.h"
#include "../scene/GbScene.h"
#include "../scene/GbSceneObject.h"
#include "../components/GbCamera.h"
#include "../components/GbCanvasComponent.h"
#include "../components/GbTransformComponent.h"
#include "../canvas/GbLabel.h"

#include "../resource/GbResourceCache.cpp"
#include "../rendering/geometry/GbPolygon.h"
#include "../rendering/geometry/GbLines.h"
#include "../rendering/geometry/GbPoints.h"
#include "../rendering/geometry/GbSkeleton.h"
#include "../rendering/shaders/GbUniformBufferObject.h"

#include "../rendering/renderer/GbRenderers.h"
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
#include "../components/GbLight.h"

#include "../components/GbAnimationComponent.h"
#include "../processes/GbAnimationProcess.h"
#include "../physics/GbInverseKinematics.h"

#include "../../view/GL/GbGLWidget.h"
#include "../../view/GbWidgetManager.h"

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
    std::shared_ptr<ResourceHandle> cylinderHandle = engine->resourceCache()->polygonCache()->getCylinder(0.075f, 0.075f, m_columnHeight, 36, 1);
    cylinderHandle->setCore(true);
    m_cylinder = std::static_pointer_cast<Mesh>(cylinderHandle->resource(false));

    std::shared_ptr<ResourceHandle> coneHandle = engine->resourceCache()->polygonCache()->getCylinder(0.2f, 0.0f, m_coneHeight, 36, 1);
    coneHandle->setCore(true);
    m_cone = std::static_pointer_cast<Mesh>(coneHandle->resource(false));

    // Initialize GL draw settings
    m_renderSettings.setShapeMode(GL_TRIANGLES);
    m_renderSettings.addSetting(std::make_shared<DepthSetting>(false));
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
void DebugCoordinateAxes::setTransform(Transform * transform)
{
    m_transform = transform;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::clear()
{
    m_transform = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Q_UNUSED(shaderProgram)
    if (!m_transform) return;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugCoordinateAxes::drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram,
    RenderSettings * settings)
{
    Transform transform;
    if (m_transform) {
        transform = *m_transform;
    }
    VertexArrayData* cylinderData = m_cylinder->meshData().begin()->second;
    VertexArrayData* coneData = m_cone->meshData().begin()->second;

    // Initialize uniforms
    if (!m_setUniforms) {
        shaderProgram->setUniformValue("xConeTransform", m_xConeTransform->worldMatrix());
        shaderProgram->setUniformValue("xCylTransform", m_xCylinderTransform->worldMatrix());
        shaderProgram->setUniformValue("yConeTransform", m_yConeTransform->worldMatrix());
        shaderProgram->setUniformValue("yCylTransform", m_yCylinderTransform->worldMatrix());
        shaderProgram->setUniformValue("zConeTransform", m_zConeTransform->worldMatrix());
        shaderProgram->setUniformValue("zCylTransform", m_zCylinderTransform->worldMatrix());
        m_setUniforms = true;
    }
    // Set world matrix to rotation and translation of scene object
    shaderProgram->setUniformValue("worldMatrix", transform.rotationTranslationMatrix());

    // X-axis
    Vector4g color = Vector4g(0.0f, 0.0f, 0.0f, 1.0f);
    for (size_t i = 0; i < 3; i++) {
        color[i] = 1.0f;
        if (i != 0) color[i - 1] = 0.0f;
        shaderProgram->setUniformValue("axis", (int)i);
        shaderProgram->setUniformValue("color", color);
        shaderProgram->setUniformValue("isCone", true, true);
        coneData->drawGeometry(settings->shapeMode());

        shaderProgram->setUniformValue("isCone", false, true);
        cylinderData->drawGeometry(settings->shapeMode());
    }

    //shaderProgram->release();
}




/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::DebugGrid() :
    m_grid(Lines::getPlane(1.0, 100))
{
    // Set the grid as a core resource
    std::shared_ptr<ResourceHandle> handle =
        CoreEngine::engines().begin()->second->resourceCache()->polygonCache()->getGridPlane(1.0, 100);
    handle->setCore(true);

    // Set GL draw settings
    m_renderSettings.setShapeMode(GL_TRIANGLES);
    m_renderSettings.addSetting(std::make_shared<DepthSetting>(true, GL_LEQUAL));
    m_renderSettings.addDefaultBlend();
    setName(m_grid->getName());
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugGrid::~DebugGrid()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram)
{
    shaderProgram->setUniformValue("lineColor", Vector4g(1.0, 1.0, 1.0, 1.0));

	if (!UBO::getCameraBuffer()) return;
    auto cameraBuffer = UBO::getCameraBuffer();

    // Set world matrix uniform and update uniforms in shader queue
    Matrix3x3g worldMatrix3x3;
    const Matrix4x4g& viewMatrix = cameraBuffer->getUniformValue("viewMatrix").get<Matrix4x4g>();
    Vector3g eye = viewMatrix.getColumn(3);
    //real_g distance = eye.length();
    //float scale = (float)pow(4.0f, ceil(log10(distance) / log10(4.0)));
    float scale = 100;
    worldMatrix3x3 *= scale;
    Matrix4x4g worldMatrix(worldMatrix3x3);

    m_grid->setLineColor(Vector4g(0.9f, 0.9f, 0.9f, 0.5f));
    m_grid->setLineThickness(1.25f);
    m_grid->setConstantScreenThickness(false);
    m_grid->setFadeWithDistance(true);
    m_grid->transform().updateWorldMatrix(worldMatrix);
    m_grid->setUseMiter(false);

    // Add rotation to transform
    m_grid->transform().rotateAboutAxis({ 1.0f, 0.0f, 0.0f }, Constants::PI_2);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::releaseUniforms(const std::shared_ptr<Gb::ShaderProgram>& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugGrid::drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings * settings)
{
    // Draw outer grid
    Q_UNUSED(settings);
    m_grid->draw(shaderProgram);

    // Draw inner grid
    Matrix3x3g worldMatrix3x3 = shaderProgram->getUniformValue("worldMatrix")->get<Matrix4x4g>();
    worldMatrix3x3 *= (real_g)(1.0 / 10.0);
    Matrix4x4g worldMatrix(worldMatrix3x3);
    worldMatrix.setTranslation({ 0.0f, -0.05f, 0.0f });
    m_grid->setLineColor(Vector4g((real_g)0.9, (real_g)0.9, (real_g)0.9, (real_g)0.5));
    m_grid->setLineThickness(0.5f);
    m_grid->transform().updateWorldMatrix(worldMatrix);

    m_grid->draw(shaderProgram);

}



/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// DebugManager
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::DebugManager(CoreEngine* engine) :
    Manager(engine, "Debug Manager"),
    m_raycast(std::make_unique<Raycast>()),
    m_fpsCounter(nullptr)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
DebugManager::~DebugManager()
{
    // Remove scene objects
    DagNode::eraseFromNodeMap(m_cameraObject->getUuid());
}
/////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent * DebugManager::camera()
{
    return m_cameraObject->camera();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::draw(std::shared_ptr<Scene> scene)
{
    drawStatic();
    drawDynamic(scene);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawDynamic(std::shared_ptr<Scene> scene)
{
    // Render component-specific renderables
    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {

        const std::unordered_map<Component::ComponentType, std::vector<Component*>>& components = so->components();
        for (const auto& componentMapPair : components) {
            // Iterate through scene object components
            Component::ComponentType componentType = componentMapPair.first;
            for (const auto& component : componentMapPair.second) {

                // Draw debug renderable based on component type
                DebugRenderable* renderable = nullptr;
                switch (componentType) {
                case Component::kTransform: {
                    // Handled separately
                    break;
                }
                case Component::kRigidBody: {
                    RigidBodyComponent* rigidBodyComp = static_cast<RigidBodyComponent*>(component);
                    // TODO: Draw if disabled, just grey
                    for (const auto& shapePair : rigidBodyComp->body()->shapes()) {
                        const PhysicsShapePrefab& shape = shapePair.second.prefab();
                        drawPhysicsShape(shape, *so->transform(),
                            rigidBodyComp->isEnabled());
                    }
                    break;
                }
                case Component::kBoneAnimation: {
                    BoneAnimationComponent* comp = static_cast<BoneAnimationComponent*>(component);
                    drawAnimation(comp);
                    break;
                }
                case Component::kCharacterController: {
                    CharControlComponent* comp = static_cast<CharControlComponent*>(component);
                    drawCharacterController(comp);
                    break;
                }
                case Component::kLight: {
                    Light* comp = static_cast<Light*>(component);
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
        }
    }

    // Render coordinate axes
    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {
        // Render transform component
        DebugRenderable* transformRenderable = &m_dynamicRenderables["coordinateAxes"];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            transformRenderable->m_renderable);

        // Set uniforms
        transformRenderable->m_shaderProgram->bind();
        debugAxes->setTransform(so->transform().get());
        transformRenderable->draw();
    }

    // Draw raycast hit
    drawRaycastContact();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawStatic()
{
    // Render label and icon for scene object location
    m_textCanvas.m_shaderProgram->bind();
    m_textCanvas.draw();

    for (auto it = m_staticRenderables.begin(); it != m_staticRenderables.end();) {

        if (!it->second.object())
        {
            // Erase debug renderable if the object is gone
            m_staticRenderables.erase(it++);    // or "it = m.erase(it)" since C++11
        }
        else
        {
            // Set camera uniforms
            it->second.m_shaderProgram->bind();
            it->second.m_shaderProgram->updateUniforms();

            // Draw debug renderable
            it->second.draw();
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
void DebugManager::step(unsigned long deltaMs)
{
    if (!m_postConstructionDone) return;
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
        if (m_frameDeltas.size() > 10) {
            m_frameDeltas.pop_front();
        }
        m_frameDeltas.push_back(1000.0 / double(deltaMs));
        float fps = getFps();
        m_fpsCounter->setText(QString::number(fps, 'g', 3));
    }

    // For testing
    //for (const auto& scenePair:  m_engine->scenario()->getScenes()) {
    //    const std::shared_ptr<Scene>& scene = scenePair.second;
    //    for (const std::shared_ptr<SceneObject>& so : scene->topLevelSceneObjects()) {
            //if (so->characterController()) {
            //    so->characterController()->move({ 0.01, 0, 0 });
            //}
    //        if (so->boneAnimation()) {
    //            const Skeleton& skelly = 
    //                so->boneAnimation()->animationController()->getMesh()->skeleton();
    //            IKChain testChain(skelly);
    //        }
    //    }
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::postConstruction()
{
    // Set up scene
    m_scene = Scene::create(m_engine);
    m_scene->setName("Debug Objects");

    // Set up canvas
    auto canvas = std::make_shared<CanvasComponent>(m_engine);
    canvas->setName("Debug Canvas");
    auto textProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/text.frag",
        ":/shaders/text.vert");
    m_textCanvas = { canvas, textProgram, canvas };
    std::shared_ptr<Label> mainLabel = std::make_shared<Label>(canvas.get(), "Debug View",
        "arial", 30);
    mainLabel->setToGui();
    mainLabel->setFaceCamera(true);
    mainLabel->setOnTop(true);
    mainLabel->setCoordinates({ 0.0f, 0.95f });
    mainLabel->setAlignment(Glyph::kMiddle, Glyph::kCenter);
    canvas->addGlyph(mainLabel);

    m_fpsCounter = std::make_shared<Label>(canvas.get(), "0",
        "courier", 30);
    m_fpsCounter->setToGui();
    m_fpsCounter->setFaceCamera(true);
    m_fpsCounter->setOnTop(true);
    m_fpsCounter->setCoordinates({ -0.97f, 0.95f });
    m_fpsCounter->setColor(Color(Vector3g(0.4f, 0.2f, 0.7f)));
    m_fpsCounter->setAlignment(Glyph::kMiddle, Glyph::kLeft);
    canvas->addGlyph(m_fpsCounter);

    // Set up grid
    // TODO: Use actual infinite ground plane
    // https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders
    m_lineShaderProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/lines.frag",
        ":/shaders/lines.vert");
    auto grid = std::make_shared<DebugGrid>();
    m_staticRenderables["grid"] = { grid, m_lineShaderProgram, grid };

    // Set up a line plane
    auto plane = Lines::getPlane(1.0, 1000);
    m_dynamicRenderables["plane"] = { plane, m_lineShaderProgram, plane };

    // Set up line and point box
    auto box = Lines::getCube();
    m_dynamicRenderables["cube"] = { box, m_lineShaderProgram, box };

    std::shared_ptr<Points> boxPoints = std::make_shared<Points>(*box);
    m_dynamicRenderables["pointCube"] = { boxPoints , m_pointShaderProgram, boxPoints };

    // Set up line and point sphere
    auto sphere = Lines::getSphere(20, 30);
    m_dynamicRenderables["sphere"] = { sphere, m_lineShaderProgram, sphere };

    std::shared_ptr<Points> spherePoints = std::make_shared<Points>(*sphere);
    m_dynamicRenderables["pointSphere"] = { spherePoints , m_pointShaderProgram, spherePoints };

    // Set up simple shader program
    m_simpleShaderProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/simple.frag",
        ":/shaders/simple.vert");

    // Set up point shader program
    m_pointShaderProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/points.frag",
        ":/shaders/points.vert");

    // Set up coordinate axes
    m_axisShaderProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/axes.frag",
        ":/shaders/axes.vert");
    auto coordinateAxes = std::make_shared<DebugCoordinateAxes>(m_engine);
    m_dynamicRenderables["coordinateAxes"] = { coordinateAxes, m_axisShaderProgram , coordinateAxes };
    m_staticRenderables["coordinateAxes"] = m_dynamicRenderables["coordinateAxes"];

    // Create debug skeleton program
    m_debugSkeletonProgram = m_engine->resourceCache()->getShaderProgramByFilePath(
        ":/shaders/debug_skeleton.frag",
        ":/shaders/debug_skeleton.vert");

    auto skeletonPoints = std::make_shared<Points>(100);
    skeletonPoints->renderSettings().addSetting(std::make_shared<DepthSetting>(false));
    m_dynamicRenderables["skeletonPoints"] = { skeletonPoints, m_debugSkeletonProgram, skeletonPoints};

    // Set renderable for raycast hit
    auto raycastHit = Lines::getPrism(0.25f, 0.0f, 2.0f, 6, 1);
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

    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::loadFromJson(const QJsonValue & json)
{
    const QJsonObject& object = json.toObject();
    if (object.contains("debugScene")) {
        m_scene->loadFromJson(json["debugScene"]);
        m_cameraObject = m_scene->getSceneObjectByName("Debug Camera");
    }
    else {
        initializeCamera();
        m_cameraObject->loadFromJson(object.value("cameraObject"));
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::processRaycasts() {
    // Generate raycasts
    bool hit;

    if (!m_glWidget->underMouse()) return;

    for (const std::pair<Uuid, std::shared_ptr<Scene>>& scenePair : m_engine->scenario()->getScenes()) {
        // Skip if the scene has no physics
        const std::shared_ptr<Scene>& scene = scenePair.second;
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
    camera()->controller().profile().m_movementTypes = {
        {CameraController::kZoom, true},
        {CameraController::kPan, true},
        {CameraController::kTilt, true},
        {CameraController::kTranslate, true},
        {CameraController::kRotate, true},
        {CameraController::kTilt, true}
    };
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
    Vector4g lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    Vector4g pointColor(0.9f, 0.25f, 0.2f, 0.9f);
    float pointSize = 0.006f;
    float lineThickness = 0.0075f;
    if (!isEnabled) {
        lineThickness = 0.005f;
        lineColor = Vector4g(0.5f, 0.5f, 0.5f, 0.9f);
        pointColor = lineColor;
        pointSize = 0.004f;
    }
    else {
        if (wasHit) {
            lineThickness *= 1.25f;
            lineColor = Vector4g(0.8f, 0.9f, 1.0f, 1.0f);
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
        Vector3g scale = {boxGeometry->hx() * 2.0f, boxGeometry->hy()* 2.0f,  boxGeometry->hz() * 2.0f };
        worldMatrix.addScale(scale);

        // Set line and world space uniforms
        std::shared_ptr<Lines> lines = std::static_pointer_cast<Lines>(
            m_dynamicRenderables["cube"].object());
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
        points->draw(m_pointShaderProgram);
        break;
    }
    case PhysicsGeometry::kSphere: {
        auto sphereGeometry = std::static_pointer_cast<SphereGeometry>(shape.geometry());
        Vector3g scale = { sphereGeometry->radius(), sphereGeometry->radius(),  sphereGeometry->radius()};
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
        points->draw(m_pointShaderProgram);
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
    Vector4g lineColor(0.75f, 0.9f, 1.0f, 0.9f);
    float lineThickness = 0.1f;

    // Set camera uniforms
    m_lineShaderProgram->bind();
    
    // Set world matrix uniform
    float prismLength = 1.0;
    Quaternion rotation = Quaternion::rotationTo(Vector3g(0.0f, 0.0f, 1.0f), hit.normal());
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
    QString coordAxesStr = QStringLiteral("coordinateAxes");
    if (Map::HasKey(m_dynamicRenderables, coordAxesStr)) {
        DebugRenderable* coordAxes = &m_dynamicRenderables[coordAxesStr];
        auto debugAxes = std::static_pointer_cast<DebugCoordinateAxes>(
            coordAxes->m_renderable);
        debugAxes->clear();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawAnimation(BoneAnimationComponent * animComp)
{
    // Bind shader program
    if (!m_debugSkeletonProgram) return;
    m_debugSkeletonProgram->bind();

    auto mesh = animComp->animationController()->getMesh();
    if (!mesh) return;

    // Get renderable
    Points& points = *std::static_pointer_cast<Points>(
        m_dynamicRenderables["skeletonPoints"].m_renderable);

    // Bind uniforms
    animComp->bindUniforms(m_debugSkeletonProgram);
    points.setPointColor({1.0f, 0.0f, 1.0f, 1.0f});
    points.setPointSize(0.005f);
    points.transform().updateWorldMatrix(animComp->sceneObject()->transform()->worldMatrix());
    
    // Get colors
    Skeleton skeleton = mesh->skeleton().prunedBoneSkeleton();
    IKChain chain(skeleton);
    std::vector<Vector4g> colors(points.numPoints());
    size_t idx;
    Vector4g color;
    for (const auto& nodePair : skeleton.nodes()) {
        MeshNode* node = nodePair.second;
        if (!node->hasBone()) continue;

        idx = node->bone().m_index;
       
        IKNode* ikNode = chain.getNode(node->getName());
        if (ikNode->isEndEffector()) {
            color = Vector4g(0.85f, 0.5f, 0.1f, 1.0f);
        }
        else if (ikNode->isRoot()) {
            color = Vector4g(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (ikNode->isSubBase()) {
            color = Vector4g(1.0f, 1.0f, 0.2f, 1.0f);
        }
        else {
            color = Vector4g(0.75f, 0.5f, 1.0f, 1.0f);
        }
        colors[idx] = color;
    }

    m_debugSkeletonProgram->setUniformValue("colors", colors);

    // Draw skeleton points
    points.draw(m_debugSkeletonProgram);
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
    Vector4g lineColor(0.85f, 0.8f, 0.0f, 0.9f);
    float lineThickness = 0.0075f;
    if (!comp->isEnabled()) {
        lineThickness = 0.005f;
        lineColor = Vector4g(0.5f, 0.5f, 0.5f, 0.9f);
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
                auto capsule = std::static_pointer_cast<Mesh>(
                    m_engine->resourceCache()->polygonCache()->getCylinder(
                        radius, halfHeight)->resource(false));
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
            lines->draw(m_lineShaderProgram);

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
            //lines->draw(m_lineShaderProgram);
            break;
        }
        default:
            break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////
void DebugManager::drawLight(Light * light)
{
    if (!m_simpleShaderProgram) return;

    // Get color and world matrix uniforms
    Vector4g color = light->getDiffuseColor().toVector4g();
    if (!light->isEnabled()) {
        color = Vector4g(0.5f, 0.5f, 0.5f, 0.75f);
    }
    Matrix4x4g worldMatrix = light->sceneObject()->transform()->rotationTranslationMatrix();

    // Set camera uniforms
    m_simpleShaderProgram->bind();
    
    // Get geometry and set uniforms
    const auto& sphereHandle = m_engine->resourceCache()->polygonCache()->getSphere(20, 30);
    
    m_simpleShaderProgram->setUniformValue("worldMatrix", worldMatrix);
    m_simpleShaderProgram->setUniformValue("color", color, true);

    // Draw outer capsule (with contact offset)
    auto mesh = std::static_pointer_cast<Mesh>(sphereHandle->resource(false));
    mesh->meshData().begin()->second->drawGeometry(GL_TRIANGLES);
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces


#pragma once

// Standard
#include <memory>

// QT
#include <QElapsedTimer>

// Internal
#include "core/GManager.h"
#include "core/mixins/GRenderable.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/containers/GStrictGrowContainer.h"

namespace rev {

class Color;
class CoreEngine;
class InputHandler;
class SceneObject;
class Scene;
class Camera;
class CameraComponent;
class CanvasComponent;
class TransformComponent;
class BoneAnimationComponent;
class CharControlComponent;
class Renderer;
class Mesh;
class ShaderProgram;
class Lines;
class LightComponent;
class Label;
class PhysicsShapePrefab;
class PhysicsRaycast;
class SortingLayer;
class AABB;
class DrawCommand;
class RenderContext;
class WorldRay;
struct WorldRayHit;
class GLWidget;
class OpenGlRenderer;

/// @class DebugCoordinateAxes
/// @brief Represents set of coordinate axes to render for debug objects
class DebugCoordinateAxes : public Renderable {
public:
    /// @name Constructors/Destructor
    /// @{c

    DebugCoordinateAxes(CoreEngine* engine);
    virtual ~DebugCoordinateAxes();

    /// @}

    /// @name Public Methods
    /// @{

    void setTransform(TransformInterface* transform);

    virtual size_t getSortID() override { return 0; }

    /// @brief Clear all transient data from the coordinate axes
    void clear();

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    /// @name Protected members
    /// @{

    struct AxesUniforms {
        std::array<UniformData, 3> m_axisIndex;
        std::array<UniformData, 3> m_axisColor;
        std::array<UniformData, 3> m_coneTransforms;
        std::array<UniformData, 3> m_cylinderTransforms;
        UniformData m_worldMatrix; ///< The world matrix of the axes
        UniformData m_false; ///< Uniform data for a true value @todo Make static 
        UniformData m_true; ///< Uniform data for a false value @todo Make static
    };

    AxesUniforms m_uniforms;
    bool m_setUniforms;
    Mesh* m_cone; ///< Cone for arrows
    Mesh* m_cylinder; ///< Cylinder for axes
    float m_coneHeight;
    float m_columnHeight;

    /// @brief Transforms for each of the cones and cylinders
    RenderContext& m_renderContext;
    std::shared_ptr<Transform> m_xConeTransform;
    std::shared_ptr<Transform> m_yConeTransform;
    std::shared_ptr<Transform> m_zConeTransform;
    std::shared_ptr<Transform> m_xCylinderTransform;
    std::shared_ptr<Transform> m_yCylinderTransform;
    std::shared_ptr<Transform> m_zCylinderTransform;
    TransformInterface* m_transform = nullptr;

    /// @}
};

/// @class DebugGrid
/// @brief Represents a 2-dimensional entity to display on a canvas
class DebugGrid : public NameableInterface, public Renderable {
public:
    /// @name Constructors/Destructor
    /// @{

    DebugGrid(CoreEngine* core);
    virtual ~DebugGrid();

    /// @}

    /// @name Public Methods
    /// @{

    virtual size_t getSortID() override { return 0; }

    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(rev::ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(rev::ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    /// @name Protected members
    /// @{


    struct GridUniforms {
        UniformData m_lineColor;
    };

    GridUniforms m_uniforms;
    CoreEngine* m_engine;
    std::shared_ptr<Lines> m_grid; ///< Lines object for a single grid

    /// @}
};

enum class DebugDrawFlag {
    kDrawAxes = 1 << 0,
    kDrawGrid = 1 << 1,
    kDrawBoundingBoxes = 1 << 2,
    kDrawCameraFrustums = 1 << 3,
    kDrawShadowFrustums = 1 << 4 // Whether or not to draw frustums for shadow map cameras
};
MAKE_FLAGS(DebugDrawFlag, DebugDrawFlags)
MAKE_BITWISE(DebugDrawFlag)

/// @class DebugDrawSettings
class DebugDrawSettings {
public:
    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const DebugDrawSettings& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, DebugDrawSettings& orObject);


    /// @}

    DebugDrawFlags m_drawFlags;
};


/// @class DebugManager 
class DebugManager: public Manager {
    Q_OBJECT
public:
    /// @name Static
    /// @{
   
    /// @brief Struct containing a debug renderable
    /// @details The renderable has an optional duration attribute, which can be set to allow for temporary drawing
    struct DebugRenderable {
        /// @name Constructors/Destructor
        /// @{
        DebugRenderable() {
        }
        DebugRenderable(const std::shared_ptr<Renderable>& renderable,
            ShaderProgram* shaderProgram,
            const std::shared_ptr<SceneObject> so,
            float duration = -1):
            DebugRenderable(renderable, shaderProgram, duration)
        {
            m_sceneObject = so;
            m_hasSceneObject = true;
        }
        DebugRenderable(const std::shared_ptr<Renderable>& renderable, 
            ShaderProgram* shaderProgram,
            float duration = -1):
            m_renderable(renderable),
            m_shaderProgram(shaderProgram),
            m_durationInSecs(duration)
        {
            if (m_durationInSecs > 0.0f) {
                m_timer.start();
            }
        }
        ~DebugRenderable() {

        }

        /// @}


        /// @name Methods
        /// @{

        /// @brief Return scene object associated with the renderable
        std::shared_ptr<SceneObject> object() const {
            if (std::shared_ptr<SceneObject> strongPtr = m_sceneObject.lock()) {
                return strongPtr;
            }
            else {
                return nullptr;
            }
        }

        /// @brief Is done drawing
        bool isDone() const{
            if(m_durationInSecs > 0 && (m_timer.elapsed()/1000.0) > m_durationInSecs){
                return true;
            }
            else {
                return false;
            }
        }

        /// @brief Draw the debug renderable
        void draw();

        /// @}

        /// @name Members
        /// @{

        bool m_hasSceneObject{ false };
        std::shared_ptr<Renderable> m_renderable{nullptr}; ///< The renderable
        ShaderProgram* m_shaderProgram{nullptr};
        std::weak_ptr<SceneObject> m_sceneObject;
        float m_durationInSecs; ///< Duration of the renderable in seconds
        QElapsedTimer m_timer;
        bool m_enabled;

        /// @}
    };


    /// @}

	/// @name Constructors/Destructor
	/// @{
    DebugManager(CoreEngine* engine);
	~DebugManager();
	/// @}

    /// @name Properties
    /// @{

    DebugDrawSettings& debugSettings() { return m_settings; }

    const std::shared_ptr<SortingLayer>& debugRenderLayer() const { return m_debugRenderLayer; }


    CameraComponent* camera();
    CanvasComponent* textCanvas();
    std::shared_ptr<SceneObject> cameraObject() { return m_cameraObject; }
    std::shared_ptr<Scene> scene() { return m_scene; }

    /// @}

	/// @name Public Methods
	/// @{

    /// @brief Clear any data that needs to be reset on scenario load
    void clear();

    /// @brief Routines to add debug objects (doodads)
    // TODO: Implement

    /// @brief Rendering routines
    void draw(Scene* scene);
    void drawStatic();
    void drawDynamic(Scene* scene);
    void drawDynamic(const std::shared_ptr<SceneObject>& so);
    void drawDoodads();

    /// @brief Creat deferred draw commands
    void createDrawCommands(Scene&, OpenGlRenderer& renderer);

    /// @brief Step forward for debug functionality
    void step(double deltaSec);

    /// @brief Called after construction
    virtual void postConstruction() override final;

    /// @brief Add grid
    /// See: https://gamedev.stackexchange.com/questions/141916/antialiasing-shader-grid-lines

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const DebugManager& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, DebugManager& orObject);


    /// @}


protected:

    /// @name Protected Methods
    /// @{

    /// @brief Process raycasts made between each frame
    void processRaycasts();

    void processInputs();

    void initializeCamera();
    void initializeCanvas();

    InputHandler& inputHandler() const;

    RenderContext& mainRenderContext();


    //void getGlyphs(const std::shared_ptr<SceneObject>& object);
    //void removeLabel(const std::shared_ptr<SceneObject>& object);

    float getFps() const;


    void drawPhysicsShape(Int32_t index, const PhysicsShapePrefab& shape, const TransformComponent& translation, bool isEnabled, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawPhysicsRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void drawRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawAnimation(BoneAnimationComponent* comp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawCharacterController(CharControlComponent* comp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawLight(LightComponent* light);

    void createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, float lineThickness, const Vector4& color);
    
    /// @brief Create a command to draw a camera frustum
    void createDrawFrustumCommand(const Camera* cam, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, const Color& color);

    void clearCounts();

    /// @}

    /// @name Protected Members
    /// @{

    struct DebugUniforms {
        std::vector<UniformData> m_lightWorldMatrices;
        std::vector<UniformData> m_lightColors;
        UniformData m_gridWorldMatrix;
        StrictGrowContainer<std::vector<UniformData>> m_physicsWorldMatrices; ///< For various physics debug renderables
        UniformData m_raycastWorldMatrix;
        UniformData m_physicsRaycastWorldMatrix;
        StrictGrowContainer<std::vector<UniformData>> m_animationWorldMatrices; ///< For various animation debug renderables
        StrictGrowContainer<std::vector<UniformData>> m_animationColors; ///< For various animation debug renderables
        StrictGrowContainer<std::vector<UniformData>> m_charControllerWorldMatrices; ///< For character controller animation debug renderables
        StrictGrowContainer<std::vector<UniformData>> m_boxWorldMatrices; ///< For various box debug renderables
        StrictGrowContainer<std::vector<UniformData>> m_frustumWorldMatrices; ///< For various frustum debug renderables
    };
    DebugUniforms m_debugUniforms; ///< Uniforms used to render debug objects
    
    // Counts for debug renderables to access uniforms
    Int32_t m_animationCount{ 0 };
    Int32_t m_charControllerCount{ 0 };
    Int32_t m_boxCount{ 0 };
    Int32_t m_frustumCount{ 0 };

    /// @brief Pointer to the main GL widget
    GLWidget* m_glWidget;

    /// @brief Debug scene
    std::shared_ptr<Scene> m_scene;

    /// @brief Selected scene objects
    std::vector<std::shared_ptr<SceneObject>> m_selectedSceneObjects;

    /// @brief Camera object
    std::shared_ptr<SceneObject> m_cameraObject;

    /// @brief Canvas object
    std::shared_ptr<SceneObject> m_canvasObject;

    /// @brief Canvas Renderable
    std::shared_ptr<Label> m_fpsCounter;

    /// @brief All renderables to be drawn for a given scene object or component type
    tsl::robin_map<QString, DebugRenderable> m_dynamicRenderables;

    /// @brief Camera frustums to be drawn
    std::vector<DebugRenderable> m_cameraFrustumRenderables;

    /// @brief All default debug renderables to be drawn
    tsl::robin_map<QString, DebugRenderable> m_staticRenderables;

    /// @brief All user-defined renderables, with possible transience
    std::vector<DebugRenderable> m_doodads;

    /// @brief Raycast for the current frame
    std::unique_ptr<WorldRay> m_raycast;
    std::vector<WorldRayHit> m_hits;

    /// @brief PhysX-based raycast for the current frame
    std::unique_ptr<PhysicsRaycast> m_physicsRaycast;

    ShaderProgram* m_frustumLineShaderProgram;
    ShaderProgram* m_lineShaderProgram; ///< Line shader program
    ShaderProgram* m_simpleShaderProgram;
    ShaderProgram* m_axisShaderProgram;
    ShaderProgram* m_pointShaderProgram;
    ShaderProgram* m_debugSkeletonProgram;

    /// @brief Sphere mesh to avoid querying polygon cache every frame
    Mesh* m_sphereMesh = nullptr;

    /// @brief Configuration settings
    DebugDrawSettings m_settings;

    /// @brief List of recent frame deltas in seconds
    std::list<float> m_frameDeltas;

    std::shared_ptr<SortingLayer> m_debugRenderLayer;

    /// @}
};



} // End namespaces

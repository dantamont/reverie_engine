/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_DEBUG_MANAGER_H
#define GB_DEBUG_MANAGER_H

// Standard
#include <memory>

// QT

// Internal
#include "../GManager.h"
#include "../mixins/GRenderable.h"
#include "../containers/GContainerExtensions.h"
#include <core/containers/GFlags.h>

namespace rev {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class CoreEngine;
class InputHandler;
class SceneObject;
class Scene;
class CameraComponent;
class CanvasComponent;
class Transform;
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
struct SortingLayer;
class AABB;
class DrawCommand;
class RenderContext;
class WorldRay;
struct WorldRayHit;
namespace View {
class GLWidget;
}
class MainRenderer;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class DebugCoordinateAxes
/// @brief Represents set of coordinate axes to render for debug objects
class DebugCoordinateAxes : public Object, public Renderable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{c

    DebugCoordinateAxes(CoreEngine* engine);
    virtual ~DebugCoordinateAxes();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void setTransform(Transform* transform);

    virtual size_t getSortID() override { return 0; }

    /// @brief Clear all transient data from the coordinate axes
    void clear();

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    bool m_setUniforms;

    /// @brief Cone for arrows
    Mesh* m_cone;

    /// @brief Cylinder for axes
    Mesh* m_cylinder;

    float m_coneHeight;
    float m_columnHeight;

    /// @brief Transforms for each of the cones and cylinders
    std::shared_ptr<Transform> m_xConeTransform;
    std::shared_ptr<Transform> m_yConeTransform;
    std::shared_ptr<Transform> m_zConeTransform;
    std::shared_ptr<Transform> m_xCylinderTransform;
    std::shared_ptr<Transform> m_yCylinderTransform;
    std::shared_ptr<Transform> m_zCylinderTransform;

    Transform* m_transform = nullptr;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DebugGrid
/// @brief Represents a 2-dimensional entity to display on a canvas
class DebugGrid : public Object, public Nameable, public Renderable {

public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    DebugGrid(CoreEngine* core);
    virtual ~DebugGrid();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual size_t getSortID() override { return 0; }

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(rev::ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(rev::ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    CoreEngine* m_engine;

    /// @brief Lines object for a single grid
    std::shared_ptr<Lines> m_grid;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

enum class DebugDrawFlag {
    kDrawAxes = 1 << 0,
    kDrawGrid = 1 << 1,
    kDrawBoundingBoxes = 1 << 2
};
MAKE_FLAGS(DebugDrawFlag, DebugDrawFlags)
MAKE_BITWISE(DebugDrawFlag)

/// @class DebugDrawSettings
class DebugDrawSettings: public Serializable {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

    DebugDrawFlags m_drawFlags;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DebugManager 
class DebugManager: public Manager, public Serializable {
    Q_OBJECT
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
   
    /// @brief Struct containing a debug renderable
    /// @details The renderable has an optional duration attribute, which can be set to allow for temporary drawing
    struct DebugRenderable {
        //--------------------------------------------------------------------------------------------
        /// @name Constructors/Destructor
        /// @{
        DebugRenderable() {
        }
        DebugRenderable(const std::shared_ptr<Renderable>& renderable, 
            ShaderProgram* shaderProgram,
            const std::shared_ptr<Object>& object,
            float duration = -1):
            m_renderable(renderable),
            m_shaderProgram(shaderProgram),
            m_object(object),
            m_durationInSecs(duration)
        {
            if (m_durationInSecs > 0.0f) {
                m_timer.start();
            }
        }
        ~DebugRenderable() {

        }

        /// @}


        //--------------------------------------------------------------------------------------------
        /// @name Methods
        /// @{

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

        /// @brief Obtain the object corresponding to the debug renderable
        std::shared_ptr<Object> object() const{
            if (std::shared_ptr<Object> strongPtr = m_object.lock()) {
                return strongPtr;
            }
            else {
                return nullptr;
            }
        }

        /// @}

        //--------------------------------------------------------------------------------------------
        /// @name Members
        /// @{

        /// @brief the renderable
        std::shared_ptr<Renderable> m_renderable;

        ShaderProgram* m_shaderProgram;

        /// @brief The object represented by the debug renderable
        std::weak_ptr<Object> m_object;

        /// @brief Duration of the renderable in seconds
        float m_durationInSecs;

        QElapsedTimer m_timer;

        bool m_enabled;

        /// @}
    };


    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    DebugManager(CoreEngine* engine);
	~DebugManager();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    DebugDrawSettings& debugSettings() { return m_settings; }

    const std::shared_ptr<SortingLayer>& debugRenderLayer() const { return m_debugRenderLayer; }


    CameraComponent* camera();
    CanvasComponent* textCanvas();
    std::shared_ptr<SceneObject> cameraObject() { return m_cameraObject; }
    std::shared_ptr<Scene> scene() { return m_scene; }

    /// @}

	//--------------------------------------------------------------------------------------------
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
    void createDrawCommands(Scene&, MainRenderer& renderer);

    /// @brief Step forward for debug functionality
    void step(unsigned long deltaMs);

    /// @brief Called after construction
    virtual void postConstruction() override final;

    /// @brief Add grid
    /// See: https://gamedev.stackexchange.com/questions/141916/antialiasing-shader-grid-lines

	/// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "DebugManager"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::DebugManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

signals:

    /// @brief Emit on selection of a scene object
    void selectedSceneObject(size_t sceneObjectID);

public slots:

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
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


    void drawPhysicsShape(const PhysicsShapePrefab& shape, const TransformComponent& translation, bool isEnabled, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawPhysicsRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void drawRaycastContact(std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawAnimation(BoneAnimationComponent* comp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawCharacterController(CharControlComponent* comp, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    void drawLight(LightComponent* light);

    void createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);
    void createDrawBoxCommand(const AABB& box, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands, float lineThickness, const Vector4& color);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the main GL widget
    View::GLWidget* m_glWidget;

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

    /// @brief All default debug renderables to be drawn
    tsl::robin_map<QString, DebugRenderable> m_staticRenderables;

    /// @brief All user-defined renderables, with possible transience
    std::vector<DebugRenderable> m_doodads;

    /// @brief Raycast for the current frame
    std::unique_ptr<WorldRay> m_raycast;
    std::vector<WorldRayHit> m_hits;

    /// @brief PhysX-based raycast for the current frame
    std::unique_ptr<PhysicsRaycast> m_physicsRaycast;

    /// @brief Line shader program
    ShaderProgram* m_lineShaderProgram;
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


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
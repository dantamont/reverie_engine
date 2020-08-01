/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_DEBUG_MANAGER_H
#define GB_DEBUG_MANAGER_H

// Standard
#include <memory>

// QT

// Internal
#include "../GbManager.h"
#include "../mixins/GbRenderable.h"
#include "../containers/GbContainerExtensions.h"

namespace Gb {


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
class Raycast;
struct SortingLayer;
class AABB;
class DrawCommand;
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
    /// @{

    DebugCoordinateAxes(CoreEngine* engine);
    virtual ~DebugCoordinateAxes();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void setTransform(const std::shared_ptr<Transform>& transform);

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
    std::shared_ptr<Mesh> m_cone;

    /// @brief Cylinder for axes
    std::shared_ptr<Mesh> m_cylinder;

    float m_coneHeight;
    float m_columnHeight;

    /// @brief Transforms for each of the cones and cylinders
    std::shared_ptr<Transform> m_xConeTransform;
    std::shared_ptr<Transform> m_yConeTransform;
    std::shared_ptr<Transform> m_zConeTransform;
    std::shared_ptr<Transform> m_xCylinderTransform;
    std::shared_ptr<Transform> m_yCylinderTransform;
    std::shared_ptr<Transform> m_zCylinderTransform;

    std::shared_ptr<Transform> m_transform = nullptr;

    /// @}
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class DebugGrid
/// @brief Represents a 2-dimensional entity to display on a canvas
class DebugGrid : public Object, public Renderable {

public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    DebugGrid();
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
    virtual void bindUniforms(Gb::ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(Gb::ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Lines object for a single grid
    std::shared_ptr<Lines> m_grid;

    /// @}
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
            const std::shared_ptr<ShaderProgram>& shaderProgram,
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
        void draw() {
            m_renderable->draw(*m_shaderProgram);
        }

        inline void drawStatic()
        {
        }

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

        std::shared_ptr<ShaderProgram> m_shaderProgram;

        /// @brief The object represented by the debug renderable
        std::weak_ptr<Object> m_object;

        /// @brief Duration of the renderable in seconds
        float m_durationInSecs;

        QElapsedTimer m_timer;

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

    const std::shared_ptr<SortingLayer>& debugRenderLayer() const { return m_debugRenderLayer; }


    CameraComponent* camera();
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
    void draw(const std::shared_ptr<Scene>& scene);
    void drawStatic();
    void drawDynamic(const std::shared_ptr<Scene>& scene);
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
    virtual const char* namespaceName() const { return "Gb::DebugManager"; }
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

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

    void initializeCamera();

    InputHandler& inputHandler() const;


    //void getGlyphs(const std::shared_ptr<SceneObject>& object);
    //void removeLabel(const std::shared_ptr<SceneObject>& object);

    float getFps() const;


    void drawPhysicsShape(const PhysicsShapePrefab& shape, const TransformComponent& translation, bool isEnabled);

    void drawRaycastContact();

    void drawAnimation(BoneAnimationComponent* comp);

    void drawCharacterController(CharControlComponent* comp);

    void drawLight(LightComponent* light);

    void createDrawCommand(SceneObject& so, std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the main GL widget
    View::GLWidget* m_glWidget;

    /// @brief Debug scene
    std::shared_ptr<Scene> m_scene;

    /// @brief Camera object
    std::shared_ptr<SceneObject> m_cameraObject;

    /// @brief Canvas Renderable
    std::shared_ptr<CanvasComponent> m_textCanvas;
    std::shared_ptr<Label> m_fpsCounter;

    /// @brief All renderables to be drawn for a given scene object or component type
    std::unordered_map<QString, DebugRenderable> m_dynamicRenderables;

    /// @brief All default debug renderables to be drawn
    std::unordered_map<QString, DebugRenderable> m_staticRenderables;

    /// @brief All user-defined renderables, with possible transience
    std::vector<DebugRenderable> m_doodads;

    /// @brief Raycast for the current frame
    std::unique_ptr<Raycast> m_raycast;

    /// @brief Line shader program
    std::shared_ptr<ShaderProgram> m_lineShaderProgram;
    std::shared_ptr<ShaderProgram> m_simpleShaderProgram;
    std::shared_ptr<ShaderProgram> m_axisShaderProgram;
    std::shared_ptr<ShaderProgram> m_pointShaderProgram;
    std::shared_ptr<ShaderProgram> m_debugSkeletonProgram;

    /// @brief List of recent frame deltas in seconds
    std::list<float> m_frameDeltas;

    std::shared_ptr<SortingLayer> m_debugRenderLayer;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
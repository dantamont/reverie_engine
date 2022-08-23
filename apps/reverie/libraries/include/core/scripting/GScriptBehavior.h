#pragma once

namespace rev {

class SceneObject;
class InputHandler;
class CoreEngine;
class ResourceCache;
class TransformComponent;
class Scene;

/// @class ScriptBehavior
/// @brief Class to get wrapped up by a script to be called via python or another scripting language
class ScriptBehavior {
public:

    /// @name Constructors/Destructor
    /// @{
    ScriptBehavior(SceneObject& sceneObject);
    virtual ~ScriptBehavior();
    /// @}

    /// @name Properties
    /// @{

    SceneObject& sceneObject() { return *m_sceneObject; }
    void setSceneObject(SceneObject& so) { m_sceneObject = &so; }

    InputHandler* inputHandler();
    CoreEngine* engine();
    ResourceCache* resourceCache();
    const TransformComponent* transform();
    Scene* scene();

    /// @}

    /// @name Public Methods
    /// @{
    
    virtual void initialize();
    virtual void update(double deltaSec);
    virtual void lateUpdate(double deltaSec);
    virtual void fixedUpdate(double deltaSec);
    virtual void onSuccess();
    virtual void onFail();
    virtual void onAbort();

    /// @}

private:
    friend class PyScriptBehavior;

    CoreEngine* m_engine;
    SceneObject* m_sceneObject;
    Scene* m_scene;


    static bool s_logMessages;
};


} // End namespaces

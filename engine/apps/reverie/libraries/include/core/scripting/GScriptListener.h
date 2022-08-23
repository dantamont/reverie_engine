#pragma once

namespace rev {

class SceneObject;
class InputHandler;
class CoreEngine;
class ResourceCache;
class TransformComponent;
class Scene;
class CustomEvent;

/// @class ScriptListener
/// @brief Class to get wrapped up by a script to be called via python or another scripting language
class ScriptListener {
public:

    /// @name Constructors/Destructor
    /// @{
    ScriptListener(SceneObject& sceneObject);
    virtual ~ScriptListener();
    /// @}

    /// @name Properties
    /// @{

    SceneObject& sceneObject() { return *m_sceneObject; }
    void setSceneObject(SceneObject& so) { m_sceneObject = &so; }

    InputHandler* inputHandler();
    CoreEngine* engine();
    ResourceCache* resourceCache();
    TransformComponent* transform();
    Scene* scene();

    /// @}

    /// @name Public Methods
    /// @{
    
    virtual bool eventTest(const CustomEvent& ev);
    virtual void perform(const CustomEvent& ev);

    /// @}

private:
    friend class PyScriptListener;

    CoreEngine* m_engine;
    SceneObject* m_sceneObject;
    Scene* m_scene;


    static bool s_logMessages;
};


} // End namespaces

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SCRIPT_BEHAVIOR_H
#define GB_SCRIPT_BEHAVIOR_H

// QT

// Internal
#include "../GbObject.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class SceneObject;
class InputHandler;
class CoreEngine;
class ResourceCache;
class TransformComponent;
class Scene;

/////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ScriptBehavior
/// @brief Class to get wrapped up by a script to be called via python or another scripting language
class ScriptBehavior: public Object {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    ScriptBehavior(SceneObject& sceneObject);
    virtual ~ScriptBehavior();
    /// @}

    //--------------------------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{
    
    virtual void initialize();
    virtual void update(unsigned long deltaMs);
    virtual void lateUpdate(unsigned long deltaMs);
    virtual void fixedUpdate(unsigned long deltaMs);
    virtual void onSuccess();
    virtual void onFail();
    virtual void onAbort();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------    
    /// @name Object Properties
    /// @{
    /// @property className
    const char* className() const override { return "ScriptBehavior"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "Gb::ScriptBehavior"; }
    /// @}

private:
    friend class PyScriptBehavior;

    CoreEngine* m_engine;
    SceneObject* m_sceneObject;
    Scene* m_scene;


    static bool s_logMessages;
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
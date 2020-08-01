#ifndef GB_COMPONENT_WIDGETS_H
#define GB_COMPONENT_WIDGETS_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../parameters/GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class Serializable;
class Component;
class ScriptComponent;
class Renderer;
class ShaderComponent;
class TransformComponent;
enum class RotationSpace;
class RigidBodyComponent;
class CharControlComponent;
class PhysicsShapePrefab;
class Light;
class CameraComponent;

namespace View {

class PhysicsShapeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ComponentWidget
/// @brief Base class for all component widgets
class ComponentWidget : public ParameterWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

public slots:

signals:

    /// @brief Toggled component
    void toggled(int state);

protected slots:


protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    QCheckBox* m_toggle;
    QLabel* m_typeLabel;

    Component* m_component;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ComponentJsonWidget
/// @brief Subclass of a JSON Widget that is specific to components
class ComponentJsonWidget : public JsonWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ComponentJsonWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ComponentJsonWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}


protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    Component* component();

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() override;

    /// @brief What do do after reloading serializable
    virtual void postLoad() override;

    virtual void layoutWidgets() override;


    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    bool m_wasPlaying;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class GenericComponentWidget
/// @brief Generic component widget that uses JSON data to modify components
class GenericComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    GenericComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~GenericComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

protected:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    ComponentJsonWidget* m_jsonWidget;

    /// @}
};




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
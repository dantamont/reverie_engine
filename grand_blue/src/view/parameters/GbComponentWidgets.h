#ifndef GB_COMPONENT_WIDGETS_H
#define GB_COMPONENT_WIDGETS_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class Serializable;
class CoreEngine;
class Component;
class ScriptComponent;
class Renderer;
class RendererComponent;

namespace View {

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
    /// @name Private Methods
    /// @{

    Component* component();

    /// @brief What to do prior to reloading serializable
    virtual void preLoad() override;

    /// @brief What do do after reloading serializable
    virtual void postLoad() override;

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
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScriptWidget
/// @brief Widget representing a script component
class ScriptWidget : public ComponentWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ScriptWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ScriptWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @}

public slots:

signals:

private slots:


private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a script component
    ScriptComponent* scriptComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Path to the python script used by this component
    FileLoadWidget* m_fileWidget;
    QPushButton* m_confirmButton;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
#ifndef GB_MODEL_COMPONENT_WIDGET_H
#define GB_MODEL_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidgets.h"
#include "../parameters/GRenderableWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class ModelComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ModelSelectWidget
class ModelSelectWidget : public ParameterWidget {
    Q_OBJECT
public:
    ModelSelectWidget(CoreEngine* core, ModelComponent* modelComponent, QWidget* parent = nullptr);

    virtual void update() override;

public slots:
    void populateModels();

protected:

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    ModelComponent* m_modelComponent;
    QComboBox* m_modelSelect;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ModelComponentWidget
/// @brief Widget representing a character controller component
class ModelComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ModelComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~ModelComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a shader component
    ModelComponent* modelComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    ModelComponent* m_modelComponent;

    //RenderableWidget<ModelComponent>* m_renderable;
    RenderSettingsWidget* m_renderSettings;

    ModelSelectWidget* m_model;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
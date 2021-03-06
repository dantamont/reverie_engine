#ifndef GB_CUBEMAP_COMPONENT_WIDGET_H
#define GB_CUBEMAP_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidgets.h"
#include "../parameters/GRenderableWidget.h"
#include "../parameters/GParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CubeMapComponent;

namespace View {

class ColorWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ModelSelectWidget
//class ModelSelectWidget : public ParameterWidget {
//    Q_OBJECT
//public:
//    ModelSelectWidget(CoreEngine* core, ModelComponent* modelComponent, QWidget* parent = nullptr);
//
//    virtual void update() override;
//
//public slots:
//    void populateModels();
//
//protected:
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    ModelComponent* m_modelComponent;
//    QComboBox* m_modelSelect;
//};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CubeMapComponentWidget
/// @brief Widget representing a character controller component
class CubeMapComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    CubeMapComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~CubeMapComponentWidget();

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
    CubeMapComponent* cubeMapComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    CubeMapComponent* m_cubeMapComponent;

    //RenderableWidget<ModelComponent>* m_renderable;
    QLineEdit* m_nameEdit;
    QCheckBox* m_isDefaultCubeMap;
    FileLoadWidget* m_textureLoad;
    ColorWidget* m_colorWidget;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
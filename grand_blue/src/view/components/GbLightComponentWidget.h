#ifndef GB_LIGHT_COMPONENT_WIDGET_H
#define GB_LIGHT_COMPONENT_WIDGET_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"
#include "../parameters/GbVectorWidget.h"
#include "../parameters/GbColorWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class LightComponent;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightDiffuseColorWidget
class LightDiffuseColorWidget : public ColorWidget {
public:
    LightDiffuseColorWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;

protected:

    virtual void updateColor() override;
    virtual void clear() override;

    LightComponent* m_lightComponent;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightAmbientColorWidget
class LightAmbientColorWidget : public ColorWidget {
public:
    LightAmbientColorWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;
    virtual void clear() override;

protected:

    virtual void updateColor() override;

    LightComponent* m_lightComponent;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightSpecularColorWidget
class LightSpecularColorWidget : public ColorWidget {
public:
    LightSpecularColorWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;
    virtual void clear() override;

protected:

    virtual void updateColor() override;

    LightComponent* m_lightComponent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightDirectionWidget
class LightDirectionWidget : public VectorWidget<float, 4> {
public:
    LightDirectionWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;
    virtual void clear() override;

protected:

    virtual void updateVector() override;

    LightComponent* m_lightComponent;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightAttenuationWidget
class LightAttenuationWidget : public VectorWidget<float, 4> {
public:
    LightAttenuationWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;
    virtual void clear() override;

protected:

    virtual void updateVector() override;

    LightComponent* m_lightComponent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightCutoffWidget
class LightCutoffWidget : public VectorWidget<float, 4> {
public:
    LightCutoffWidget(CoreEngine* core, LightComponent* light, QWidget* parent = nullptr);

    virtual void update() override;
    virtual void clear() override;

protected:

    virtual void updateVector() override;

    LightComponent* m_lightComponent;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LightComponentWidget
/// @brief Widget representing a light component
class LightComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    LightComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~LightComponentWidget();

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

    /// @brief Return component as a shader component
    LightComponent* lightComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Toggle widgets based on light type
    void toggleWidgets();

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Light type widget
    QComboBox* m_lightType;
    QLineEdit* m_lightIntensity;

    LightDirectionWidget* m_lightDirection; // for directional and spot lights
    LightAttenuationWidget* m_lightAttenuation; // for point light
    LightCutoffWidget* m_lightCutoffs; // for spot light

    LightDiffuseColorWidget*  m_diffuseColorWidget;
    LightAmbientColorWidget*  m_ambientColorWidget;
    LightSpecularColorWidget* m_specularColorWidget;

    /// @brief Stacked layout for different light types
    //QStackedLayout* m_stackedLayout;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
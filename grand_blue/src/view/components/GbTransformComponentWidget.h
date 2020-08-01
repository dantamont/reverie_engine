#ifndef GB_TRANSFORM_COMPONENT_WIDGET_H
#define GB_TRANSFORM_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"
#include "../parameters/GbVectorWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
class Transform;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TranslationWidget
class TranslationWidget : public VectorWidget<double, 3> {
public:
    TranslationWidget(CoreEngine* core, Transform* transform, QWidget* parent = nullptr);

    virtual void update() override;

    /// @brief Replace transform used by the widget
    void setTransform(Transform& transform);

protected:

    virtual void updateVector() override;

    Transform* m_transform;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RotationWidget
class RotationWidget : public ParameterWidget {
public:
    RotationWidget(CoreEngine* core, Transform* transform, QWidget* parent = nullptr);

    virtual void update() override;

    /// @brief Replace transform used by the widget
    void setTransform(Transform& transform);

protected:
    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    virtual void updateRotation();

    virtual void updateWidgetRotation();

    /// @brief Get rotation type from comboboxes
    int getRotationType() const;
    RotationSpace getRotationSpace() const;

    int ensureDifferentAxis(int index);


    QCheckBox* m_isInertial;
    QComboBox* m_axisBox1;
    QComboBox* m_axisBox2;
    QComboBox* m_axisBox3;
    QLineEdit* m_rotEdit1;
    QLineEdit* m_rotEdit2;
    QLineEdit* m_rotEdit3;

    Transform* m_transform;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScaleWidget
class ScaleWidget : public VectorWidget<double, 3> {
public:
    ScaleWidget(CoreEngine* core, Transform* transform, QWidget* parent = nullptr);

    virtual void update() override;

    /// @brief Replace transform used by the widget
    void setTransform(Transform& transform);

protected:
    virtual void updateVector() override;

    Transform* m_transform;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TransformWidget
/// @brief Widget representing a Transform 
class TransformWidget : public ParameterWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    TransformWidget(CoreEngine* core, Transform* transform, QWidget *parent = 0);
    ~TransformWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    virtual void update() override;

    /// @brief Replace transform used by the widget
    void setTransform(Transform& transform);

    /// @}

private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return transform
    Transform* transform() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    Transform* m_transform;

    QTabWidget* m_tabWidget;
    QComboBox* m_inheritanceTypeWidget;
    TranslationWidget* m_translationWidget;

    RotationWidget* m_rotationWidget;

    ScaleWidget* m_scaleWidget;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class TransformComponentWidget
/// @brief Widget representing a Transform component
class TransformComponentWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    TransformComponentWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~TransformComponentWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    virtual void update() override {}

    /// @}

public slots:
signals:
private slots:
private:
    //---------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{

    /// @brief Return component as a shader component
    TransformComponent* transformComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    TransformWidget* m_transformWidget;

    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
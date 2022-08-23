#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "GComponentWidget.h"

#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/time/GExpireTimer.h"

#include "ripple/network/messages/GRequestTransformMessage.h"
#include "ripple/network/messages/GTransformMessage.h"
#include "ripple/network/messages/GTransformUpdateMessage.h"

#include "geppetto/qt/widgets/types/GVectorWidget.h"

namespace rev {

/// @class TranslationWidget
class TranslationWidget : public VectorWidget<Real_t, 3> {
public:
    TranslationWidget(WidgetManager* core, json& transformJson, QWidget* parent = nullptr);

    /// @brief Refresh the widget to represent the value held by the actual transform
    virtual void update() override;

protected:

    /// @brief Update the value of the internal vector based on the widget input values
    virtual void updateVectorFromWidget() override;

    Vector<Real_t, 3> m_vector; /// The vector modified in lieu of JSON, so that VectorWidget can be used
    json& m_transformJson;
};


/// @class RotationWidget
class RotationWidget : public ParameterWidget {
public:
    RotationWidget(WidgetManager* wm, json& transformJson, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @brief Update the rotation of the transform from the widget
    virtual void updateRotationFromWidget();

    /// @brief Update the rotation displayed based on the value in the transform
    virtual void updateWidgetRotation();

    /// @brief Get rotation type from comboboxes
    EulerAngles::EulerType getRotationType() const;

    /// @brief Get rotation space form comboboxes
    RotationSpace getRotationSpace() const;

    int ensureDifferentAxis(int index);


    QCheckBox* m_isInertial{ nullptr };
    QComboBox* m_axisBox1{ nullptr };
    QComboBox* m_axisBox2{ nullptr };
    QComboBox* m_axisBox3{ nullptr };
    QLineEdit* m_rotEdit1{ nullptr };
    QLineEdit* m_rotEdit2{ nullptr };
    QLineEdit* m_rotEdit3{ nullptr };

    json& m_transformJson;
    Vector3 m_lastDegreeAngles; ///< Last explicitly entered values
};


/// @class ScaleWidget
class ScaleWidget : public VectorWidget<Real_t, 3> {
public:
    ScaleWidget(WidgetManager* wm, json& transformJson, QWidget* parent = nullptr);

    virtual void update() override;

protected:
    virtual void updateVectorFromWidget() override;

    Vector<Real_t, 3> m_vector; /// The vector modified in lieu of JSON, so that VectorWidget can be used
    json& m_transformJson;
};


/// @class TransformWidget
/// @brief Widget representing a Transform 
class TransformWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    TransformWidget(WidgetManager* wm, json& transformJson, json metadata, QWidget *parent = 0);
    ~TransformWidget();

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Update the display in widgets
    virtual void update() override;

    /// @brief Update the transform in the main application with the stored JSON value
    void updateTransformFromJson();

    /// @}

public slots:

    /// @brief Update the widget on receipt of an updated transform from the main application
    void update(const GTransformMessage& message);

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    json& m_transformJson; ///< The JSON representing the transform modified by this widget
    GRequestTransformMessage m_requestTransformMessage; ///< Message to request an update to the transform from the main application
    GTransformUpdateMessage m_transformUpdateMessage; ///< Message to send to update transform

    ExpireTimer m_requestUpdateTimer{ 200000 }; ///< Timer for requesting updates at 5Hz

    QTabWidget* m_tabWidget{ nullptr };
    QComboBox* m_inheritanceTypeWidget{ nullptr };

    TranslationWidget* m_translationWidget{ nullptr };
    RotationWidget* m_rotationWidget{ nullptr };
    ScaleWidget* m_scaleWidget{ nullptr };

    /// @}
};


/// @class TransformComponentWidget
/// @brief Widget representing a Transform component
class TransformComponentWidget : public SceneObjectComponentWidget {
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    TransformComponentWidget(WidgetManager* wm, const json& transformJson, Int32_t sceneObjectId, QWidget *parent = 0);
    ~TransformComponentWidget();

    /// @}

    /// @name Public methods
    /// @{

    virtual void update() override {}

    /// @}

private:
    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    TransformWidget* m_transformWidget{ nullptr };

    /// @}
};


// End namespaces
}

#pragma once

// Qt
#include <QStackedLayout>

// Project
#include "geppetto/qt/widgets/types/GParameterWidgets.h"

#include "enums/GRigidBodyTypeEnum.h"
#include "enums/GPhysicsGeometryTypeEnum.h"
#include "ripple/network/messages/GUpdateRigidBodyComponentMessage.h"
#include "ripple/network/messages/GCreatePhysicsShapePrefabMessage.h"
#include "ripple/network/messages/GDeletePhysicsShapePrefabMessage.h"
#include "ripple/network/messages/GUpdatePhysicsShapePrefabMessage.h"

namespace rev {

/// @class PhysicsGeometryWidget
class PhysicsGeometryWidget : public ParameterWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    PhysicsGeometryWidget(WidgetManager* wm, const json& prefabJson, const json& bodyJson, QWidget* parent = nullptr);
    ~PhysicsGeometryWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override {}

    void reinitialize(const json& prefabJson);

    /// @}

signals:

    /// @brief Called when the geometry is switched to a different type
    void switchedGeometry(const QString& prefabName);

protected:

    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    /// @brief Update the geometry of the physics shape
    void updatePhysicsShapeGeometry(GPhysicsGeometryType geometryType);

    /// @brief Send message to update shape prefab in the main application
    void sendUpdatePrefabMessage();

    /// @brief Send message to update the rigid body in the main application
    void sendUpdateRigidBodyComponentMessage();

    /// @}


    /// @name Protected Members
    /// @{
    friend class PhysicsShapeWidget;

    QComboBox* m_typeWidget = nullptr;

    QStackedLayout* m_stackedLayout;
    QWidget* m_boxWidget = nullptr;
    QWidget* m_sphereWidget = nullptr;

    QLabel* m_hxLabel;
    QLineEdit* m_hxLineEdit;
    QLabel* m_hyLabel;
    QLineEdit* m_hyLineEdit;
    QLabel* m_hzLabel;
    QLineEdit* m_hzLineEdit;

    QLabel* m_radLabel;
    QLineEdit* m_radiusLineEdit;

    json m_prefabJson;
    json m_bodyJson;

    /// @}

};


/// @class PhysicsShapeWidget
class PhysicsShapeWidget : public ParameterWidget{
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    PhysicsShapeWidget(WidgetManager* wm, const json& prefabJson, const json& rigidBodyJson, const Uuid& componentId, QWidget* parent = nullptr);
    ~PhysicsShapeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override {}

    PhysicsGeometryWidget* geometryWidget() { return m_geometryWidget; }

    GUpdateRigidBodyComponentMessage& updateRigidBodyComponentMessage() { return m_updateRigidBodyComponentMessage; }
    GUpdatePhysicsShapePrefabMessage& updatePrefabMessage() { return m_updatePrefabMessage; }

    void repopulatePrefabs(bool block = true);
    void repopulateMaterials();

    /// @}

protected slots:

    void showContextMenu(const QPoint&);

protected:

    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    /// @brief Change the prefab being looked at by the widget
    void setCurrentPrefab(const GString& prefabName);

    /// @brief Create a physics shape prefab
    void createPhysicsShapePrefab();

    /// @brief Remove a physics shape prefab
    void removePhysicsShapePrefab();

    /// @}


    /// @name Protected Members
    /// @{

    json m_currentPrefabJson; ///< The JSON representing the prefab currently selcted in a combobox
    json m_bodyJson; ///< The Json representing the rigid body associate with the physics shape

    QComboBox* m_prefabs = nullptr;
    QPushButton* m_addPrefab = nullptr;
    QPushButton* m_removePrefab = nullptr;

    PhysicsGeometryWidget* m_geometryWidget = nullptr;
    QComboBox* m_materialWidget = nullptr;

    GUpdateRigidBodyComponentMessage m_updateRigidBodyComponentMessage; ///< @todo Remove from here, and move logic to rigid body component widget
    GUpdatePhysicsShapePrefabMessage m_updatePrefabMessage;
    GCreatePhysicsShapePrefabMessage m_createPrefabMessage;
    GDeletePhysicsShapePrefabMessage m_deletePrefabMessage;

    /// @}

};

} // rev

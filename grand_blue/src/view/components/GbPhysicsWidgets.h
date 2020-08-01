#ifndef GB_PHYSICS_WIDGETS_H 
#define GB_PHYSICS_WIDGETS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt

// Project
#include "../parameters/GbParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class PhysicsShapePrefab;
class PhysicsGeometry;
class PhysicsMaterial;
class RigidBody;

namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PhysicsGeometryWidget
class PhysicsGeometryWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    PhysicsGeometryWidget(CoreEngine* core,
        const std::shared_ptr<PhysicsShapePrefab>& prefab,
        QWidget* parent = nullptr,
        RigidBody* body = nullptr);
    ~PhysicsGeometryWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {}

    void reinitialize(const std::shared_ptr<PhysicsShapePrefab>& prefab);
    /// @}

signals:

    void switchedGeometry(const QString& prefabName);

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    std::shared_ptr<PhysicsShapePrefab> prefab() const {
        if (std::shared_ptr<PhysicsShapePrefab> fab = m_prefab.lock()) {
            return fab;
        }
        else {
            return nullptr;
        }
    }

    void switchGeometry(int index);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
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

    RigidBody* m_body = nullptr;

    std::weak_ptr<PhysicsShapePrefab> m_prefab;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PhysicsShapeWidget
class PhysicsShapeWidget : public ParameterWidget{
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    PhysicsShapeWidget(CoreEngine* core,
        const std::shared_ptr<PhysicsShapePrefab>& prefab,
        QWidget* parent = nullptr,
        RigidBody* body = nullptr);
    ~PhysicsShapeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {}

    PhysicsGeometryWidget* geometryWidget() { return m_geometryWidget; }


    /// @}

protected slots:

    void showContextMenu(const QPoint&);

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Method to checkValidity widgets
    virtual void initializeWidgets() override;

    /// @brief Methods to checkValidity connections for this widget
    virtual void initializeConnections() override;

    /// @brief Method to ay out widgets
    virtual void layoutWidgets() override;

    std::shared_ptr<PhysicsShapePrefab> currentPrefab() const {
        if (std::shared_ptr<PhysicsShapePrefab> fab = m_currentPrefab.lock()) {
            return fab;
        }
        else {
            return nullptr;
        }
    }

    void repopulatePrefabs(bool block = true);
    void repopulateMaterials();

    void setCurrentPrefab(const std::shared_ptr<PhysicsShapePrefab>& prefab);

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    RigidBody* m_body = nullptr;

    QComboBox* m_prefabs = nullptr;
    //QAction* m_deletePrefabAction = nullptr;
    QPushButton* m_addPrefab = nullptr;
    QPushButton* m_removePrefab = nullptr;

    PhysicsGeometryWidget* m_geometryWidget = nullptr;
    QComboBox* m_materialWidget = nullptr;

    std::weak_ptr<PhysicsShapePrefab> m_currentPrefab;

    /// @}

};





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif
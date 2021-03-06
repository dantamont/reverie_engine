#ifndef GB_PHYSICS_WIDGETS_H 
#define GB_PHYSICS_WIDGETS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt

// Project
#include "../parameters/GParameterWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

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
        PhysicsShapePrefab* prefab,
        QWidget* parent = nullptr,
        RigidBody* body = nullptr);
    ~PhysicsGeometryWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {}

    void reinitialize(PhysicsShapePrefab* prefab);
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

    PhysicsShapePrefab* m_prefab;

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
        PhysicsShapePrefab* prefab,
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

    void repopulatePrefabs(bool block = true);
    void repopulateMaterials();

    void setCurrentPrefab(PhysicsShapePrefab* prefab);

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

    /// @brief The prefab currently selected via combobox
    PhysicsShapePrefab* m_currentPrefab;

    /// @}

};





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif
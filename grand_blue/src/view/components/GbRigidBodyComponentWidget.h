#ifndef GB_RIGID_BODY_COMPONENT_WIDGET_H
#define GB_RIGID_BODY_COMPONENT_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "GbComponentWidgets.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class RigidBodyWidget
/// @brief Widget representing a rigid body component
class RigidBodyWidget : public ComponentWidget {
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    RigidBodyWidget(CoreEngine* core, Component* component, QWidget *parent = 0);
    ~RigidBodyWidget();

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
    RigidBodyComponent* rigidBodyComponent() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    std::shared_ptr<PhysicsShapePrefab> shapePrefab();

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    QComboBox* m_rigidTypeWidget;
    QLineEdit* m_density;
    QCheckBox* m_isKinematic;
    PhysicsShapeWidget* m_shapeWidget;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
#pragma once

// Qt
#include <QtWidgets>

// External
#include "ripple/network/messages/GUpdateRigidBodyComponentMessage.h"

// Internal
#include "GComponentWidget.h"

namespace rev {

class PhysicsShapeWidget;

/// @class RigidBodyWidget
/// @brief Widget representing a rigid body component
class RigidBodyWidget : public SceneObjectComponentWidget {
public:
    /// @name Constructors/Destructor
    /// @{

    RigidBodyWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget *parent = 0);
    ~RigidBodyWidget();

    /// @}

    PhysicsShapeWidget* shapeWidget() { return m_shapeWidget; }

private:

    /// @name Private Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Private Members
    /// @{

    QComboBox* m_rigidTypeWidget;
    QLineEdit* m_density;
    QCheckBox* m_isKinematic;
    PhysicsShapeWidget* m_shapeWidget;

    GUpdateRigidBodyComponentMessage m_updateRigidBodyComponentMessage; ///< Message to send to main application for updating rigid body component

    /// @}
};



// End namespaces        
}
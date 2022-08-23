#pragma once

#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

#include "ripple/network/messages/GCreatePolygonMeshMessage.h"
#include "ripple/network/messages/GLoadModelMessage.h"
#include "ripple/network/messages/GRenameModelMessage.h"

namespace rev {

class WidgetManager;
class ModelResourceWidget;
class FileLoadWidget;

/// @class LoadModelWidget
class LoadModelWidget : public ParameterWidget {
public:
    /// @name Constructors and Destructors
    /// @{
    LoadModelWidget(WidgetManager* manager, QWidget* parent = nullptr);
    virtual ~LoadModelWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:
    friend class LoadModelCommand;

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    Uuid m_modelId{ false }; ///< The UUID of the most recent model loaded with this widget
    FileLoadWidget* m_fileLoadWidget{ nullptr };
    QDialogButtonBox* m_confirmButtons{ nullptr };
    GLoadModelMessage m_loadModelMessage{}; // Message to load model

    /// @}

};



///// @class MeshResourceWidget
//class MeshResourceWidget : public ParameterWidget {
//    Q_OBJECT
//public:
//
//    /// @name Constructors and Destructors
//    /// @{
//    MeshResourceWidget(WidgetManager* manager,
//        const std::shared_ptr<ResourceHandle>& meshHandle,
//        QWidget* parent = nullptr);
//    virtual ~MeshResourceWidget();
//
//    /// @}
//
//    /// @name Public Methods
//    /// @{
//
//    virtual void update() override {
//    }
//
//    /// @}
//
//protected:
//
//    /// @name Protected Methods
//    /// @{
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    /// @}
//
//
//    /// @name Protected Members
//    /// @{
//
//    std::shared_ptr<ResourceHandle> m_meshHandle{ nullptr };
//
//    /// @brief The name of the mesh (not editable)
//    QLabel* m_meshName{ nullptr };
//
//    /// @}
//
//};


/// @class CreateMeshWidget
/// @brief Widget to create a mesh
class CreateMeshWidget : public ParameterWidget {
    Q_OBJECT
public:

    /// @name Constructors and Destructors
    /// @{
    CreateMeshWidget(WidgetManager* manager, QWidget* parent = nullptr);
    virtual ~CreateMeshWidget();

    /// @}

    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:
    friend class AddMeshCommand;

    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    virtual void createMesh();

    /// @}

    /// @name Protected Members
    /// @{

    Uuid m_meshId{ false };

    /// @brief The mesh
    QComboBox* m_shapeType{ nullptr };

    /// @brief Sphere
    QWidget* m_sphereWidget{ nullptr };
    QLineEdit* m_sphereStackCount{ nullptr };
    QLineEdit* m_sphereSectorCount{ nullptr };

    /// @brief Grid plane
    QWidget* m_gridPlaneWidget{ nullptr };
    QLineEdit* m_gridPlaneSpacing{ nullptr };
    QLineEdit* m_numPlaneHalfSpaces{ nullptr };

    /// @brief Grid cube
    QWidget* m_gridCubeWidget{ nullptr };
    QLineEdit* m_gridCubeSpacing{ nullptr };
    QLineEdit* m_numCubeHalfSpaces{ nullptr };

    /// @brief Cylinder
    QWidget* m_cylinderWidget{ nullptr };
    QLineEdit* m_baseRadius{ nullptr };
    QLineEdit* m_topRadius{ nullptr };
    QLineEdit* m_cylinderHeight{ nullptr };
    QLineEdit* m_cylinderSectorCount{ nullptr };
    QLineEdit* m_cylinderStackCount{ nullptr };

    /// @brief Capsule
    QWidget* m_capsuleWidget{ nullptr };
    QLineEdit* m_capsuleRadius{ nullptr };
    QLineEdit* m_capsuleHalfHeight{ nullptr };

    /// @brief Widget for polygon mesh control
    QStackedWidget* m_shapeOptions{ nullptr };

    QDialogButtonBox* m_confirmButtons{ nullptr };

    GCreatePolygonMeshMessage m_createMeshMessage{};

    /// @}

};



/// @class MeshTreeWidget
//class MeshTreeWidget : public TreeWidget {
//    Q_OBJECT
//public:
//
//    /// @name Constructors and Destructors
//    /// @{
//    MeshTreeWidget(WidgetManager* manager,
//        const std::shared_ptr<ResourceHandle>& modelHandle,
//        QWidget* parent = nullptr);
//    ~MeshTreeWidget();
//
//    /// @}
//
//    /// @name Public Methods
//    /// @{
//
//    /// @brief Add item to the widget
//    void addItem(const std::shared_ptr<ResourceHandle>& meshHandle);
//
//    /// @brief Remove item from the widget
//    void removeItem(const std::shared_ptr<ResourceHandle>& meshHandle);
//
//    /// @}
//
//public slots:
//    /// @brief Populate the widget
//    void repopulate();
//
//protected:
//
//    /// @name Protected Methods
//    /// @{
//
//    /// @brief initialize an item added to the widget
//    virtual void initializeItem(QTreeWidgetItem* item) override;
//
//    /// @brief Initialize the widget
//    virtual void initializeWidget() override;
//
//#ifndef QT_NO_CONTEXTMENU
//    /// @brief Generates a context menu, overriding default implementation
//    /// @note Context menus can be executed either asynchronously using the popup() function or 
//    ///       synchronously using the exec() function
//    void contextMenuEvent(QContextMenuEvent *event) override;
//
//#endif // QT_NO_CONTEXTMENU
//
//    /// @}
//
//    /// @name Protected Members
//    /// @{
//
//    std::shared_ptr<ResourceHandle> m_modelHandle;
//
//    /// @}
//};


///// @class ModelResourceWidget
//class ModelResourceWidget : public ParameterWidget {
//public:
//
//    /// @name Constructors and Destructors
//    /// @{
//    ModelResourceWidget(WidgetManager* manager,
//        const std::shared_ptr<ResourceHandle>& modelHandle,
//        QWidget* parent = nullptr);
//    virtual ~ModelResourceWidget();
//
//    /// @}
//
//    /// @name Public Methods
//    /// @{
//
//    virtual void update() override {
//    }
//
//    /// @}
//
//protected:
//
//    /// @name Protected Methods
//    /// @{
//
//    virtual void initializeWidgets() override;
//    virtual void initializeConnections() override;
//    virtual void layoutWidgets() override;
//
//    /// @}
//
//
//    /// @name Protected Members
//    /// @{
//
//    std::shared_ptr<ResourceHandle> m_modelHandle;
//
//    /// @brief Name of the model
//    QLineEdit* m_nameWidget;
//
//    /// @brief List widget for all meshes associated with the model
//    MeshTreeWidget* m_meshesWidget;
//
//    /// @brief List widget for all textures associated with the model
//    //MaterialTreeWidget* m_materialsWidget;
//
//    GRenameModelMessage m_renameModelMessage; ///< Message to signal renaming of the model
//
//    /// @}
//
//};


} // rev

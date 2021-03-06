#ifndef GB_MODEL_WIDGETS_H 
#define GB_MODEL_WIDGETS_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project
#include "GParameterWidgets.h"
#include "../../core/mixins/GRenderable.h"
#include "../tree/GTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class ResourceHandle;

namespace View {

class ModelResourceWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LoadModelWidget
class LoadModelWidget : public ParameterWidget {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    LoadModelWidget(CoreEngine* core, QWidget* parent = nullptr);
    virtual ~LoadModelWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:
    friend class LoadModelCommand;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    //QComboBox* m_loadMode;
    //QStackedWidget* m_stackedWidget;

    //QWidget* m_loadWidgets;
    FileLoadWidget* m_fileLoadWidget;
    QDialogButtonBox* m_confirmButtons;

    //ModelResourceWidget* m_createModelWidget;

    QString m_fileName;
    Uuid m_modelID = Uuid(false); // null UUID

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class MeshResourceWidget
class MeshResourceWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    MeshResourceWidget(CoreEngine* core,
        const std::shared_ptr<ResourceHandle>& meshHandle,
        QWidget* parent = nullptr);
    virtual ~MeshResourceWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_meshHandle;

    /// @brief The name of the mesh (not editable)
    QLabel* m_meshName;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CreateMeshWidget
/// @brief Widget to create a mesh
class CreateMeshWidget : public ParameterWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    CreateMeshWidget(CoreEngine* core, QWidget* parent = nullptr);
    virtual ~CreateMeshWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:
    friend class AddMeshCommand;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    virtual void createMesh();

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_modelHandle;

    Uuid m_meshHandleID = Uuid(false);

    /// @brief The mesh
    QComboBox* m_shapeType;

    /// @brief Sphere
    QWidget* m_sphereWidget;
    QLineEdit* m_sphereStackCount;
    QLineEdit* m_sphereSectorCount;

    /// @brief Grid plane
    QWidget* m_gridPlaneWidget;
    QLineEdit* m_gridPlaneSpacing;
    QLineEdit* m_numPlaneHalfSpaces;

    /// @brief Grid cube
    QWidget* m_gridCubeWidget;
    QLineEdit* m_gridCubeSpacing;
    QLineEdit* m_numCubeHalfSpaces;

    /// @brief Cylinder
    QWidget* m_cylinderWidget;
    QLineEdit* m_baseRadius;
    QLineEdit* m_topRadius;
    QLineEdit* m_cylinderHeight;
    QLineEdit* m_cylinderSectorCount;
    QLineEdit* m_cylinderStackCount;

    /// @brief Capsule
    QWidget* m_capsuleWidget;
    QLineEdit* m_capsuleRadius;
    QLineEdit* m_capsuleHalfHeight;

    /// @brief Widget for polygon mesh control
    QStackedWidget* m_shapeOptions;

    QDialogButtonBox* m_confirmButtons;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class MeshTreeWidget
class MeshTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    MeshTreeWidget(CoreEngine* engine, 
        const std::shared_ptr<ResourceHandle>& modelHandle,
        QWidget* parent = nullptr);
    ~MeshTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add item to the widget
    void addItem(const std::shared_ptr<ResourceHandle>& meshHandle);

    /// @brief Remove item from the widget
    void removeItem(const std::shared_ptr<ResourceHandle>& meshHandle);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{

    virtual const char* className() const override { return "MeshTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::MeshTreeWidget"; }

    /// @}

public slots:
    /// @brief Populate the widget
    void repopulate();

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{
    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

#ifndef QT_NO_CONTEXTMENU
    /// @brief Generates a context menu, overriding default implementation
    /// @note Context menus can be executed either asynchronously using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;

#endif // QT_NO_CONTEXTMENU

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class MaterialTreeWidget


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ModelResourceWidget
class ModelResourceWidget : public ParameterWidget {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ModelResourceWidget(CoreEngine* core,
        const std::shared_ptr<ResourceHandle>& modelHandle,
        QWidget* parent = nullptr);
    virtual ~ModelResourceWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    virtual void update() override {
    }

    /// @}

protected:

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_modelHandle;

    /// @brief Name of the model
    QLineEdit* m_nameWidget;

    /// @brief List widget for all meshes associated with the model
    MeshTreeWidget* m_meshesWidget;

    /// @brief List widget for all textures associated with the model
    //MaterialTreeWidget* m_materialsWidget;


    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // rev

#endif
#ifndef GB_RESOURCE_WIDGETS_H
#define GB_RESOURCE_WIDGETS_H

// TODO: Allow safe changing of names 
///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GObject.h"
#include "../parameters/GParameterWidgets.h"
#include "../../core/mixins/GLoadable.h"
#include "../../model_control/commands/GUndoCommand.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CoreEngine;
class ResourceHandle;
class Material;
class ShaderProgram;
class Model;

namespace View {
class LoadModelWidget;
class LoadTextureWidget;
class LoadAudioWidget;
class CreateMeshWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LoadTextureCommand
/// @brief Load a texture
class LoadTextureCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    LoadTextureCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~LoadTextureCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The material added by this command
    Uuid m_textureHandleID;

    /// @brief The widget used to load a texture
    View::LoadTextureWidget* m_textureLoadWidget = nullptr;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class DeleteTextureCommand
/// @brief Load a texture
class DeleteTextureCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DeleteTextureCommand(CoreEngine* core,
        const QString &text,
        const std::shared_ptr<ResourceHandle>& texture,
        QUndoCommand *parent = nullptr);
    ~DeleteTextureCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The material added by this command
    Uuid m_textureHandleID;

    QJsonObject m_textureJSON;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddMaterialCommand
/// @brief Add a material
class AddMaterialCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddMaterialCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~AddMaterialCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The material added by this command
    Uuid m_materialID;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CopyMaterialCommand
/// @brief Copy a material
class CopyMaterialCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CopyMaterialCommand(CoreEngine* core,
        const QString &text,
        const Material& model,
        QUndoCommand *parent = nullptr);
    ~CopyMaterialCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    QString getUniqueName();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The JSON for the material copied by this command
    QJsonValue m_materialJson;

    Uuid m_materialUuid;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddMeshCommand
/// @brief Add a mesh
class AddMeshCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddMeshCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~AddMeshCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Mesh widget
    View::CreateMeshWidget* m_meshWidget = nullptr;

    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddModelCommand
/// @brief Add a model
class AddModelCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddModelCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~AddModelCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The model ID added by this command
    Uuid m_modelHandleID;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LoadModelCommand
/// @brief Add a model
class LoadModelCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    LoadModelCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~LoadModelCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Create model widget
    void createModelWidget();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief model widget
    View::LoadModelWidget* m_modelWidget;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class CopyModelCommand
/// @brief Copy a model
class CopyModelCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CopyModelCommand(CoreEngine* core,
        const QString &text,
        const Model& model,
        QUndoCommand *parent = nullptr);
    ~CopyModelCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    QString getUniqueName();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The model added by this command
    Uuid m_modelHandleID;

    /// @brief The JSON for the model copied by this command
    QJsonValue m_modelJson;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class LoadAudioCommand
/// @brief Add audio
class LoadAudioCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    LoadAudioCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~LoadAudioCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Create audio widget
    void createAudioWidget();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief model widget
    View::LoadAudioWidget* m_audioWidget;

    /// @}

};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AddShaderCommand
/// @brief Add a shader program
class AddShaderCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    AddShaderCommand(CoreEngine* core,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~AddShaderCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected methods
    /// @{

    /// @brief Create shader widget
    void createShaderWidget();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief shader widget
    QWidget* m_shaderWidget;
    View::FileLoadWidget* m_vertexWidget;
    View::FileLoadWidget* m_fragmentWidget;
    View::FileLoadWidget* m_geometryWidget;
    View::FileLoadWidget* m_computeWidget;

    QString m_fragFile;
    QString m_vertFile;
    QString m_geomFile;
    QString m_compFile;

    /// @brief The shader program added by this command
    Uuid m_shaderProgramID;

    /// @brief The JSON for the shader added by this command
    QJsonValue m_shaderJson;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class DeleteMaterialCommand
/// @brief Delete a material
class DeleteMaterialCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DeleteMaterialCommand(CoreEngine* core,
        Material* material,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~DeleteMaterialCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The JSON for the material removed by this command
    QJsonValue m_materialJson;

    Uuid m_materialHandleID;
    GString m_materialName;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class DeleteModelCommand
/// @brief Delete a model
class DeleteModelCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DeleteModelCommand(CoreEngine* core,
        Model* model,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~DeleteModelCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The JSON for the model removed by this command
    QJsonValue m_modelJson;
    GString m_modelName;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class DeleteShaderCommand
/// @brief Delete a shader program
class DeleteShaderCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DeleteShaderCommand(CoreEngine* core,
        ShaderProgram* shader,
        const QString &text,
        QUndoCommand *parent = nullptr);
    ~DeleteShaderCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add command
    virtual void redo() override;

    /// @brief Undoes the add command
    virtual void undo() override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief The JSON for the shader removed by this command
    QJsonValue m_shaderJson;
    GString m_shaderName;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace: View
namespace View {
class ResourceTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceJsonWidget
class ResourceJsonWidget : public ParameterWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ResourceHandle>& resource, QWidget *parent = 0);
    ~ResourceJsonWidget();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @}

public slots:

signals:

protected slots:


protected:
    //---------------------------------------------------------------------------------------
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    std::shared_ptr<ResourceHandle> resourceHandle() const;

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QLabel* m_typeLabel;

    /// @brief Serializable object
    Uuid m_resourceHandleID;
    GString m_resourceName;

    QTextEdit* m_textEdit;
    QPushButton* m_confirmButton = nullptr;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceItem
class ResourceItem : public QTreeWidgetItem, public rev::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ResourceItemType {
        kResourceHandle = 2000, // Tree widget item takes a type
        kCategory
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ResourceItem(const QString& text);
    ResourceItem(const std::shared_ptr<ResourceHandle>& handle);
    ~ResourceItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    std::shared_ptr<ResourceHandle> handle() { return m_handle; }

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Return handle represented by this tree item
    inline const std::shared_ptr<ResourceHandle>& handle() const { return m_handle; }

    /// @brief Get the resource item type of this tree item
    ResourceItemType itemType() const { return ResourceItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ResourceTreeWidget* resourceTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "ResourceItem"; }
    virtual const char* namespaceName() const override { return "rev::View:ResourceItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ComponentTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get the resource item type of the given object
    static ResourceItemType getItemType(std::shared_ptr<Object> object);

    /// @brief Initialize the component tree item
    void initializeItem();


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Pointer to the object corresponding to this tree item
    std::shared_ptr<ResourceHandle> m_handle;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceTreeWidget
// TODO: Clean up with standard tree widget
class ResourceTreeWidget : public QTreeWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ResourceTreeWidget(CoreEngine* engine, const QString& name, QWidget* parent = nullptr);
    ~ResourceTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Populate the resource widget
    void repopulate();

    /// @brief Reload the given resource widget
    void reloadItem(const Uuid& handleId);

    /// @brief Add component item to the widget
    void addItem(std::shared_ptr<ResourceHandle> handle);
    void addItem(View::ResourceItem* item);

    /// @brief Remove component item from the widget
    void removeItem(ResourceItem* resourceItem);

    /// @brief Get tree item corresponding to the given component
    View::ResourceItem* getItem(std::shared_ptr<ResourceHandle> handle);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "ResourceTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::ResourceTreeWidget"; }

    /// @brief Returns True if this AbstractService represents a service
    /// @details This is not a service
    virtual bool isService() const override { return false; };

    /// @brief Returns True if this AbstractService represents a tool.
    /// @details This is not a tool
    virtual bool isTool() const override { return false; };

    /// @}

protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /// @brief What to do on item expanded
    void onItemExpanded(QTreeWidgetItem* item);

    /// @brief What to do on current item change
    //void onCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous);

    /// @}
protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class ResourceItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{


    void initializeCategories();

    /// @brief Override default mouse release event
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    /// @brief Remove an item
    void removeItem(const Uuid& handleId);

    /// @brief Initialize the widget
    void initializeWidget();

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

    ResourceItem* m_imageItem;
    ResourceItem* m_textureItem;
    ResourceItem* m_materialItem;
    ResourceItem* m_meshItem;
    ResourceItem* m_cubeTextureItem;
    ResourceItem* m_animationItem;
    ResourceItem* m_skeletonItem;
    ResourceItem* m_modelItem;
    ResourceItem* m_shaderItem;
    ResourceItem* m_scriptItem;
    ResourceItem* m_audioItem;

    /// @brief Actions performable in this widget
    QAction* m_addModel;
    QAction* m_loadModel;
    QAction* m_copyModel;
    QAction* m_addShaderProgram;
    QAction* m_addTexture;
    QAction* m_addMaterial;
    QAction* m_copyMaterial;
    QAction* m_addMesh;
    QAction* m_deleteModel;
    QAction* m_deleteMaterial;
    QAction* m_deleteShaderProgram;

    /// @brief The resource clicked by a right-mouse operation
    ResourceItem* m_currentResourceItem;

    /// @brief Core engine for the application
    CoreEngine* m_engine;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
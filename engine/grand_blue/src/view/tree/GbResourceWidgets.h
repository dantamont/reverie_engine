#ifndef GB_RESOURCE_WIDGETS_H
#define GB_RESOURCE_WIDGETS_H

// TODO: Allow safe changing of names 
///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../core/GbObject.h"
#include "../parameters/GbParameterWidgets.h"
#include "../../core/mixins/GbLoadable.h"
#include "../../model_control/commands/GbUndoCommand.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {

class CoreEngine;
class ResourceHandle;
class Material;
class ShaderProgram;
class Model;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
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
    std::shared_ptr<Material> m_material;

    /// @brief The JSON for the material added by this command
    QJsonValue m_materialJson;

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

    /// @brief Create material widget
    void createModelWidget();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Model file
    QString m_modelFileOrName;

    /// @brief shader widget
    QWidget* m_modelWidget;

    /// @brief The model added by this command
    std::shared_ptr<Model> m_model;

    /// @brief The JSON for the model added by this command
    QJsonValue m_modelJson;

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

    /// @brief The JSON for the model copied by this command
    QJsonValue m_modelJson;

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

    QString m_fragFile;
    QString m_vertFile;

    /// @brief The shader program added by this command
    std::shared_ptr<ShaderProgram> m_shaderProgram;

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
        std::shared_ptr<Material> material,
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

    /// @brief The material deleted by this command
    std::shared_ptr<Material> m_material;

    /// @brief The JSON for the material removed by this command
    QJsonValue m_materialJson;

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
        std::shared_ptr<Model> model,
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

    /// @brief The model deleted by this command
    std::shared_ptr<Model> m_model;

    /// @brief The JSON for the model removed by this command
    QJsonValue m_modelJson;

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
        std::shared_ptr<ShaderProgram> shader,
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

    /// @brief The shader program deleted by this command
    std::shared_ptr<ShaderProgram> m_shaderProgram;

    /// @brief The JSON for the shader removed by this command
    QJsonValue m_shaderJson;

    /// @}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace: View
namespace View {
class ResourceTreeWidget;

/// @struct ResourceLike
/// @brief Wrapper to convert resource-like types to a serializable object
struct ResourceLike {
    ResourceLike();
    ResourceLike(std::shared_ptr<Object> object);
    ~ResourceLike();

    /// @brief Return stored object
    std::shared_ptr<Object> object() const;

    /// @brief Convert stored object to serializable
    std::shared_ptr<Serializable> serializable() const;

    std::shared_ptr<ResourceHandle> m_handle;
    std::shared_ptr<Model> m_model;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<ShaderProgram> m_shaderProgram;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Add a method for managing type values getting updating outside of the widget
/// @class ResourceJsonWidget
class ResourceJsonWidget : public ParameterWidget{
    Q_OBJECT
public:
    //---------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{

    ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ResourceHandle>& resource, QWidget *parent = 0);
    ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<Model>& resource, QWidget *parent = 0);
    ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<Material>& resource, QWidget *parent = 0);
    ResourceJsonWidget(CoreEngine* core, const std::shared_ptr<ShaderProgram>& resource, QWidget *parent = 0);
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

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}
    //---------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    QLabel* m_typeLabel;

    /// @brief Serializable object
    ResourceLike m_object;

    QTextEdit* m_textEdit;
    QPushButton* m_confirmButton;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceItem
class ResourceItem : public QTreeWidgetItem, public Gb::Object {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum ResourceItemType {
        kResourceHandle = 2000, // Tree widget item takes a type
        kMaterial,
        kModel,
        kShaderProgram,
        kCategory
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    ResourceItem(const QString& text);
    ResourceItem(std::shared_ptr<Object> resourceLike);
    ~ResourceItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Return object represented by this tree item
    inline const ResourceLike& object() { return m_object; }

    /// @brief Get the resource item type of this tree item
    ResourceItemType itemType() const { return ResourceItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::ResourceTreeWidget* resourceTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "ResourceItem"; }
    virtual const char* namespaceName() const override { return "Gb::View:ResourceItem"; }

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
    ResourceLike m_object;

    /// @brief Pointer to widget if this item has one
    QWidget* m_widget;

    /// @}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ResourceTreeWidget
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

    /// @brief Reload the given resource-like widget
    void reloadItem(std::shared_ptr<Object> object);

    /// @brief Add component item to the widget
    void addItem(std::shared_ptr<Object> object);
    void addItem(View::ResourceItem* item);

    /// @brief Remove component item from the widget
    void removeItem(ResourceItem* resourceItem);

    /// @brief Get tree item corresponding to the given component
    View::ResourceItem* getItem(std::shared_ptr<Object> itemObject);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Gb::Object overrides
    /// @{
    virtual const char* className() const override { return "ResourceTreeWidget"; }
    virtual const char* namespaceName() const override { return "Gb::View::ResourceTreeWidget"; }

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
    void removeItem(std::shared_ptr<Object> itemObject);

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

    ResourceItem* m_resourceItem;
    ResourceItem* m_shaderItem;
    ResourceItem* m_modelItem;
    ResourceItem* m_materialItem;

    /// @brief Actions performable in this widget
    QAction* m_addModel;
    QAction* m_copyModel;
    QAction* m_addShaderProgram;
    QAction* m_addMaterial;
    QAction* m_copyMaterial;
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
#pragma once
/// @todo Allow safe changing of names 

// Qt
#include <QtWidgets>

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "geppetto/qt/actions/commands/GUndoCommand.h"

namespace rev {

class GGetResourceDataMessage;
class GResourceDataMessage;
class GResourceAddedMessage;
class GResourceModifiedMessage;
class GScenarioJsonMessage;

class WidgetManager;
class LoadModelWidget;
class LoadTextureWidget;
class LoadAudioWidget;
class CreateMeshWidget;
class FileLoadWidget;

/// @class LoadTextureCommand
/// @brief Load a texture
class LoadTextureCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    LoadTextureCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~LoadTextureCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eLoadTexture; }

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}

protected:

    /// @name Protected members
    /// @{

    Uuid m_textureHandleID; ///< The material added by this command
    LoadTextureWidget* m_textureLoadWidget{ nullptr }; ///< The widget used to load a texture

    /// @}

};


/// @class AddMaterialCommand
/// @brief Add a material
class AddMaterialCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddMaterialCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~AddMaterialCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eAddMaterial; }

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}

protected:
    /// @name Protected members
    /// @{

    Uuid m_materialID; ///< The material added by this command

    /// @}

};


/// @class CopyMaterialCommand
/// @brief Copy a material
class CopyMaterialCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    CopyMaterialCommand(WidgetManager* wm, const QString &text, const json& materialJson, QUndoCommand *parent = nullptr);
    ~CopyMaterialCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eCopyMaterial; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    /// @name Protected methods
    /// @{

    GString getUniqueName();

    /// @}

    /// @name Protected members
    /// @{

    json m_materialJson; ///< The JSON for the material copied by this command
    Uuid m_materialUuid;

    /// @}

};


/// @class AddMeshCommand
/// @brief Add a mesh
class AddMeshCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddMeshCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~AddMeshCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eAddMesh; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    /// @name Protected members
    /// @{

    CreateMeshWidget* m_meshWidget{ nullptr }; ///< Mesh widget

    /// @}

};


/// @class AddModelCommand
/// @brief Add a model
class AddModelCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddModelCommand(WidgetManager* wm, const QString &text,  QUndoCommand *parent = nullptr);
    ~AddModelCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eAddModel; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    /// @name Protected members
    /// @{

    Uuid m_modelHandleID; ///< The model ID added by this command

    /// @}

};


/// @class LoadModelCommand
/// @brief Add a model
class LoadModelCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    LoadModelCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~LoadModelCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eLoadModel; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    /// @name Protected methods
    /// @{

    /// @brief Create model widget
    void createModelWidget();

    /// @}

    /// @name Protected members
    /// @{

    /// @brief model widget
    LoadModelWidget* m_modelWidget{ nullptr };

    /// @}

};


/// @class CopyModelCommand
/// @brief Copy a model
class CopyModelCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    CopyModelCommand(WidgetManager* wm, const QString &text, const json& modelJson, QUndoCommand *parent = nullptr);
    ~CopyModelCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eCopyModel; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    /// @name Protected methods
    /// @{

    GString getUniqueName();

    /// @}

    /// @name Protected members
    /// @{

    Uuid m_modelHandleID; ///< The model added by this command
    json m_modelJson; ///< The JSON for the model copied by this command

    /// @}

};


/// @class LoadAudioCommand
/// @brief Add audio
class LoadAudioCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    LoadAudioCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~LoadAudioCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eLoadAudio; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    /// @name Protected methods
    /// @{

    /// @brief Create audio widget
    void createAudioWidget();

    /// @}

    /// @name Protected members
    /// @{

    LoadAudioWidget* m_audioWidget{ nullptr };

    /// @}

};


/// @class AddShaderCommand
/// @brief Add a shader program
class AddShaderCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    AddShaderCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    ~AddShaderCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eAddShader; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    /// @name Protected methods
    /// @{

    /// @brief Create shader widget
    void createShaderWidget();

    /// @}

    /// @name Protected members
    /// @{

    /// @brief shader widget
    QWidget* m_shaderWidget{ nullptr };
    FileLoadWidget* m_vertexWidget{ nullptr };
    FileLoadWidget* m_fragmentWidget{ nullptr };
    FileLoadWidget* m_geometryWidget{ nullptr };
    FileLoadWidget* m_computeWidget{ nullptr };

    QString m_fragFile;
    QString m_vertFile;
    QString m_geomFile;
    QString m_compFile;

    Uuid m_shaderProgramID; ///< The shader program added by this command
    json m_shaderJson; ///< The JSON for the shader added by this command

    /// @}

};


/// @class DeleteMaterialCommand
/// @brief Delete a material
class DeleteMaterialCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    DeleteMaterialCommand(WidgetManager* wm, const json& materialResourceJson, const QString &text, QUndoCommand *parent = nullptr);
    ~DeleteMaterialCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eDeleteMaterial; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:
    /// @name Protected members
    /// @{

    json m_materialResourceJson; ///< The JSON for the material handle removed by this command
    Uuid m_materialHandleID;
    GString m_materialName;

    /// @}

};


/// @class DeleteModelCommand
/// @brief Delete a model
class DeleteModelCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    DeleteModelCommand(WidgetManager* wm, const json& modelResourceJson, const QString &text, QUndoCommand *parent = nullptr);
    ~DeleteModelCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eDeleteModel; }

    /// @brief Redoes the command
    virtual void redo() override;

    /// @brief Undoes the command
    virtual void undo() override;

    /// @}
protected:

    /// @name Protected members
    /// @{

    json m_modelResourceJson; ///< The JSON for the model handle removed by this command
    GString m_modelName;

    /// @}

};


/// @class DeleteShaderCommand
/// @brief Delete a shader program
class DeleteShaderCommand : public UndoCommand {
public:
    /// @name Constructors/Destructor
    /// @{
    DeleteShaderCommand(WidgetManager* wm, const json& shaderResourceJson, const QString &text, QUndoCommand *parent = nullptr);
    ~DeleteShaderCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const override { return ECommandType::eDeleteShader; }

    /// @brief Redoes the add command
    virtual void redo() override;

    /// @brief Undoes the add command
    virtual void undo() override;

    /// @}
protected:
    /// @name Protected members
    /// @{

    json m_shaderResourceJson; ///< The JSON for the shader resource handle removed by this command
    GString m_shaderName;

    /// @}

};


class ResourceTreeWidget;


/// @class ResourceJsonWidget
class ResourceJsonWidget : public ParameterWidget{
    Q_OBJECT
public:
    /// @name Constructors/Destructor
    /// @{

    ResourceJsonWidget(WidgetManager* wm, const json& resourceHandleJson, QWidget *parent = 0);
    ~ResourceJsonWidget();

    /// @}

protected:
    /// @name Protected Events
    /// @{

    /// @brief Scroll event
    void wheelEvent(QWheelEvent* event) override;

    /// @}
    
    /// @name Protected Methods
    /// @{

    virtual void initializeWidgets() override;
    virtual void initializeConnections() override;
    virtual void layoutWidgets() override;

    /// @}

    /// @name Protected Members
    /// @{

    QLabel* m_typeLabel{ nullptr };

    json m_resourceHandleJson;
    Uuid m_resourceHandleID{ false };
    GString m_resourceName;

    QTextEdit* m_textEdit{ nullptr };
    QPushButton* m_confirmButton{ nullptr };

    /// @}
};



/// @class ResourceItem
class ResourceItem : public QTreeWidgetItem {
public:
    /// @name Static
    /// @{

    enum ResourceItemType {
        kResourceHandle = 2000, ///< Tree widget item takes a type
        kCategory ///< Bins for resource handle items
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    ResourceItem(const char* name);
    ResourceItem(const json& handleJson);
    ~ResourceItem();

    /// @}

    /// @name Public Methods
    /// @{

    const Uuid& handleId() const { return m_handleId; }
    const json& resourceHandleJson() const { return m_resourceHandleJson; }

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    void setWidget();
    void removeWidget();

    /// @brief Get the resource item type of this tree item
    ResourceItemType itemType() const { return ResourceItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    ResourceTreeWidget* resourceTreeWidget() const;

    /// @}


protected:
    /// @name Friends
    /// @{

    friend class ComponentTreeWidget;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Initialize the component tree item
    void initializeItem();

    /// @}

    /// @name Protected Members
    /// @{

    Uuid m_handleId;
    json m_resourceHandleJson; ///< The JSON of the resource handle corresponding to this item
    QWidget* m_widget{ nullptr }; ///< Pointer to widget if this item has one

    /// @}
};


/// @class ResourceTreeWidget
// TODO: Clean up with standard tree widget
class ResourceTreeWidget : public QTreeWidget, public rev::NameableInterface {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    ResourceTreeWidget(WidgetManager* wm, const QString& name, QWidget* parent = nullptr);
    ~ResourceTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Populate the resource widget
    void repopulate(const GScenarioJsonMessage& message);

    /// @brief Request to reload the given resource item
    void requestReloadItem(const Uuid& handleId);

    /// @brief Reload the given resource widget
    void reloadItem(const GResourceDataMessage& handleId);
    void reloadItem(const GResourceAddedMessage& handleId);
    void reloadItem(const GResourceModifiedMessage& handleId);

    /// @brief Add component item to the widget
    void addItem(const json& handleJson);
    void addItem(ResourceItem* item);

    /// @brief Remove component item from the widget
    void removeItem(ResourceItem* resourceItem);

    /// @brief Get tree item corresponding to the given component
    ResourceItem* getItem(const Uuid& handleId);

    /// @brief Resize columns to fit content
    void resizeColumns();

    /// @}


protected slots:
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
    /// @name Friends
    /// @{

    friend class ResourceItem;

    /// @}

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
    /// @note Context menus can be executed either asynchronous ly using the popup() function or 
    ///       synchronously using the exec() function
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

    void reloadItem(const Uuid& handleId, const json& handleJson);

    /// @}

    /// @name Protected Members
    /// @{

    ResourceItem* m_textureItem{ nullptr };
    ResourceItem* m_materialItem{ nullptr };
    ResourceItem* m_meshItem{ nullptr };
    ResourceItem* m_cubeTextureItem{ nullptr };
    ResourceItem* m_animationItem{ nullptr };
    ResourceItem* m_skeletonItem{ nullptr };
    ResourceItem* m_modelItem{ nullptr };
    ResourceItem* m_shaderItem{ nullptr };
    ResourceItem* m_scriptItem{ nullptr };
    ResourceItem* m_audioItem{ nullptr };

    /// @brief Actions performable in this widget
    QAction* m_addModel{ nullptr };
    QAction* m_loadModel{ nullptr };
    QAction* m_copyModel{ nullptr };
    QAction* m_addShaderProgram{ nullptr };
    QAction* m_addTexture{ nullptr };
    QAction* m_addMaterial{ nullptr };
    QAction* m_copyMaterial{ nullptr };
    QAction* m_addMesh{ nullptr };
    QAction* m_deleteModel{ nullptr };
    QAction* m_deleteMaterial{ nullptr };
    QAction* m_deleteShaderProgram{ nullptr };

    /// @brief The resource clicked by a right-mouse operation
    ResourceItem* m_currentResourceItem{ nullptr };

    /// @brief Core engine for the application
    WidgetManager* m_widgetManager{ nullptr };

    /// @}
};


// End namespaces        
}

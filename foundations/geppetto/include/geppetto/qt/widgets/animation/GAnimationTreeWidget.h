#pragma once

// Qt
#include <QtWidgets>

// Internal
#include "geppetto/qt/widgets/types/GParameterWidgets.h"
#include "fortress/types/GLoadable.h"
#include "geppetto/qt/widgets/tree/GTreeWidget.h"

#include "ripple/network/messages/GModifyAnimationComponentMotionMessage.h"
#include "ripple/network/messages/GAddAnimationClipToStateMessage.h"
#include "ripple/network/messages/GRemoveAnimationClipFromStateMessage.h"
#include "ripple/network/messages/GRemoveAnimationComponentMotionMessage.h"
#include "ripple/network/messages/GAddAnimationStateMachineStateMessage.h"
#include "ripple/network/messages/GRemoveAnimationStateMachineStateMessage.h"

namespace rev {

class AnimationNodeWidget;
class AnimationTreeWidgetInterface;

/// @class AnimationItem
class AnimationItem : public TreeItem {
public:
    /// @name Static
    /// @{

    enum class AnimationItemType {
        kClip = 2000,
        kMotion,
        kState
    };

    /// @}

    /// @name Constructors and Destructors
    /// @{
    AnimationItem(Int32_t itemId, const json& animationItemJson, AnimationItemType type);
    ~AnimationItem();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Get the resource item type of this tree item
    AnimationItemType itemType() const { return AnimationItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    AnimationTreeWidgetInterface* animationTreeWidget() const;


    /// @}

protected:
    /// @name Friends
    /// @{

    friend class AnimationTreeWidgetInterface;

    /// @}

    /// @name Protected Methods
    /// @{

    virtual void initializeItem() override;
    void updateText();

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @}

    /// @name Protected Members
    /// @{

    GModifyAnimationComponentMotionMessage m_modifyMotionMessage;

    /// @}

};


enum class AnimationTreeMode {
    kClips, // List animation clips
    kMotion, // List animation motions
    kStates // List animation states
};

/// @class AnimationTreeWidget
class AnimationTreeWidgetInterface : public TreeWidget {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimationTreeWidgetInterface(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, const char* widgetName, QWidget* parent = nullptr);
    ~AnimationTreeWidgetInterface();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Get currently selected item object
    AnimationItem* selectedAnimationItem(uint32_t index = 0) const {
        if (!selectedItems().size()) {
            return nullptr;
        }
        return static_cast<AnimationItem*>(selectedItems()[index]);
    }

    /// @brief Add animation item to the widget
    virtual void addItem(Uint32_t itemIndex, json& itemJson) = 0;

    /// @brief Remove component item from the widget
    void removeItem(AnimationItem* AnimationItem);

    /// @brief The mode describing the type of object that the tree is displaying
    virtual AnimationTreeMode treeMode() const = 0; 

    /// @}

signals:

    void selectedItem (const Uuid& uuid, AnimationTreeMode mode);

public slots:

    /// @brief Populate the widget
    virtual void repopulate() = 0;

protected slots:
   
    /// @name Protected Slots
    /// @{

    /// @brief For object scelection
    virtual void onItemClicked(QTreeWidgetItem *item, int column) override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}
protected:
    /// @name Friends
    /// @{

    friend class AnimationItem;

    /// @}

    /// @name Protected Methods
    /// @{

    Uuid getStateMachineId() const;
    Uint32_t getSceneObjectId() const { return m_sceneObjectId; }

    AnimationItem* currentContextItem() const {
        return static_cast<AnimationItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    ///// @brief Request repopulate
    //void requestRepopulate();

    /// @}

    /// @name Protected Members
    /// @{

    AnimationNodeWidget* m_nodeWidget; ///< The animation node widget associated with this widget, may not necessarily be the parent
    Uint32_t m_sceneObjectId; ///< The ID of the scene object associated with the motions being modified
    json& m_animationComponentJson; ///< The JSON describing the animation component

    /// @}
};



/// @class AnimationClipTreeWidget
class AnimationClipTreeWidget : public AnimationTreeWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimationClipTreeWidget(WidgetManager* wm, json& animationComponentJson, json& stateJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent = nullptr);
    ~AnimationClipTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add animation item to the widget
    virtual void addItem(Uint32_t itemIndex, json& itemJson);

    /// @brief The mode describing the type of object that the tree is displaying
    virtual AnimationTreeMode treeMode() const { return AnimationTreeMode::kClips; }

    /// @}

public slots:

    /// @brief Populate the widget
    virtual void repopulate();

protected slots:

    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem* item, int column) override;

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class AnimationItem;

    /// @}

    /// @name Protected Methods
    /// @{

    Uuid getStateId() const;

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    /// @name Protected Members
    /// @{

    json& m_stateJson; ///< The state JSON, if displaying animation clips
    GAddAnimationClipToStateMessage m_addClipMessage;
    GRemoveAnimationClipFromStateMessage m_removeClipMessage;

    /// @}
};


/// @class AnimationMotionTreeWidget
class AnimationMotionTreeWidget : public AnimationTreeWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimationMotionTreeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent = nullptr);
    ~AnimationMotionTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add animation item to the widget
    virtual void addItem(Uint32_t itemIndex, json& itemJson);

    /// @brief The mode describing the type of object that the tree is displaying
    virtual AnimationTreeMode treeMode() const { return AnimationTreeMode::kMotion; }

    /// @}
    
public slots:

    /// @brief Populate the widget
    virtual void repopulate();

protected slots:

    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem* item, int column) override;

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class AnimationItem;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    /// @name Protected Members
    /// @{

    GRemoveAnimationComponentMotionMessage m_removeMotionMessage;

    /// @}
};

/// @class AnimationStateTreeWidget
class AnimationStateTreeWidget : public AnimationTreeWidgetInterface {
    Q_OBJECT
public:
    /// @name Constructors and Destructors
    /// @{
    AnimationStateTreeWidget(WidgetManager* wm, json& animationComponentJson, Uint32_t sceneObjectId, AnimationNodeWidget* nodeWidget, QWidget* parent = nullptr);
    ~AnimationStateTreeWidget();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Add animation item to the widget
    virtual void addItem(Uint32_t itemIndex, json& itemJson);

    /// @brief The mode describing the type of object that the tree is displaying
    virtual AnimationTreeMode treeMode() const { return AnimationTreeMode::kStates; }

    /// @}

public slots:

    /// @brief Populate the widget
    virtual void repopulate();

protected slots:

    /// @name Protected Slots
    /// @{

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem* item, int column) override;

    /// @}

protected:
    /// @name Friends
    /// @{

    friend class AnimationItem;

    /// @}

    /// @name Protected Methods
    /// @{

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    /// @name Protected Members
    /// @{

    GAddAnimationStateMachineStateMessage m_addStateMessage;
    GRemoveAnimationStateMachineStateMessage m_removeStateMessage;

    /// @}
};


// End namespaces
}

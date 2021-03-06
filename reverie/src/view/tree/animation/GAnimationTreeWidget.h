#ifndef GB_ANIMATION_TREE_WIDGET_H
#define GB_ANIMATION_TREE_WIDGET_H


///////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>

// Internal
#include "../../../core/GObject.h"
#include "../../parameters/GParameterWidgets.h"
#include "../../../core/mixins/GLoadable.h"
#include "../GTreeWidget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

class CoreEngine;
class Motion;
class BaseAnimationState;
class AnimationState;
class AnimationClip;
class AnimationController;
class Motion;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace View {

class AnimationTreeWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class AnimationItem
class AnimationItem : public TreeItem<Object> {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum class AnimationItemType {
        kClip = 2000,
        kMotion,
        kState
    };

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AnimationItem(Object* itemSource, AnimationItemType type);
    ~AnimationItem();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Set the widget for this item in the given tree widget
    /// @note This is only called on the double click event
    virtual void setWidget() override;
    virtual void removeWidget(int column = 0) override;

    /// @brief Return object represented by this tree item
    template<typename T>
    inline T* itemObject() { 
        return static_cast<T*>(m_object); 
    }

    /// @brief Get the resource item type of this tree item
    AnimationItemType itemType() const { return AnimationItemType(type()); }

    /// @brief Convenience method for retrieving casted inspector widget
    View::AnimationTreeWidget* animationTreeWidget() const;


    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{
    virtual const char* className() const override { return "AnimationItem"; }
    virtual const char* namespaceName() const override { return "rev::View:AnimationItem"; }

    /// @}

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class AnimationTreeWidget;

    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void initializeItem() override;
    void updateText();

    /// @brief Perform an action from this item
    void performAction(UndoCommand* command);

    /// @}

};



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class AnimationTreeMode {
    kClips, // List animation clips
    kMotion, // List animation motions
    kStates // List animation states
};

/// @class AnimationTreeWidget
class AnimationTreeWidget : public TreeWidget {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    AnimationTreeWidget(CoreEngine* engine, AnimationController* controller, AnimationTreeMode mode, QWidget* parent = nullptr);
    AnimationTreeWidget(CoreEngine* engine, AnimationController* controller, AnimationState* state, QWidget* parent = nullptr);
    ~AnimationTreeWidget();

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Get currently selected item object
    template<typename T>
    T* selectedItemObject(size_t index = 0) {
        if (!selectedItems().size()) {
            return nullptr;
        }
        return static_cast<AnimationItem*>(selectedItems()[index])->itemObject<T>();
    }

    /// @brief Add animation item to the widget
    void addItem(AnimationClip* clip);
    void addItem(Motion* clip);
    void addItem(BaseAnimationState* state);

    /// @brief Remove component item from the widget
    void removeItem(AnimationItem* AnimationItem);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name rev::Object overrides
    /// @{

    virtual const char* className() const override { return "AnimationTreeWidget"; }
    virtual const char* namespaceName() const override { return "rev::View::AnimationTreeWidget"; }

    virtual bool isService() const override { return false; };
    virtual bool isTool() const override { return true; };

    /// @}

signals:

    void selectedItem (const Uuid& uuid, AnimationTreeMode mode);

public slots:
    /// @brief Populate the widget
    void repopulate();


protected slots:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Slots
    /// @{

    /// @brief For object scelection
    virtual void onItemClicked(QTreeWidgetItem *item, int column) override;

    /// @brief What to do on item double click
    virtual void onItemDoubleClicked(QTreeWidgetItem *item, int column) override;

    /// @}
protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Friends
    /// @{

    friend class AnimationItem;

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    AnimationItem* currentContextItem() const {
        return static_cast<AnimationItem*>(m_currentItems[kContextClick]);
    }

    /// @brief initialize an item added to the widget
    virtual void initializeItem(QTreeWidgetItem* item) override;

    //virtual void onDropAbove(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;
    //virtual void onDropBelow(QDropEvent* event, QTreeWidgetItem* source, QTreeWidgetItem* destination) override;

    /// @brief Remove an item
    void removeItem(Object* itemObject);

    /// @brief Initialize the widget
    virtual void initializeWidget() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{
    AnimationTreeMode m_treeMode;

    AnimationController* m_controller;

    /// @brief The state, if displaying clips
    AnimationState* m_state = nullptr;

    /// @}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
}
}

#endif // COMPONENT_WIDGETS_H
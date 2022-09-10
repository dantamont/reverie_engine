#include "geppetto/qt/widgets/tree/GComponentTreeWidget.h"

#include "heave/kinematics/GTransform.h"

#include "enums/GWidgetActionTypeEnum.h"
#include "enums/GWidgetTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/actions/commands/GComponentCommand.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/list/GBlueprintView.h"
#include "geppetto/qt/widgets/tree/GComponentTreeItems.h"
#include "geppetto/qt/widgets/tree/GSceneTreeWidget.h"

namespace rev {


ComponentTreeWidget::ComponentTreeWidget(WidgetManager* wm, const QString & name, QWidget * parent) :
    rev::NameableInterface(name.toStdString()),
    QTreeWidget(parent),
    m_widgetManager(wm)
{
    static const json myJson = { {"widgetType", EWidgetType::eComponentTree} };
    m_getScenarioJsonMessage.setJsonBytes(GJson::ToBytes(myJson));
    initializeWidget();
}

ComponentTreeWidget::~ComponentTreeWidget()
{
}

void ComponentTreeWidget::selectScene(const GString& name) {
    // Send message to retrieve scenario data and select scene
    json myJson { 
        {"widgetType", EWidgetType::eComponentTree},
        {"widgetAction", EWidgetActionType::eSelectScene},
        {"sceneName", name}
    };

    m_getScenarioJsonMessage.setJsonBytes(GJson::ToBytes(myJson));
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_getScenarioJsonMessage);
}

void ComponentTreeWidget::onSelectScene(const GString& name)
{
    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    clearSceneObject();

    // Set new scene
    m_currentSceneObjectId = -1;
    m_currentSceneName = name;

    // Get JSON representing the new scene
    const json* sceneJson = nullptr;
    const std::string& mainSceneName = m_widgetManager->scenarioJson()["scene"]["name"].get_ref<const std::string&>();
    if (m_currentSceneName == mainSceneName) {
        // Main scene
        sceneJson = &m_widgetManager->scenarioJson()["scene"];
    }
    else {
        // Debug scene
        sceneJson = &m_widgetManager->scenarioJson()["debug"]["debugScene"];
    }

    // Populate widget with components of new scene
    /// @todo Enable rendering of script-generated components
    for (const json& components : (*sceneJson)["components"]) {
        for (const json& comp : components) {
            addItem(GSceneType(ESceneType::eScene), comp);
        }
    }

    // Reenable updates
    setUpdatesEnabled(true);
}

void ComponentTreeWidget::clearScene()
{    
    // Clear the widget
    cachedClear();
}

void ComponentTreeWidget::selectSceneObject(uint32_t sceneObjectID)
{
    // Send message to retrieve scenario data and select scene object
    json myJson{
        {"widgetType", EWidgetType::eComponentTree},
        {"widgetAction", EWidgetActionType::eSelectSceneObject},
        {"sceneObjectId", sceneObjectID}
    };

    m_getScenarioJsonMessage.setJsonBytes(GJson::ToBytes(myJson));
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_getScenarioJsonMessage);
}

void ComponentTreeWidget::onSelectSceneObject(Uint32_t sceneObjectId)
{
    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    // Clear the widget
    clearScene();
    m_currentSceneObjectId = sceneObjectId;

    // Return if no new scene object
    if (m_currentSceneObjectId < 0) {
#ifdef DEBUG_MODE
        assert(false && "selectSceneObject::Warning, no scene object found");
#endif
        return;
    }

    // Add render layers widget
    const json& currentSceneObjectJson = m_widgetManager->sceneObjectJson(sceneObjectId);
    auto item = new ComponentItem(
        ComponentItem::ComponentItemType::kSceneObjectSettings,
        currentSceneObjectJson
        );
    addItem(item);

    // Add transform component
    addItem(GSceneType(ESceneType::eSceneObject), currentSceneObjectJson["transform"]);

    // Populate with components of new scene object
    /// @todo Include script-generated components
    const json& componentJson = currentSceneObjectJson["components"];
    for (const json& component : componentJson) {
        addItem(GSceneType(ESceneType::eSceneObject), component);
    }

    // Reenable updates
    setUpdatesEnabled(true);
}

void ComponentTreeWidget::onSelectedBlueprint(QModelIndex blueprintIndex)
{
    //clearBlueprint();

    // Cache that the blueprint was selected
    size_t idx = blueprintIndex.row();
    m_selectedBlueprintJson = m_widgetManager->scenarioJson()["blueprints"][idx];
    m_currentSceneObjectId = -1;
    m_currentSceneName = GString();

    // Temporarily disable updates for a performance boost
    setUpdatesEnabled(false);

    // Clear the widget
    clearScene();

    auto item = new ComponentBlueprintItem(m_selectedBlueprintJson);
    addItem(item);

    /// @todo Re-do all of this, but with JSON, which makes this approach possible now :)
    // Create a new scene object just for the widgets
    //m_blueprintSceneObject = SceneObject::Create(&m_widgetManager->scenario()->scene(), SceneObjectFlag::kIsInvisible);
    //m_blueprintSceneObject->loadFromJson(blueprint.sceneObjectJson());
    //m_blueprintSceneObject->setEnabled(false);
    //m_blueprintSceneObject->setVisible(false);

    //// Add render layers widget
    //auto item = new ComponentBlueprintItem(m_blueprintSceneObject.get());
    //addItem(item);


    //// Add transform component
    //addItem((Component*)(&m_blueprintSceneObject->transform()));

    //// Populate with components of new scene object
    //for (const auto& componentList : m_blueprintSceneObject->components()) {
    //    for (const auto& comp : componentList) {
    //        addItem(comp);
    //    }
    //}
    //
    //// Disable all of the newly added widgets
    //for (int i = 0; i < topLevelItemCount(); ++i)
    //{
    //    QTreeWidgetItem *item = topLevelItem(i);
    //    ComponentItem* compItem = dynamic_cast<ComponentItem*>(item);
    //    compItem->m_widget->setDisabled(true);
    //    item->setDisabled(true);
    //}

    // Reenable updates
    setUpdatesEnabled(true);
}

void ComponentTreeWidget::clearSceneObject()
{
    // Clear the widget
    cachedClear();
}

void ComponentTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)
}

void ComponentTreeWidget::addItem(GSceneType sceneType, const json& componentJson)
{
    // Create component item
    ComponentItem::ComponentItemType componentType = ComponentItem::kInvalid;

    switch ((ESceneType)sceneType) {
    case ESceneType::eSceneObject:
        componentType = static_cast<ComponentItem::ComponentItemType>(Int32_t(componentJson["type"]) + 2000);
        break;
    case ESceneType::eScene:
        // There is only one scene component type!
        componentType = ComponentItem::kPhysicsScene;
        break;
    default:
        assert(false && "Invalid scene type");
        break;
    }
    auto* componentItem = new ComponentItem(componentType, componentJson);

    // Add component item
    addItem(componentItem);
}

void ComponentTreeWidget::addItem(ComponentItem * item)
{
    // Insert component item into the widget
    addTopLevelItem(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    //resizeColumns();
}

void ComponentTreeWidget::addItem(ComponentBlueprintItem * item)
{
    // Insert component item into the widget
    addTopLevelItem(item);

    // Create widget for component
    item->setWidget();

    // Resize columns to fit widget
    //resizeColumns();
}

void ComponentTreeWidget::removeItem(const Uuid& componentId)
{
    removeItem(getItem(componentId));
}

ComponentItem * ComponentTreeWidget::getItem(const Uuid& componentId)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        ComponentItem* item = static_cast<ComponentItem*>(*it);
        if (item->data().get<Uuid>("id") == componentId) {
            return item;
        }
        ++it;
    }

    assert(false && "Error, no item found that corresponds to the given object");

    return nullptr;
}

void ComponentTreeWidget::resizeColumns()
{
    resizeColumnToContents(0);
}

void ComponentTreeWidget::onItemExpanded(QTreeWidgetItem * item)
{
    Q_UNUSED(item);
}

//void ComponentTreeWidget::clearBlueprint()
//{
    // Removed, very buggy managing components
    //if (m_blueprintSceneObject) {
    //    //m_widgetManager->simulationLoop()->pause();

    //    // If a blueprint was being viewed, delete temporary scene object, and update blueprint JSON
    //    if (m_selectedBlueprint.sceneObjectJson().isEmpty()) {
    //        assert(false && "Error, blueprint must have been selected to have scene object");
    //    }
    //    m_selectedBlueprint = Blueprint(*m_blueprintSceneObject);

    //    m_blueprintSceneObject->removeFromScene();
    //    m_blueprintSceneObject = nullptr;

    //    //m_widgetManager->simulationLoop()->play();
    //}
//}

void ComponentTreeWidget::cachedClear()
{
    // Clear cache of items
    for (const ComponentItem* item : m_cachedItems) {
        delete item;
    }
    m_cachedItems.clear();

    // Cache items
    Int32_t numItems = topLevelItemCount();
    for (Int32_t i = 0; i < numItems; i++) {
        ComponentItem* item = static_cast<ComponentItem*>(topLevelItem(i));
        m_cachedItems.push_back(item);
        if (nullptr != m_cachedItems.back()->widget()) {
            m_cachedItems.back()->widget()->hide();
        }
    }

    // Then delete them
    while (0 != topLevelItemCount()) {
        takeTopLevelItem(0);
    }
}

void ComponentTreeWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTreeWidget::mouseReleaseEvent(event);
}

void ComponentTreeWidget::dropEvent(QDropEvent * event)
{
    ComponentTreeWidget* widget = static_cast<ComponentTreeWidget*>(event->source());
    if (widget == this) {
        ComponentItem* sourceItem = static_cast<ComponentItem*>(widget->currentItem());
        Q_UNUSED(sourceItem)
        QModelIndex destIndex = indexAt(event->pos());
        if (destIndex.isValid()) {
            // Dropping onto an item
            DropIndicatorPosition dropIndicator = dropIndicatorPosition();
            switch (dropIndicator) {
            case QAbstractItemView::AboveItem:
                // Dropping above an item
                // TODO: Implement reordering
                event->setDropAction(Qt::IgnoreAction);
                break;
            case QAbstractItemView::BelowItem:
                // Dropping below an item
                // TODO: Implement reordering
                event->setDropAction(Qt::IgnoreAction);
            case QAbstractItemView::OnItem:
            {
                // On an item, so ignore
                event->setDropAction(Qt::IgnoreAction);
            }
            case QAbstractItemView::OnViewport:
                // Not on the tree 
                event->setDropAction(Qt::IgnoreAction);
                break;
            }
        }
        else {
            // Dropping above an item
            event->setDropAction(Qt::IgnoreAction);
        }
    }
    else {
        // Ignore drops from another widget
        event->setDropAction(Qt::IgnoreAction);
    }
    QTreeWidget::dropEvent(event);
}

void ComponentTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    QTreeWidget::dragEnterEvent(event);
}

void ComponentTreeWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
    QTreeWidget::dragLeaveEvent(event);
}

void ComponentTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidget::dragMoveEvent(event);
}

void ComponentTreeWidget::removeItem(ComponentItem * componentItem)
{
    if ((int)componentItem->componentType() - 2000 < 0) {
        assert(false && "Error, item type is not implemented");
    }

    delete componentItem;
}

void ComponentTreeWidget::initializeWidget()
{
    setMinimumWidth(350);

    // Set tree widget settings
    setColumnCount(0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    // Enable drag and drop
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false); // is default, but making explicit for reference

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    /// Scene object components 

    // Initialize add renderer component action
    m_addShaderComponent = new QAction(tr("&New Shader Component"), this);
    m_addShaderComponent->setStatusTip("Create a new shader component");
    connect(m_addShaderComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager,
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eShader),
                "Create New Shader Component"));
        },
        Qt::DirectConnection);

    // Initialize add camera component action
    m_addCameraComponent = new QAction(tr("&New Camera Component"), this);
    m_addCameraComponent->setStatusTip("Create a new camera component");
    connect(m_addCameraComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager,
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eCamera),
                "Create New Camera"
                ));
        },
        Qt::DirectConnection);

    // Initialize add light component action
    m_addLightComponent = new QAction(tr("&New Light Component"), this);
    m_addLightComponent->setStatusTip("Create a new light component");
    connect(m_addLightComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eLight),
                "Create New Light"));
        },
        Qt::DirectConnection);

    // Initialize create script component action
    m_addScriptComponent = new QAction(tr("&New Script Component"), this);
    m_addScriptComponent->setStatusTip("Create a new script component");
    connect(m_addScriptComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager,
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eScript),
                "Create New Script"));
        },
        Qt::DirectConnection);

    // Initialize create model component action
    m_addModelComponent = new QAction(tr("&New Model Component"), this);
    m_addModelComponent->setStatusTip("Create a new Model component");
    connect(m_addModelComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId, 
                static_cast<Int32_t>(ESceneObjectComponentType::eModel),
                "Instantiate Model"
            ));
        },
        Qt::DirectConnection);

    // Initialize create model component action
    m_addAnimationComponent = new QAction(tr("&New Animation Component"), this);
    m_addAnimationComponent->setStatusTip("Create a new Animation component");
    connect(m_addAnimationComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager,
                m_currentSceneObjectId, 
                static_cast<Int32_t>(ESceneObjectComponentType::eBoneAnimation),
                "Instantiate Animation Component"
            ));
        },
        Qt::DirectConnection);

    // Initialize create event listener component action
    m_addListenerComponent = new QAction(tr("&New Event Listener Component"), this);
    m_addListenerComponent->setStatusTip("Create a new Event Listener component");
    connect(m_addListenerComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eListener),
                "Instantiate Listener"
                ));
        },
        Qt::DirectConnection);

    // Initialize create rigid body component action
    m_addRigidBodyComponent = new QAction(tr("&New Rigid Body Component"), this);
    m_addRigidBodyComponent->setStatusTip("Create a new Rigid Body component");
    connect(m_addRigidBodyComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eRigidBody),
                "Instantiate Rigid Body"
            ));
        },
        Qt::DirectConnection);

    // Initialize create canvas component action
    m_addCanvasComponent = new QAction(tr("&New Canvas Component"), this);
    m_addCanvasComponent->setStatusTip("Create a new Canvas component");
    connect(m_addCanvasComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager,
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eCanvas),
                "Instantiate Canvas"
            ));
        },
        Qt::DirectConnection);

    // Initialize create canvas component action
    m_addCharControllerComponent = new QAction(tr("&New Character Controller Component"), this);
    m_addCharControllerComponent->setStatusTip("Create a new Character Controller component");
    connect(m_addCharControllerComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId, 
                static_cast<Int32_t>(ESceneObjectComponentType::eCharacterController),
                "Instantiate Character Controller"
            ));
        },
        Qt::DirectConnection);

    // Initialize create cubemap component action
    m_addCubeMapComponent = new QAction(tr("&New Cube Map Component"), this);
    m_addCubeMapComponent->setStatusTip("Create a new Cube Map component");
    connect(m_addCubeMapComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eCubeMap),
                "Instantiate Cube Map"
            ));
        },
        Qt::DirectConnection);

    // Initialize create audio component action
    m_addAudioSourceComponent = new QAction(tr("&New Audio Source Component"), this);
    m_addAudioSourceComponent->setStatusTip("Create a new Audio Source component");
    connect(m_addAudioSourceComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
            m_widgetManager->actionManager()->performAction(
                new AddComponentCommand(
                    m_widgetManager, 
                    m_currentSceneObjectId, 
                    static_cast<Int32_t>(ESceneObjectComponentType::eAudioSource),
                    "Instantiate Audio Source"
                ));
        },
        Qt::DirectConnection);

    // Initialize create audio listener component action
    m_addAudioListenerComponent = new QAction(tr("&New Audio Listener Component"), this);
    m_addAudioListenerComponent->setStatusTip("Create a new Audio Listener component");
    connect(m_addAudioListenerComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {
        m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                m_currentSceneObjectId,
                static_cast<Int32_t>(ESceneObjectComponentType::eAudioListener),
                "Instantiate Audio Listener"
            ));
        },
        Qt::DirectConnection);


    // Scene components ====================================================================
    // Initialize create physics scene component action
    m_addPhysicsSceneComponent = new QAction(tr("&New Physics Scene Component"), this);
    m_addPhysicsSceneComponent->setStatusTip("Create a new Scene Physics component");
    connect(m_addPhysicsSceneComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new AddComponentCommand(
                m_widgetManager, 
                GSceneComponentType(ESceneComponentType::ePhysicsScene),
                "Instantiate Scene Physics"
            ));
        },
        Qt::DirectConnection);


    // Initialize delete component action
    m_deleteComponent = new QAction(tr("&Delete Component"), this);
    m_deleteComponent->setStatusTip("Delete the selected component");
    connect(m_deleteComponent,
        &QAction::triggered,
        m_widgetManager->actionManager(),
        [this] {m_widgetManager->actionManager()->performAction(
            new DeleteComponentCommand(m_widgetManager,
                m_currentSceneObjectId, 
                m_currentComponentItem->data().m_data.m_json,
                "Delete Component"));
        },
        Qt::DirectConnection);

    // Connect signal for double click events
    connect(this, &ComponentTreeWidget::itemDoubleClicked,
        this, &ComponentTreeWidget::onItemDoubleClicked,
        Qt::DirectConnection);

    // Connect signal for item expansion
    connect(this, &ComponentTreeWidget::itemExpanded,
        this, &ComponentTreeWidget::onItemExpanded,
        Qt::DirectConnection);

    // Connect signals for scene object selection and deselection
    SceneTreeWidget* sceneWidget = m_widgetManager->sceneTreeWidget();
    sceneWidget->m_selectedSceneObjectSignal.connect(this, &ComponentTreeWidget::selectSceneObject);
    sceneWidget->m_deselectedSceneObjectSignal.connect(this, &ComponentTreeWidget::clearSceneObject);

    // Connect signals for scene selection and deselection
    sceneWidget->m_selectedSceneSignal.connect(this, &ComponentTreeWidget::selectScene);
    sceneWidget->m_deselectedSceneSignal.connect(this, &ComponentTreeWidget::clearScene);
    sceneWidget->m_selectedSceneSignal.connect(this, &ComponentTreeWidget::selectScene);

    // Populate from blueprint widget as well
    connect(m_widgetManager->blueprintWidget(),
        &BlueprintListView::clicked,
        this,
        &ComponentTreeWidget::onSelectedBlueprint,
        Qt::DirectConnection);
}

#ifndef QT_NO_CONTEXTMENU

void ComponentTreeWidget::contextMenuEvent(QContextMenuEvent * event)
{
    if (m_currentSceneObjectId >= 0) {
        // If a scene object is selected

        // Create menu
        QMenu menu(this);

        // Add actions to the menu
        if (itemAt(event->pos())) {
            auto* item = static_cast<ComponentItem*>(itemAt(event->pos()));
            m_currentComponentItem = item;

            // Add action to delete component
            menu.addAction(m_deleteComponent);
        }
        else {
            // Options to add things when there is no item selected
            const json& sceneObjectJson = m_widgetManager->sceneObjectJson(m_currentSceneObjectId);
            const json& componentsJson = sceneObjectJson["components"];
            bool hasShaderComponent = componentsJson.end() != 
                std::find_if(componentsJson.begin(), componentsJson.end(),
                    [](const json& componentJson) {
                        return ESceneObjectComponentType::eShader == GSceneObjectComponentType(componentJson["type"].get<Int32_t>());
                    }
            );
            bool hasCameraComponent = componentsJson.end() !=
                std::find_if(componentsJson.begin(), componentsJson.end(),
                    [](const json& componentJson) {
                        return ESceneObjectComponentType::eCamera == GSceneObjectComponentType(componentJson["type"].get<Int32_t>());
                    }
            );

            if (!hasShaderComponent) {
                menu.addAction(m_addShaderComponent);
            }
            if (!hasCameraComponent) {
                menu.addAction(m_addCameraComponent);
            }
            // TODO: Set up component dependencies/constraints, and automatically add
            menu.addAction(m_addLightComponent);
            menu.addAction(m_addScriptComponent);
            menu.addAction(m_addModelComponent);
            menu.addAction(m_addAnimationComponent);
            menu.addAction(m_addListenerComponent);
            menu.addAction(m_addRigidBodyComponent);
            menu.addAction(m_addCanvasComponent);
            menu.addAction(m_addCharControllerComponent);
            menu.addAction(m_addCubeMapComponent);
            menu.addAction(m_addAudioSourceComponent);
            menu.addAction(m_addAudioListenerComponent);
        }

        // Display menu at click location
        menu.exec(event->globalPos());
    }
    else if (!m_currentSceneName.isEmpty()) {
        // Create menu
        QMenu menu(this);

        // Add actions to the menu
        if (itemAt(event->pos())) {
            auto* item = static_cast<ComponentItem*>(itemAt(event->pos()));
            m_currentComponentItem = item;

            // Add action to delete component
            menu.addAction(m_deleteComponent);
        }
        else {
            menu.addAction(m_addPhysicsSceneComponent);
        }

        // Display menu at click location
        menu.exec(event->globalPos());
    }
}
#endif

} // rev
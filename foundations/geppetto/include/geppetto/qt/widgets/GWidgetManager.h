#pragma once

// QT
#include <QtWidgets>
#include <QDockWidget>
#include <QMenuBar>
#include <QTimer>

// External
#include "fortress/json/GJson.h"
#include "fortress/layer/application/GManagerInterface.h"
#include "fortress/process/GProcessInterface.h"
#include "fortress/system/memory/GGarbageCollector.h"

#include "ripple/network/messages/GCubemapsDataMessage.h"
#include "ripple/network/messages/GModelDataMessage.h"
#include "ripple/network/messages/GTransformMessage.h"
#include "ripple/network/messages/GCanvasComponentDataMessage.h"
#include "ripple/network/messages/GAudioResourceDataMessage.h"
#include "ripple/network/messages/GAudioResourcesDataMessage.h"
#include "ripple/network/messages/GResourceAddedMessage.h"
#include "ripple/network/messages/GResourceModifiedMessage.h"
#include "ripple/network/messages/GResourceRemovedMessage.h"
#include "ripple/network/messages/GScenarioJsonMessage.h"
#include "ripple/network/messages/GResourceDataMessage.h"
#include "ripple/network/messages/GResourcesDataMessage.h"
#include "ripple/network/messages/GBlueprintsDataMessage.h"
#include "ripple/network/messages/GAdvanceProgressWidgetMessage.h"
#include "ripple/network/messages/GRequestAddTexturesToMaterialMessage.h"


class QMainWindow;

namespace rev {

class CoreEngine;
class GMessageGateway;
class MainWindow;
class ActionManager;

typedef GarbageCollector<QWidget> WidgetCollector;

class GLWidgetInterface;
class ResourceTreeWidget;
class SceneTreeWidget;
class ComponentTreeWidget;
class PlayerControls;
class ParameterWidget;
class ResourceListView;
class BlueprintListView;

/// @class Widget Manager
/// @brief Manager for controlling widgets and their layout
/// @note Multiple inheritance requires QObject flag be first
class WidgetManager: public QObject, public ManagerInterface, public ProcessInterface{
    Q_OBJECT
public:

	WidgetManager(QMainWindow* window);
	~WidgetManager();

    const json& sceneObjectJson(Uint32_t id) const;

    const json& sceneJson() const {
        return m_scenarioJson["scene"];
    }

    json& sceneJson() {
        return m_scenarioJson["scene"];
    }

    const json& scenarioJson() const {
        return m_scenarioJson;
    }

    void setScenarioJson(const json& j) {
        m_scenarioJson = j;
    }

    rev::BlueprintListView* blueprintWidget() const { return m_blueprintView; }

    GMessageGateway* messageGateway() { return m_gateway; }

    /// @brief Set the gateway of the widget manager
    void setMessageGateway(GMessageGateway* gateway);

    /// @brief Returns the main window for the application
    QMainWindow* mainWindow() { return m_mainWindow; }

    ActionManager* actionManager() { return m_actionManager; }
    void setActionManager(ActionManager* actionManager);

    /// @brief Returns the scene tree widget
    rev::SceneTreeWidget* sceneTreeWidget() { return m_sceneTreeWidget; }

    rev::ResourceTreeWidget* resourceTreeWidget() { return m_resourceTreeWidget; }

    /// @brief Returns the component widget
    rev::ComponentTreeWidget* componentWidget() { return m_componentWidget; }

    /// @brief The widget for controlling scene playback
    rev::PlayerControls* playbackWidget() { return m_playbackWidget; }

    virtual void onUpdate(double deltaSec) override final;
    virtual void onLateUpdate(double deltaSec) override final;
    virtual void onPostUpdate(double deltaSec) override final;

    rev::ParameterWidget* parameterWidget(const Uuid& id) {
        return m_parameterWidgets[id];
    }
    void addParameterWidget(ParameterWidget* widget);
    void removeParameterWidget(ParameterWidget* widget);

    /// @brief Called on main window resize
    void resize(int w, int h);

    /// @brief Get pointer to main GL widget
    rev::GLWidgetInterface* mainGLWidget() { return m_glWidget; }

    /// @brief Set the OpenGL widget
    void initializeGLWidget(rev::GLWidgetInterface* widget);

    /// @brief Clear the widget manager
    void clear();

    /// @brief Called after construction of the manager
    virtual void postConstruction() override;


	/// @}

signals:

    /// @brief Passing by value so that original message can be destructed
    void receivedCubemapsDataMessage(const GCubemapsDataMessage message);
    void receivedModelDataMessage(const GModelDataMessage message);
    void receivedTransformMessage(const GTransformMessage message);
    void receivedCanvasDataMessage(const GCanvasComponentDataMessage message);
    void receivedAudioResourceDataMessage(const GAudioResourceDataMessage message);
    void receivedAudioResourcesDataMessage(const GAudioResourcesDataMessage message);
    void receivedResourceAddedMessage(const GResourceAddedMessage message);
    void receivedResourceModifiedMessage(const GResourceModifiedMessage message);
    void receivedResourceRemovedMessage(const GResourceRemovedMessage message);
    void receivedScenarioJsonMessage(const GScenarioJsonMessage message);
    void receivedResourceDataMessage(const GResourceDataMessage message);
    void receivedResourcesDataMessage(const GResourcesDataMessage message);
    void receivedBlueprintsDataMessage(const GBlueprintsDataMessage message);
    void receivedAdvanceProgressWidgetMessage(const GAdvanceProgressWidgetMessage message);
    void receivedRequestAddTexturesToMaterialMessage(const GRequestAddTexturesToMaterialMessage message);

protected:

    friend class rev::MainWindow;
    friend class rev::ParameterWidget;

    /// @brief Find if a scene object JSON matches the given ID, returning a pointer to the JSON
    const json* getSceneObjectJson(Uint32_t id, const json& sceneObjectJson) const;

	/// @brief Initializes the OpenGL widget
	void initializeDefault();

    QMutex m_updateLock;
    QMainWindow* m_mainWindow{ nullptr }; ///< The main window for the application
    rev::GLWidgetInterface* m_glWidget{ nullptr }; ///<  The OpenGL widget
    tsl::robin_map<Uuid, ParameterWidget*> m_parameterWidgets; ///< Map of all parameter widgets

    ActionManager* m_actionManager{ nullptr };

    /// @note Refreshed on scene or scene object selection in component tree widget
    json m_scenarioJson; ///< Json representing the current scenario. Mirrors the application data

    /// @brief Resource widgets
    rev::ResourceTreeWidget* m_resourceTreeWidget{ nullptr };
    rev::ResourceListView* m_resourceListView{ nullptr };
    rev::BlueprintListView* m_blueprintView{ nullptr };

    rev::SceneTreeWidget* m_sceneTreeWidget{ nullptr }; ///< scene object tree widget
    rev::ComponentTreeWidget* m_componentWidget{ nullptr }; ///< scene object inspector widget
    rev::PlayerControls* m_playbackWidget{ nullptr }; ///< scene playback widget

    /// @brief Docks in the main window
    QDockWidget* m_sceneDock{ nullptr };
    QDockWidget* m_resourceTreeDock{ nullptr };
    QDockWidget* m_bottomDock{ nullptr };
    QDockWidget* m_rightDock{ nullptr };
    QDockWidget* m_topDock{ nullptr };

    GMessageGateway* m_gateway{ nullptr }; ///< Gateway for sending/receiving messages from main application
};



// End namespacing
}

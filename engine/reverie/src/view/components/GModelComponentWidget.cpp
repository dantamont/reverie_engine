#include "GModelComponentWidget.h"
#include "../parameters/GRenderableWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"

#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GScriptComponent.h"
#include "../../core/components/GCameraComponent.h"

#include "../style/GFontIcon.h"
#include "../../core/geometry/GEulerAngles.h"
#include "../../core/components/GModelComponent.h"

namespace rev {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ModelSelectWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelSelectWidget::ModelSelectWidget(CoreEngine* core,
    ModelComponent* modelComponent,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_modelComponent(modelComponent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelSelectWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelSelectWidget::initializeWidgets()
{
    m_modelSelect = new QComboBox();
    populateModels();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelSelectWidget::initializeConnections()
{
    // Model selection
    connect(m_modelSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        Q_UNUSED(index);

        if (index < 0) return; // When widget is visible while creating a model

        pauseSimulation();

        //QString modelName = m_modelSelect->currentText();
        Uuid modelID = Uuid(m_modelSelect->currentData().toString());
        std::shared_ptr<ResourceHandle> modelHandle = 
            m_engine->resourceCache()->getHandle(modelID);
        
        if (modelHandle->getResourceType() != ResourceType::kModel) {
            throw("Error, handle is not a model");
        }
        if (!modelHandle) { 
            throw("Error, no model with specified name found");
        }
        
        m_modelComponent->setModelHandle(modelHandle);

        resumeSimulation();
    }
    );

    // Repopulate models on model load
    connect(m_engine->resourceCache(), &ResourceCache::resourceAdded,
        this,
        [this](const Uuid& uuid) {
        const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandle(uuid);
        if (handle->getResourceType() == ResourceType::kModel) {
            populateModels();
        }
    });
    connect(m_engine->resourceCache(), &ResourceCache::resourceDeleted,
        this,
        [this](const Uuid& uuid) {
        const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandle(uuid);
        if (handle->getResourceType() == ResourceType::kModel) {
            populateModels();
        }
    });
    connect(m_engine->resourceCache(), &ResourceCache::resourceChanged,
        this,
        [this](const Uuid& uuid) {
        const std::shared_ptr<ResourceHandle>& handle = m_engine->resourceCache()->getHandle(uuid);
        if (handle->getResourceType() == ResourceType::kModel) {
            populateModels();
        }
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelSelectWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_modelSelect);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelSelectWidget::populateModels()
{
    m_modelSelect->blockSignals(true);
    m_modelSelect->clear();
    int idx = 0;
    size_t count = 0;

    // Models can only be a top-level resource
    for (const auto& resource : m_engine->resourceCache()->topLevelResources()) {
        if (!resource->isConstructed()) { continue; }
        if (resource->getResourceType() != ResourceType::kModel) { continue; }
        m_modelSelect->addItem(resource->getName().c_str(), resource->getUuid().asQString());
        if (m_modelComponent->modelHandle()) {
            // There may be no model component on widget construction
            if (resource->getUuid() == m_modelComponent->modelHandle()->getUuid()) {
                idx = count;
            }
        }
        count++;
    }

    // Set index, making sure it does not exceed the number of models
    m_modelSelect->setCurrentIndex(std::max(idx, 0));
    m_modelSelect->blockSignals(false);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ModelComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponentWidget::ModelComponentWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent),
    m_modelComponent(static_cast<ModelComponent*>(m_component))
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponent* ModelComponentWidget::modelComponent() const {
    return m_modelComponent;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
ModelComponentWidget::~ModelComponentWidget()
{
    delete m_model;
    delete m_renderSettings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_renderSettings = new RenderSettingsWidget(m_engine, m_modelComponent->renderSettings());
    m_model = new ModelSelectWidget(m_engine, m_modelComponent);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Model:"), m_model));
    m_mainLayout->addWidget(m_renderSettings);
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
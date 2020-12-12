#include "GbModelComponentWidget.h"
#include "../parameters/GbRenderableWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/components/GbCameraComponent.h"

#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"
#include "../../core/components/GbModelComponent.h"

namespace Gb {
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
        
        if (modelHandle->getResourceType() != Resource::kModel) {
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
        [&](std::shared_ptr<ResourceHandle> handle) {
        if (handle->getResourceType() == Resource::kModel)
            populateModels();
    });
    connect(m_engine->resourceCache(), &ResourceCache::resourceDeleted,
        this,
        [&](std::shared_ptr<ResourceHandle> handle) {
        if (handle->getResourceType() == Resource::kModel)
            populateModels();
    });
    connect(m_engine->resourceCache(), &ResourceCache::resourceChanged,
        this,
        [&](std::shared_ptr<ResourceHandle> handle) {
        if (handle->getResourceType() == Resource::kModel)
            populateModels();
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
    for (const auto& modelPair : m_engine->resourceCache()->models()) {
        if (!modelPair.second->isConstructed()) continue;
        m_modelSelect->addItem(modelPair.second->getName().c_str(), 
            modelPair.second->getUuid().asQString());
        if (m_modelComponent->modelHandle()) {
            // There may be no model component on widget construction
            if (modelPair.second->getUuid() == m_modelComponent->modelHandle()->getUuid()) {
                idx = count;
            }
        }
        count++;
    }
    m_modelSelect->setCurrentIndex(std::min(idx, (int)m_engine->resourceCache()->models().size() - 1));
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
    delete m_renderable;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_renderable = new RenderableWidget<ModelComponent>(m_engine, m_modelComponent);
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
    m_mainLayout->addWidget(m_renderable);
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb
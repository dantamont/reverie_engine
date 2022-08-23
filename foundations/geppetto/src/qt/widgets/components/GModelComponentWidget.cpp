#include "geppetto/qt/widgets/components/GModelComponentWidget.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/style/GFontIcon.h"

#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/json/GJson.h"

namespace rev {


ModelSelectWidget::ModelSelectWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_componentJson(componentJson),
    m_sceneObjectId(sceneObjectId)
{
    initialize();
    m_setModelMessage.setSceneObjectId(sceneObjectId);
}

void ModelSelectWidget::update()
{
}

void ModelSelectWidget::requestPopulateModels()
{
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestModelMessage);
}

void ModelSelectWidget::initializeWidgets()
{
    m_modelSelect = new QComboBox();
    requestPopulateModels();
}

void ModelSelectWidget::initializeConnections()
{
    // Model selection
    connect(m_modelSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        Q_UNUSED(index);

        if (index < 0) return; // When widget is visible while creating a model

        Uuid modelId = Uuid(m_modelSelect->currentData().toString().toStdString());
        m_setModelMessage.setModelId(modelId);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_setModelMessage);

    }
    );

    connect(m_widgetManager, &WidgetManager::receivedModelDataMessage, this, &ModelSelectWidget::populateModels);

    /// @todo Repopulate models on model load
    /// This shouldn't be too painful. Just need to connect a signal from the resource cache to the widget message gateway, then send a message form there
    //connect(&ResourceCache::Instance(), &ResourceCache::resourceAdded,
    //    this,
    //    [this](const Uuid& uuid) {
    //    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandle(uuid);
    //    if (handle->getResourceType() == EResourceType::eModel) {
    //        requestPopulateModels();
    //    }
    //});
    //connect(&ResourceCache::Instance(), &ResourceCache::resourceDeleted,
    //    this,
    //    [this](const Uuid& uuid) {
    //    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandle(uuid);
    //    if (handle->getResourceType() == EResourceType::eModel) {
    //        requestPopulateModels();
    //    }
    //});
    //connect(&ResourceCache::Instance(), &ResourceCache::resourceChanged,
    //    this,
    //    [this](const Uuid& uuid) {
    //    const std::shared_ptr<ResourceHandle>& handle = ResourceCache::Instance().getHandle(uuid);
    //    if (handle->getResourceType() == EResourceType::eModel) {
    //        requestPopulateModels();
    //    }
    //});
}

void ModelSelectWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_modelSelect);
}

void ModelSelectWidget::populateModels(const GModelDataMessage& message)
{
    m_modelSelect->blockSignals(true);
    m_modelSelect->clear();
    int idx = 0;
    uint32_t count = 0;

    // Populate the combobox
    const std::vector<Uuid>& modelIds = message.getModelIds();
    const std::vector<GStringFixedSize<>>& modelNames = message.getModelNames();
    for (Uint32_t i = 0; i < modelNames.size(); i++) {
        m_modelSelect->addItem(modelNames[i].c_str(), modelIds[i].asString().c_str());
        if (m_componentJson.contains("model")) {
            if (modelNames[i] == m_componentJson["model"].get_ref<const std::string&>()) {
                idx = count;
            }
        }
        count++;
    }

    // Set index, making sure it does not exceed the number of models
    m_modelSelect->setCurrentIndex(std::max(idx, 0));
    m_modelSelect->blockSignals(false);
}





// ModelComponentWidget

ModelComponentWidget::ModelComponentWidget(WidgetManager* wm, json& componentJson, Uint32_t sceneObjectId, QWidget *parent) :
    ComponentWidget(wm, componentJson, sceneObjectId, parent)
{
    initialize();
}

ModelComponentWidget::~ModelComponentWidget()
{
    delete m_model;
    delete m_renderSettings;
}

void ModelComponentWidget::update()
{
}

void ModelComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_renderSettings = new RenderSettingsWidget(m_widgetManager, m_componentJson["renderSettings"], RenderSettingsOwnerType::kModelComponent, m_sceneOrObjectId);
    m_model = new ModelSelectWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId);
}

void ModelComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}

void ModelComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Model:"), m_model));
    m_mainLayout->addWidget(m_renderSettings);
}

} // rev
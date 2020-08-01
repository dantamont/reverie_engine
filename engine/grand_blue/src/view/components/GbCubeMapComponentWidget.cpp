#include "GbCubeMapComponentWidget.h"
#include "../parameters/GbRenderableWidget.h"
#include "../parameters/GbColorWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbScene.h"
#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbScriptComponent.h"
#include "../../core/components/GbCamera.h"

#include "../../core/rendering/renderer/GbRenderers.h"
#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"
#include "../../core/components/GbCubeMapComponent.h"

namespace Gb {
namespace View {


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//// ModelSelectWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////
//ModelSelectWidget::ModelSelectWidget(CoreEngine* core,
//    CubeMapComponent* modelComponent,
//    QWidget* parent) :
//    ParameterWidget(core, parent),
//    m_cubeMapComponent(modelComponent)
//{
//    initialize();
//
//    // Repopulate models on model load
//    connect(m_engine->resourceCache(), &ResourceCache::resourceAdded,
//        this,
//        [&](std::shared_ptr<ResourceHandle> handle) {
//        if (handle->getResourceType() == Resource::kModel)
//            populateModels();
//    });
//    connect(m_engine->resourceCache(), &ResourceCache::resourceDeleted,
//        this,
//        [&](std::shared_ptr<ResourceHandle> handle) {
//        if (handle->getResourceType() == Resource::kModel)
//            populateModels();
//    });
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelSelectWidget::update()
//{
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelSelectWidget::initializeWidgets()
//{
//    m_modelSelect = new QComboBox();
//    populateModels();
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelSelectWidget::initializeConnections()
//{
//    // Model selection
//    connect(m_modelSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
//        [this](int index) {
//        Q_UNUSED(index);
//        pauseSimulation();
//
//        QString modelName = m_modelSelect->currentText();
//        std::shared_ptr<ResourceHandle> modelHandle = 
//            m_engine->resourceCache()->getHandleWithName(modelName, Resource::kModel);
//        
//        if (!modelHandle) throw("Error, no model with specified name found");
//        
//        m_cubeMapComponent->setModelHandle(modelHandle);
//
//        resumeSimulation();
//    }
//    );
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelSelectWidget::layoutWidgets()
//{
//    m_mainLayout = new QVBoxLayout();
//    m_mainLayout->setSpacing(0);
//    m_mainLayout->addWidget(m_modelSelect);
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void ModelSelectWidget::populateModels()
//{
//    m_modelSelect->clear();
//    for (const auto& modelPair : m_engine->resourceCache()->models()) {
//        if (!modelPair.second->isConstructed()) continue;
//        m_modelSelect->addItem(modelPair.second->getName());
//    }
//}
//


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CubeMapComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponentWidget::CubeMapComponentWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent),
    m_cubeMapComponent(static_cast<CubeMapComponent*>(m_component))
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponent* CubeMapComponentWidget::cubeMapComponent() const {
    return m_cubeMapComponent;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CubeMapComponentWidget::~CubeMapComponentWidget()
{
    delete m_textureLoad;
    //delete m_renderable;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponentWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    //m_renderable = new RenderableWidget<CubeMapComponent>(m_engine, m_cubeMapComponent);
    m_nameEdit = new QLineEdit(m_cubeMapComponent->getName());
    m_textureLoad = new FileLoadWidget(m_engine);
    if (m_cubeMapComponent->textureHandle()) {
        m_textureLoad->lineEdit()->setText(m_cubeMapComponent->textureHandle()->getPath());
    }
    m_isDefaultCubeMap = new QCheckBox("Default");
    m_isDefaultCubeMap->setToolTip("Whether or not this is the default cubemap used for cameras");
    m_isDefaultCubeMap->setChecked(m_cubeMapComponent->isDefault());
    m_colorWidget = new ColorWidget(m_engine, m_cubeMapComponent->diffuseColor());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to set name
    connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {

        const QString& text = m_nameEdit->text();
        m_cubeMapComponent->setName(text);
    });

    // Make connection to load texture
    connect(m_textureLoad->lineEdit(), &QLineEdit::textChanged, this, [this]() {

        const QString& text = m_textureLoad->lineEdit()->text();
        m_cubeMapComponent->setCubeTexture(text);
    });

    // Make connection to make default
    connect(m_isDefaultCubeMap,
        &QCheckBox::stateChanged, this, [this](int state) {
        bool checked = state != 0;
        if (checked) {
            m_cubeMapComponent->setDefault();
        }
        else {
            m_cubeMapComponent->sceneObject()->scene()->setDefaultCubeMap(nullptr);
        }
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CubeMapComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    //m_mainLayout->addWidget(m_renderable);
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Name:"), m_nameEdit));
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Texture:"), m_textureLoad));
    m_mainLayout->addWidget(m_isDefaultCubeMap);
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Color: "), m_colorWidget));
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb
#include "geppetto/qt/widgets/components/GCubeMapComponentWidget.h"
#include "geppetto/qt/widgets/types/GRenderableWidget.h"
#include "geppetto/qt/widgets/types/GColorWidget.h"

#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "fortress/json/GJson.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "fortress/containers/math/GEulerAngles.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

namespace rev {

CubeMapComponentWidget::CubeMapComponentWidget(WidgetManager* wm, const json& componentJson, Int32_t sceneObjectId, QWidget *parent) :
    SceneObjectComponentWidget(wm, componentJson, sceneObjectId, parent)
{
    initialize();
}

CubeMapComponentWidget::~CubeMapComponentWidget()
{
    delete m_textureLoad;
}

void CubeMapComponentWidget::update()
{
}

void CubeMapComponentWidget::requestChangeCubemapColor(const Color& color)
{
    m_componentJson["diffuseColor"] = color;
    m_colorMessage.setDiffuseColor(color);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_colorMessage);
}

void CubeMapComponentWidget::requestChangeCubemapName(const std::string& name)
{
    m_componentJson["name"] = name;
    m_nameMessage.setCubemapName(name);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_nameMessage);
}

void CubeMapComponentWidget::requestChangeCubemapTexture(const std::string& path)
{
    m_componentJson["filePath"] = path;
    m_textureMessage.setTextureFilePath(path);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_textureMessage);
}

void CubeMapComponentWidget::requestSetDefaultCubemap(bool isDefault)
{
    m_componentJson["isDefault"] = isDefault;
    m_defaultCubemapMessage.setSetDefault(isDefault);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_defaultCubemapMessage);
}

void CubeMapComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_nameEdit = new QLineEdit(m_componentJson["name"].get_ref<const std::string&>().c_str());
    m_textureLoad = new FileLoadWidget(m_widgetManager);
    if (m_componentJson.contains("texture")) {
        m_textureLoad->lineEdit()->setText(m_componentJson["texture"]["filePath"].get_ref<const std::string&>().c_str());
    }
    m_isDefaultCubeMap = new QCheckBox("Default");
    m_isDefaultCubeMap->setToolTip("Whether or not this is the default cubemap used for cameras");
    m_isDefaultCubeMap->setChecked(m_componentJson["isDefault"]);

    m_cachedDiffuseColor = m_componentJson["diffuseColor"];
    m_colorWidget = new ColorWidget(m_widgetManager, m_cachedDiffuseColor);
}

void CubeMapComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Make connection to set color
    connect(m_colorWidget, &ColorWidget::colorChanged, this,
        [this](const Color& color) {
            m_cachedDiffuseColor = color;
            requestChangeCubemapColor(color);
        }
    );

    // Make connection to set name
    connect(m_nameEdit, &QLineEdit::editingFinished, this, 
        [this]() {
            const QString& text = m_nameEdit->text();
            requestChangeCubemapName(text.toStdString());
        }
    );

    // Make connection to load texture
    connect(m_textureLoad->lineEdit(), &QLineEdit::textChanged, this, 
        [this]() {
            const QString& text = m_textureLoad->lineEdit()->text();
            requestChangeCubemapTexture(text.toStdString());
        }
    );

    // Make connection to make default
    connect(m_isDefaultCubeMap,
        &QCheckBox::stateChanged, this, 
        [this](int state) {
            bool checked = state != 0;
            requestSetDefaultCubemap(checked);
        }
    );
}

void CubeMapComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    //m_mainLayout->addWidget(m_renderable);
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Name:"), m_nameEdit));
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Texture:"), m_textureLoad));
    m_mainLayout->addWidget(m_isDefaultCubeMap);
    m_mainLayout->addLayout(LabeledLayout(QStringLiteral("Color: "), m_colorWidget));
}


} // rev
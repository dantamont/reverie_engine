#include "geppetto/qt/widgets/components/GCameraComponentWidget.h"

#include "geppetto/qt/widgets/components/GComponentWidget.h"

#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/layer/framework/GFlags.h"
#include "fortress/json/GJson.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/types/GRenderLayerWidgets.h"
#include "geppetto/qt/widgets/graphics/GGLWidgetInterface.h"

#include "enums/GRenderProjectionTypeEnum.h"
#include "ripple/network/gateway/GMessageGateway.h"

namespace rev {

CameraSubWidget::CameraSubWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_cameraComponentJson(cameraComponentJson),
    m_sceneObjectId(sceneObjectId)
{
}

bool CameraSubWidget::checkCameraOptionFlag(ECameraOption option) const
{
    return getCameraOptions().testFlag(option);
}

json& CameraSubWidget::getRenderLayers()
{
    return m_cameraComponentJson["camera"]["renderLayers"];
}

ECameraOptions CameraSubWidget::getCameraOptions() const
{
    return ECameraOptions(m_cameraComponentJson["camera"]["cameraOptions"].get<Uint32_t>());
}


CameraOptionsWidget::CameraOptionsWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent) :
    CameraSubWidget(wm, cameraComponentJson, sceneObjectId, parent)
{
    initialize();
    m_optionsMessage.setSceneObjectId(m_sceneObjectId);
    m_clearColorMessage.setSceneObjectId(m_sceneObjectId);
}

void CameraOptionsWidget::update()
{
}

void CameraOptionsWidget::initializeWidgets()
{
    m_areaWidget = new QWidget();
    QVBoxLayout* areaLayout = new QVBoxLayout;
    m_checkBoxes.push_back(new QCheckBox("Frustum Culling"));
    m_checkBoxes.push_back(new QCheckBox("Occlusion Culling"));
    m_checkBoxes.push_back(new QCheckBox("Show All Render Layers"));
    m_checkBoxes.push_back(new QCheckBox("Enable Post-Processing"));

    m_checkBoxes[0]->setChecked(checkCameraOptionFlag(ECameraOption::eFrustrumCulling));
    m_checkBoxes[1]->setChecked(checkCameraOptionFlag(ECameraOption::eOcclusionCulling));
    m_checkBoxes[2]->setChecked(checkCameraOptionFlag(ECameraOption::eShowAllRenderLayers));
    m_checkBoxes[3]->setChecked(checkCameraOptionFlag(ECameraOption::eEnablePostProcessing));

    m_cachedClearColor = getCameraClearColor();
    m_clearColor = new ColorWidget(m_widgetManager, m_cachedClearColor);

    areaLayout->addWidget(m_checkBoxes[0]);
    areaLayout->addWidget(m_checkBoxes[1]);
    areaLayout->addWidget(m_checkBoxes[2]);
    areaLayout->addWidget(m_checkBoxes[3]);

    m_areaWidget->setLayout(areaLayout);

    m_area = new QScrollArea();
    m_area->setWidget(m_areaWidget);
}

void CameraOptionsWidget::initializeConnections()
{
    // Color
    connect(m_clearColor, &ColorWidget::colorChanged, this, 
        [this](const Color& color) {
            m_clearColorMessage.setColor(color);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_clearColorMessage);
        }
    );

    // Camera options
    connect(m_checkBoxes[0], &QCheckBox::stateChanged,
        this,
        [&](int state) {
            bool checked = state == 0 ? false : true;
            ECameraOptions cameraOptions = getCameraOptions();
            cameraOptions.setFlag(ECameraOption::eFrustrumCulling, checked);
            m_cameraComponentJson["camera"]["cameraOptions"] = (Uint32_t)cameraOptions;
            m_optionsMessage.setOptions(static_cast<Uint32_t>(cameraOptions));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_optionsMessage);
        }
    );

    connect(m_checkBoxes[1], &QCheckBox::stateChanged,
        this,
        [&](int state) {
            bool checked = state == 0 ? false : true;
            ECameraOptions cameraOptions = getCameraOptions();
            cameraOptions.setFlag(ECameraOption::eOcclusionCulling, checked);
            m_cameraComponentJson["camera"]["cameraOptions"] = (Uint32_t)cameraOptions;
            m_optionsMessage.setOptions(static_cast<Uint32_t>(cameraOptions));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_optionsMessage);
        }
    );

    connect(m_checkBoxes[2], &QCheckBox::stateChanged,
        this,
        [&](int state) {
            bool checked = state == 0 ? false : true;
            ECameraOptions cameraOptions = getCameraOptions();
            cameraOptions.setFlag(ECameraOption::eShowAllRenderLayers, checked);
            m_cameraComponentJson["camera"]["cameraOptions"] = (Uint32_t)cameraOptions;
            m_optionsMessage.setOptions(static_cast<Uint32_t>(cameraOptions));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_optionsMessage);
        }
    );

    connect(m_checkBoxes[3], &QCheckBox::stateChanged,
        this,
        [&](int state) {
            bool checked = state == 0 ? false : true;
            ECameraOptions cameraOptions = getCameraOptions();
            cameraOptions.setFlag(ECameraOption::eEnablePostProcessing, checked);
            m_cameraComponentJson["camera"]["cameraOptions"] = (Uint32_t)cameraOptions;
            m_optionsMessage.setOptions(static_cast<Uint32_t>(cameraOptions));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_optionsMessage);
        }
    );
}

void CameraOptionsWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_area);
}

Color CameraOptionsWidget::getCameraClearColor()
{
    return m_cameraComponentJson["camera"]["clearColor"].get<Color>();
}


CameraViewportWidget::CameraViewportWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent) :
    CameraSubWidget(wm, cameraComponentJson, sceneObjectId, parent)
{
    initialize();
    m_viewportMessage.setSceneObjectId(m_sceneObjectId);
}

void CameraViewportWidget::update()
{
}

json& CameraViewportWidget::getViewportJson()
{
    return m_cameraComponentJson["camera"]["viewport"];
}

json& CameraViewportWidget::getViewportX()
{
    return getViewportJson()["x"];
}

json& CameraViewportWidget::getViewportY()
{
    return getViewportJson()["y"];
}

json& CameraViewportWidget::getViewportWidth()
{
    return getViewportJson()["w"];
}

json& CameraViewportWidget::getViewportHeight()
{
    return getViewportJson()["h"];
}

json& CameraViewportWidget::getViewportDepth()
{
    return getViewportJson()["d"];
}

void CameraViewportWidget::initializeWidgets()
{
    const json& viewportJson = getViewportJson();
    m_depth = new QLineEdit();
    m_depth->setMaximumWidth(50);
    m_depth->setValidator(new QIntValidator(-1e8, 1e8));
    m_depth->setToolTip("depth at which to render camera viewport");
    m_depth->setText(QString::number(getViewportDepth().get<Int32_t>()));

    m_xCoordinate = new QLineEdit();
    m_xCoordinate->setMaximumWidth(50);
    m_xCoordinate->setValidator(new QDoubleValidator(-1e8, 1e8, 10));
    m_xCoordinate->setToolTip("x-coordinate in normalized screen space (homogeneous coordinates, [-1, 1])");
    m_xCoordinate->setText(QString::number(getViewportX().get<Float64_t>()));

    m_yCoordinate = new QLineEdit();
    m_yCoordinate->setMaximumWidth(50);
    m_yCoordinate->setValidator(new QDoubleValidator(-1e8, 1e8, 10));
    m_yCoordinate->setToolTip("y-coordinate in normalized screen space (homogeneous coordinates, [-1, 1])");
    m_yCoordinate->setText(QString::number(getViewportY().get<Float64_t>()));

    m_width = new QLineEdit();
    m_width->setMaximumWidth(50);
    m_width->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_width->setToolTip("Viewport width in normalized screen space");
    m_width->setText(QString::number(getViewportWidth().get<Float64_t>()));

    m_height = new QLineEdit();
    m_height->setMaximumWidth(50);
    m_height->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_height->setToolTip("Viewport height in normalized screen space");
    m_height->setText(QString::number(getViewportHeight().get<Float64_t>()));
}

void CameraViewportWidget::initializeConnections()
{
    // Depth
    connect(m_depth, &QLineEdit::editingFinished, this,
        [this]() {
        int depth = m_depth->text().toInt();
            getViewportDepth() = depth;
            m_viewportMessage.setJsonBytes(GJson::ToBytes(getViewportJson()));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_viewportMessage);
        }
    );

    // x coordinate
    connect(m_xCoordinate, &QLineEdit::editingFinished, this,
        [this]() {
            double x = m_xCoordinate->text().toDouble();
            getViewportX() = x;
            m_viewportMessage.setJsonBytes(GJson::ToBytes(getViewportJson()));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_viewportMessage);
        }
    );

    // y coordinate 
    connect(m_yCoordinate, &QLineEdit::editingFinished, this,
        [this]() {
            double y = m_yCoordinate->text().toDouble();
            getViewportY() = y;
            m_viewportMessage.setJsonBytes(GJson::ToBytes(getViewportJson()));
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_viewportMessage);
        }
    );

    // width
    connect(m_width, &QLineEdit::editingFinished, this,
        [this]() {
        double width = m_width->text().toDouble();
        getViewportWidth() = width;
        m_viewportMessage.setJsonBytes(GJson::ToBytes(getViewportJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_viewportMessage);
    });

    // height
    connect(m_height, &QLineEdit::editingFinished, this,
        [this]() {
        double height = m_height->text().toDouble();
        getViewportHeight() = height;
        m_viewportMessage.setJsonBytes(GJson::ToBytes(getViewportJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_viewportMessage);
    });
}

void CameraViewportWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();

    QScreen* screen = QGuiApplication::primaryScreen();
    float res = screen->logicalDotsPerInch();

    QBoxLayout* coordinateLayout = new QHBoxLayout;
    AddLabel(SAIcon("arrows-alt"), coordinateLayout, { res * 0.25f, res * 0.25f});
    coordinateLayout->addSpacing(15);
    AddLabel("x:", coordinateLayout);
    coordinateLayout->addWidget(m_xCoordinate);
    coordinateLayout->addSpacing(15);
    AddLabel("y:", coordinateLayout);
    coordinateLayout->addWidget(m_yCoordinate);
    coordinateLayout->addSpacing(15);
    AddLabel("z:", coordinateLayout);
    coordinateLayout->addWidget(m_depth);
    m_mainLayout->addLayout(coordinateLayout);

    QBoxLayout* sizeLayout = new QHBoxLayout;
    AddLabel(SAIcon("expand-arrows-alt"), sizeLayout, { res * 0.25f, res * 0.25f });
    sizeLayout->addSpacing(15);
    AddLabel("w:", sizeLayout);
    sizeLayout->addWidget(m_width);
    sizeLayout->addSpacing(15);
    AddLabel("h:", sizeLayout);
    sizeLayout->addWidget(m_height);
    m_mainLayout->addLayout(sizeLayout);
}



CameraProjectionWidget::CameraProjectionWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget* parent) :
    CameraSubWidget(wm, cameraComponentJson, sceneObjectId, parent)
{
    initialize();
    m_projectionMessage.setSceneObjectId(m_sceneObjectId);
}

void CameraProjectionWidget::update()
{
}

void CameraProjectionWidget::initializeWidgets()
{
    json& projectionJson = getProjectionJson();
    m_projectionType = new QComboBox();
    m_projectionType->addItem(SAIcon("image"), "Perspective");
    m_projectionType->addItem(SAIcon("border-all"), "Orthographic");

    // Perspective
    m_fov = new QLineEdit();
    m_fov->setMaximumWidth(50);
    m_fov->setValidator(new QDoubleValidator(0, 179.0, 4));
    m_fov->setToolTip("Horizontal field of view (in degrees)");
    m_fov->setText(QString::number(getFovX().get<Float64_t>()));

    m_near = new QLineEdit();
    m_near->setMaximumWidth(50);
    m_near->setValidator(new QDoubleValidator(-1e20, 1e20, 15));
    m_near->setToolTip("Near clip plane");
    m_near->setText(QString::number(getNearClipPlane().get<Float64_t>()));

    m_far = new QLineEdit();
    m_far->setMaximumWidth(50);
    m_far->setValidator(new QDoubleValidator(-1e20, 1e20, 15));
    m_far->setToolTip("Far clip plane");
    m_far->setText(QString::number(getFarClipPlane().get<Float64_t>()));

    // Orthographic
    m_left = new QLineEdit();
    m_left->setMaximumWidth(50);
    m_left->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_left->setToolTip("Left bound");
    m_left->setText(QString::number(getLeftBound().get<Float64_t>()));

    m_top = new QLineEdit();
    m_top->setMaximumWidth(50);
    m_top->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_top->setToolTip("Top bound");
    m_top->setText(QString::number(getTopBound().get<Float64_t>()));

    m_right = new QLineEdit();
    m_right->setMaximumWidth(50);
    m_right->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_right->setToolTip("Right bound");
    m_right->setText(QString::number(getRightBound().get<Float64_t>()));

    m_bottom = new QLineEdit();
    m_bottom->setMaximumWidth(50);
    m_bottom->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_bottom->setToolTip("Bottom bound");
    m_bottom->setText(QString::number(getBottomBound().get<Float64_t>()));
}

void CameraProjectionWidget::initializeConnections()
{
    // Projection type
    connect(m_projectionType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        getProjectionType() = index;
        m_stackedWidget->setCurrentIndex(index);

        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // FOV
    connect(m_fov, &QLineEdit::editingFinished, this,
        [this]() {
        double fov = m_fov->text().toDouble();
        getFovX() = fov;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Near plane
    connect(m_near, &QLineEdit::editingFinished, this,
        [this]() {
        double nearPlane = m_near->text().toDouble();
        getNearClipPlane() = nearPlane;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Far plane
    connect(m_far, &QLineEdit::editingFinished, this,
        [this]() {
        double farPlane = m_far->text().toDouble();
        getFarClipPlane() = farPlane;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Left
    connect(m_left, &QLineEdit::editingFinished, this,
        [this]() {
        double left = m_left->text().toDouble();
        getLeftBound() = left;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Top
    connect(m_top, &QLineEdit::editingFinished, this,
        [this]() {
        double top = m_top->text().toDouble();
        getTopBound() = top;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Right
    connect(m_right, &QLineEdit::editingFinished, this,
        [this]() {
        double right = m_right->text().toDouble();
        getRightBound() = right;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });

    // Bottom
    connect(m_bottom, &QLineEdit::editingFinished, this,
        [this]() {
        double bottom = m_bottom->text().toDouble();
        getBottomBound() = bottom;
        m_projectionMessage.setJsonBytes(GJson::ToBytes(getProjectionJson()));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_projectionMessage);
    });
}

void CameraProjectionWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    m_mainLayout->addWidget(m_projectionType);

    QWidget* perspective = new QWidget();
    QBoxLayout* perspectiveLayout = new QHBoxLayout();
    perspectiveLayout->addWidget(new QLabel("FOV:"));
    perspectiveLayout->addWidget(m_fov);
    perspectiveLayout->addWidget(new QLabel("Near:"));
    perspectiveLayout->addWidget(m_near);
    perspectiveLayout->addWidget(new QLabel("Far:"));
    perspectiveLayout->addWidget(m_far);
    perspective->setLayout(perspectiveLayout);

    QWidget* orthographic = new QWidget();
    QBoxLayout* orthoLayout = new QHBoxLayout();
    orthoLayout->addWidget(new QLabel("L:"));
    orthoLayout->addWidget(m_left);
    orthoLayout->addWidget(new QLabel("T:"));
    orthoLayout->addWidget(m_top);
    orthoLayout->addWidget(new QLabel("R:"));
    orthoLayout->addWidget(m_right);
    orthoLayout->addWidget(new QLabel("B:"));
    orthoLayout->addWidget(m_bottom);
    orthographic->setLayout(orthoLayout);

    m_stackedWidget = new QStackedWidget();
    m_stackedWidget->addWidget(perspective);
    m_stackedWidget->addWidget(orthographic);
    m_stackedWidget->setCurrentIndex(getProjectionType().get<Uint32_t>());
    m_mainLayout->addWidget(m_stackedWidget);
}

json& CameraProjectionWidget::getProjectionJson()
{
    return m_cameraComponentJson["camera"]["projection"];
}

json& CameraProjectionWidget::getFovX()
{
    return getProjectionJson()["fov"];
}

json& CameraProjectionWidget::getNearClipPlane()
{
    return getProjectionJson()["zNear"];
}

json& CameraProjectionWidget::getFarClipPlane()
{
    return getProjectionJson()["zFar"];
}

json& CameraProjectionWidget::getLeftBound()
{
    return getProjectionJson()["left"];
}

json& CameraProjectionWidget::getTopBound()
{
    return getProjectionJson()["top"];
}

json& CameraProjectionWidget::getRightBound()
{
    return getProjectionJson()["right"];
}

json& CameraProjectionWidget::getBottomBound()
{
    return getProjectionJson()["bottom"];
}

json& CameraProjectionWidget::getProjectionType()
{
    return getProjectionJson()["projType"];
}


CameraComponentWidget::CameraComponentWidget(WidgetManager* wm, json& cameraComponentJson, Uint32_t sceneObjectId, QWidget *parent) :
    ComponentWidget(wm, cameraComponentJson, sceneObjectId, parent) {
    initialize();
}

CameraComponentWidget::~CameraComponentWidget()
{
}

void CameraComponentWidget::populateCubemapWidget(const GCubemapsDataMessage& message)
{
    std::vector<Uuid> cubemapIds = message.getCubemapIds();
    std::vector<GStringFixedSize<>> cubemapNames = message.getCubemapNames();

    m_cubeMaps->clear();
    m_cubeMaps->addItem("", "");
    size_t idx = 0;
    size_t count = cubemapIds.size();
    const rev::Uuid cubemapID = m_componentJson.contains("cubemap") ? m_componentJson["cubemap"] : Uuid::NullID();
    for (size_t i = 0; i < count; i++) {
        if (cubemapNames[i].length() != 0) {
            m_cubeMaps->addItem(cubemapNames[i].c_str(), cubemapIds[i].asString().c_str());
        }
        else {
            m_cubeMaps->addItem(cubemapIds[i].asString().c_str(), cubemapIds[i].asString().c_str());
        }

        // Set selected cubemap in widget to current
        if (cubemapIds[i] == cubemapID) {
            idx = i + 1; // Start indexing at 1, since there is a blank entry
        }
    }
    m_cubeMaps->setCurrentIndex((uint32_t)idx);
}


void CameraComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_cameraOptions = new CameraOptionsWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId);
    m_viewport = new CameraViewportWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId);
    m_projection = new CameraProjectionWidget(m_widgetManager, m_componentJson, m_sceneOrObjectId);
    m_renderLayers = new RenderLayerSelectWidget(m_widgetManager, m_sceneOrObjectId, ERenderLayerWidgetMode::eCamera);
    m_cubeMaps = new QComboBox();

    // Request cubemap JSON to populate widget
    requestCubemapsData();
}

void CameraComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Update the widget every time cubemap data is received
    connect(m_widgetManager, &WidgetManager::receivedCubemapsDataMessage, this, &CameraComponentWidget::populateCubemapWidget);

    // Set cubemap for the camera using uuid
    connect(m_cubeMaps, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
            Q_UNUSED(idx);
            Uuid uuid;
            GString uuidStr = m_cubeMaps->currentData().toString().toStdString();
            if (uuidStr.length() != 0) {
                uuid = Uuid(uuidStr);
            }
            else {
                uuid = Uuid::NullID();
            }

            m_modifyCubemapMessage.setSceneObjectId(m_sceneOrObjectId);
            m_modifyCubemapMessage.setCubemapId(uuid);
            m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_modifyCubemapMessage);
        }
    );
}

void CameraComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    m_mainLayout->addWidget(new QLabel("Options:"));
    m_mainLayout->addWidget(m_cameraOptions);
    m_mainLayout->addWidget(new QLabel("Viewport:"));
    m_mainLayout->addWidget(m_viewport);
    m_mainLayout->addWidget(new QLabel("Projection:"));
    m_mainLayout->addWidget(m_projection);
    m_mainLayout->addWidget(new QLabel("Cube Map:"));
    m_mainLayout->addWidget(m_cubeMaps);
    m_mainLayout->addWidget(m_renderLayers);
}

void CameraComponentWidget::requestCubemapsData()
{
    static GRequestCubemapsDataMessage s_requestMessage;
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(s_requestMessage);
}


} // rev
#include "GCameraComponentWidget.h"

#include "../../core/GCoreEngine.h"
#include "../../core/loop/GSimLoop.h"
#include "../tree/GComponentWidget.h"
#include "../../core/resource/GResourceCache.h"


#include "../../core/containers/GFlags.h"
#include "../../core/scene/GScene.h"
#include "../../core/scene/GSceneObject.h"
#include "../../core/readers/GJsonReader.h"
#include "../../core/components/GScriptComponent.h"
#include "../../core/components/GCameraComponent.h"
#include "../../core/components/GCubemapComponent.h"

#include "../../core/rendering/renderer/GRenderContext.h"
#include "../../core/rendering/renderer/GMainRenderer.h"
#include "../style/GFontIcon.h"
#include "../../core/geometry/GEulerAngles.h"

#include "../parameters/GRenderLayerWidgets.h"

#include "../GL/GGLWidget.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CameraSubWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraSubWidget::CameraSubWidget(CoreEngine* core,
    CameraComponent* camera,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_cameraComponent(camera)
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CameraOptionsWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraOptionsWidget::CameraOptionsWidget(CoreEngine* core,
    CameraComponent* camera,
    QWidget* parent) :
    CameraSubWidget(core, camera, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraOptionsWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraOptionsWidget::initializeWidgets()
{
    m_areaWidget = new QWidget();
    QVBoxLayout* areaLayout = new QVBoxLayout;
    m_checkBoxes.push_back(new QCheckBox("Frustum Culling"));
    m_checkBoxes.push_back(new QCheckBox("Occlusion Culling"));
    m_checkBoxes.push_back(new QCheckBox("Show All Render Layers"));
    m_checkBoxes.push_back(new QCheckBox("Enable Post-Processing"));

    m_checkBoxes[0]->setChecked(
        m_cameraComponent->camera().cameraOptions().testFlag(CameraOption::kFrustumCulling));
    m_checkBoxes[1]->setChecked(
        m_cameraComponent->camera().cameraOptions().testFlag(CameraOption::kOcclusionCulling));
    m_checkBoxes[2]->setChecked(
        m_cameraComponent->camera().cameraOptions().testFlag(CameraOption::kShowAllRenderLayers));
    m_checkBoxes[3]->setChecked(
        m_cameraComponent->camera().cameraOptions().testFlag(CameraOption::kEnablePostProcessing));

    m_clearColor = new ColorWidget(m_engine, m_cameraComponent->camera().clearColor());

    areaLayout->addWidget(m_checkBoxes[0]);
    areaLayout->addWidget(m_checkBoxes[1]);
    areaLayout->addWidget(m_checkBoxes[2]);
    areaLayout->addWidget(m_checkBoxes[3]);

    auto* colorLayout = LabeledLayout("Clear Color: ", m_clearColor);
    colorLayout->setSpacing(5);
    areaLayout->addLayout(colorLayout);
    m_areaWidget->setLayout(areaLayout);

    m_area = new QScrollArea();
    m_area->setWidget(m_areaWidget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraOptionsWidget::initializeConnections()
{
    // Camera options
    connect(m_checkBoxes[0], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_cameraComponent->camera().cameraOptions().setFlag(CameraOption::kFrustumCulling, checked);
    });

    connect(m_checkBoxes[1], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_cameraComponent->camera().cameraOptions().setFlag(CameraOption::kOcclusionCulling, checked);
    });

    connect(m_checkBoxes[2], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_cameraComponent->camera().cameraOptions().setFlag(CameraOption::kShowAllRenderLayers, checked);
    });

    connect(m_checkBoxes[3], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_cameraComponent->camera().cameraOptions().setFlag(CameraOption::kEnablePostProcessing, checked);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraOptionsWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_area);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CameraViewportWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraViewportWidget::CameraViewportWidget(CoreEngine* core,
    CameraComponent* camera,
    QWidget* parent) :
    CameraSubWidget(core, camera, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraViewportWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraViewportWidget::initializeWidgets()
{
    const Viewport& viewport = m_cameraComponent->camera().viewport();
    m_depth = new QLineEdit();
    m_depth->setMaximumWidth(50);
    m_depth->setValidator(new QIntValidator(-1e8, 1e8));
    m_depth->setToolTip("depth at which to render camera viewport");
    m_depth->setText(QString::number(viewport.getDepth()));

    m_xCoordinate = new QLineEdit();
    m_xCoordinate->setMaximumWidth(50);
    m_xCoordinate->setValidator(new QDoubleValidator(-1e8, 1e8, 10));
    m_xCoordinate->setToolTip("x-coordinate in normalized screen space (homogeneous coordinates, [-1, 1])");
    m_xCoordinate->setText(QString::number(viewport.m_xn));

    m_yCoordinate = new QLineEdit();
    m_yCoordinate->setMaximumWidth(50);
    m_yCoordinate->setValidator(new QDoubleValidator(-1e8, 1e8, 10));
    m_yCoordinate->setToolTip("y-coordinate in normalized screen space (homogeneous coordinates, [-1, 1])");
    m_yCoordinate->setText(QString::number(viewport.m_yn));

    m_width = new QLineEdit();
    m_width->setMaximumWidth(50);
    m_width->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_width->setToolTip("Viewport width in normalized screen space");
    m_width->setText(QString::number(viewport.m_width));

    m_height = new QLineEdit();
    m_height->setMaximumWidth(50);
    m_height->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_height->setToolTip("Viewport height in normalized screen space");
    m_height->setText(QString::number(viewport.m_height));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraViewportWidget::initializeConnections()
{
    // Depth
    connect(m_depth, &QLineEdit::editingFinished, this,
        [this]() {
        int depth = m_depth->text().toInt();
        m_cameraComponent->camera().viewport().setDepth(depth);
    });

    // x coordinate
    connect(m_xCoordinate, &QLineEdit::editingFinished, this,
        [this]() {
        double x = m_xCoordinate->text().toDouble();
        m_cameraComponent->camera().viewport().m_xn = x;
    });

    // y coordinate 
    connect(m_yCoordinate, &QLineEdit::editingFinished, this,
        [this]() {

        double y = m_yCoordinate->text().toDouble();
        m_cameraComponent->camera().viewport().m_yn = y;
    });

    // width
    connect(m_width, &QLineEdit::editingFinished, this,
        [this]() {
        const std::shared_ptr<MainRenderer>& renderer = m_engine->mainRenderer();

        if (!renderer->renderContext().isCurrent()) {
            renderer->renderContext().makeCurrent();
        }

        double width = m_width->text().toDouble();
        m_cameraComponent->camera().viewport().m_width = width;
        m_cameraComponent->camera().resizeFrame(renderer->widget()->width(), renderer->widget()->height());
        //m_cameraComponent->camera().updateFrustum(Flags::toFlags<CameraUpdateFlag>((size_t)CameraUpdateFlag::kProjectionUpdated));
        m_cameraComponent->camera().lightClusterGrid().onResize();
    });

    // height
    connect(m_height, &QLineEdit::editingFinished, this,
        [this]() {
        const std::shared_ptr<MainRenderer>& renderer = m_engine->mainRenderer();

        if (!renderer->renderContext().isCurrent()) {
            renderer->renderContext().makeCurrent();
        }

        double height = m_height->text().toDouble();
        m_cameraComponent->camera().viewport().m_height = height;
        m_cameraComponent->camera().resizeFrame(renderer->widget()->width(), renderer->widget()->height());
        //m_cameraComponent->camera().updateFrustum(Flags::toFlags<CameraUpdateFlag>((size_t)CameraUpdateFlag::kProjectionUpdated));
        m_cameraComponent->camera().lightClusterGrid().onResize();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraViewportWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    //m_mainLayout->setSpacing(0);

    float res = Viewport::ScreenDPI();
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


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CameraProjectionWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraProjectionWidget::CameraProjectionWidget(CoreEngine* core,
    CameraComponent* camera,
    QWidget* parent) :
    CameraSubWidget(core, camera, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraProjectionWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraProjectionWidget::initializeWidgets()
{
    RenderProjection& projection = m_cameraComponent->camera().renderProjection();
    m_projectionType = new QComboBox();
    m_projectionType->addItem(SAIcon("image"), "Perspective");
    m_projectionType->addItem(SAIcon("border-all"), "Orthographic");

    // Perspective
    m_fov = new QLineEdit();
    m_fov->setMaximumWidth(50);
    m_fov->setValidator(new QDoubleValidator(0, 179.0, 4));
    m_fov->setToolTip("Horizontal field of view (in degrees)");
    m_fov->setText(QString::number(projection.fovX()));

    m_near = new QLineEdit();
    m_near->setMaximumWidth(50);
    m_near->setValidator(new QDoubleValidator(-1e20, 1e20, 15));
    m_near->setToolTip("Near clip plane");
    m_near->setText(QString::number(projection.nearClipPlane()));

    m_far = new QLineEdit();
    m_far->setMaximumWidth(50);
    m_far->setValidator(new QDoubleValidator(-1e20, 1e20, 15));
    m_far->setToolTip("Far clip plane");
    m_far->setText(QString::number(projection.farClipPlane()));

    // Orthographic
    m_left = new QLineEdit();
    m_left->setMaximumWidth(50);
    m_left->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_left->setToolTip("Left bound");
    m_left->setText(QString::number(projection.leftBound()));

    m_top = new QLineEdit();
    m_top->setMaximumWidth(50);
    m_top->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_top->setToolTip("Top bound");
    m_top->setText(QString::number(projection.topBound()));

    m_right = new QLineEdit();
    m_right->setMaximumWidth(50);
    m_right->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_right->setToolTip("Right bound");
    m_right->setText(QString::number(projection.rightBound()));

    m_bottom = new QLineEdit();
    m_bottom->setMaximumWidth(50);
    m_bottom->setValidator(new QDoubleValidator(-1e8, 1e8, 8));
    m_bottom->setToolTip("Bottom bound");
    m_bottom->setText(QString::number(projection.bottomBound()));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraProjectionWidget::initializeConnections()
{
    // Projection type
    connect(m_projectionType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        RenderProjection::ProjectionType type = RenderProjection::ProjectionType(index);
        m_cameraComponent->camera().renderProjection().setProjectionType(type);
        m_stackedWidget->setCurrentIndex(index);
    });

    // FOV
    connect(m_fov, &QLineEdit::editingFinished, this,
        [this]() {
        double fov = m_fov->text().toDouble();
        m_cameraComponent->camera().renderProjection().setFOV(fov);
    });

    // Near plane
    connect(m_near, &QLineEdit::editingFinished, this,
        [this]() {
        double nearPlane = m_near->text().toDouble();
        m_cameraComponent->camera().renderProjection().setNearClipPlane(nearPlane);
    });

    // Far plane
    connect(m_far, &QLineEdit::editingFinished, this,
        [this]() {
        double farPlane = m_far->text().toDouble();
        m_cameraComponent->camera().renderProjection().setFarClipPlane(farPlane);
    });

    // Left
    connect(m_left, &QLineEdit::editingFinished, this,
        [this]() {
        double left = m_left->text().toDouble();
        m_cameraComponent->camera().renderProjection().setLeftBound(left);
    });

    // Top
    connect(m_top, &QLineEdit::editingFinished, this,
        [this]() {
        double top = m_top->text().toDouble();
        m_cameraComponent->camera().renderProjection().setTopBound(top);
    });

    // Right
    connect(m_right, &QLineEdit::editingFinished, this,
        [this]() {
        double right = m_right->text().toDouble();
        m_cameraComponent->camera().renderProjection().setRightBound(right);
    });

    // Bottom
    connect(m_bottom, &QLineEdit::editingFinished, this,
        [this]() {
        double bottom = m_bottom->text().toDouble();
        m_cameraComponent->camera().renderProjection().setBottomBound(bottom);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
    m_stackedWidget->setCurrentIndex(m_cameraComponent->camera().renderProjection().getProjectionType());
    m_mainLayout->addWidget(m_stackedWidget);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CameraComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponentWidget::CameraComponentWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponent* CameraComponentWidget::cameraComponent() const {
    return static_cast<CameraComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CameraComponentWidget::~CameraComponentWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_cameraOptions = new CameraOptionsWidget(m_engine, cameraComponent());
    m_viewport = new CameraViewportWidget(m_engine, cameraComponent());
    m_projection = new CameraProjectionWidget(m_engine, cameraComponent());
    m_renderLayers = new RenderLayerSelectWidget(m_engine, cameraComponent()->camera().renderLayerIds());

    m_cubeMaps = new QComboBox();
    m_cubeMaps->addItem("", "");
    size_t idx = 0;
    std::vector<CubeMapComponent*>& cubemaps = m_component->sceneObject()->scene()->cubeMaps();
    size_t count = cubemaps.size();
    const rev::Uuid& cubemapID = cameraComponent()->cubeMapID();
    for (size_t i = 0; i < count; i++) {
        CubeMapComponent* cm = cubemaps[i];
        if (cm->getName().length() != 0) {
            m_cubeMaps->addItem(cm->getName().c_str(), cm->getUuid().asQString());
        }
        else {
            m_cubeMaps->addItem(cm->getUuid().asQString(), cm->getUuid().asQString());
        }

        // Set selected cubemap in widget to current
        if (cm->getUuid() == cubemapID) {
            idx = i + 1; // Start indexing at 1, since there is a blank entry
        }
    }
    m_cubeMaps->setCurrentIndex(idx);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();

    // Set cubemap for the camera using uuid
    connect(m_cubeMaps, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        Q_UNUSED(idx);
        QString uuid = m_cubeMaps->currentData().toString();
        if (uuid.length() != 0)
            cameraComponent()->setCubeMapID(Uuid(uuid));
        else
            cameraComponent()->setCubeMapID(Uuid(false));
    }
    );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // rev
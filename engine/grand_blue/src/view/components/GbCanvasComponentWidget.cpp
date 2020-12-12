#include "GbCanvasComponentWidget.h"
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
#include "../../core/components/GbCanvasComponent.h"

#include "../tree/GbCanvasGlyphWidget.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CanvasSubWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasSubWidget::CanvasSubWidget(CoreEngine* core,
    CanvasComponent* canvas,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_canvasComponent(canvas)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CanvasViewportWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasViewportWidget::CanvasViewportWidget(CoreEngine* core,
    CanvasComponent* canvas,
    QWidget* parent) :
    CanvasSubWidget(core, canvas, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasViewportWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasViewportWidget::initializeWidgets()
{
    const Viewport& viewport = m_canvasComponent->viewport();
    m_depth = new QLineEdit();
    m_depth->setMaximumWidth(50);
    m_depth->setValidator(new QIntValidator(-1e8, 1e8));
    m_depth->setToolTip("depth at which to render camera viewport");
    m_depth->setText(QString::number(viewport.getDepth()));

    m_xCoordinate = new QLineEdit();
    m_xCoordinate->setMaximumWidth(50);
    m_xCoordinate->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_xCoordinate->setToolTip("x-coordinate in normalized screen space");
    m_xCoordinate->setText(QString::number(viewport.m_xn));

    m_yCoordinate = new QLineEdit();
    m_yCoordinate->setMaximumWidth(50);
    m_yCoordinate->setValidator(new QDoubleValidator(0, 1.0, 10));
    m_yCoordinate->setToolTip("y-coordinate in normalized screen space");
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
void CanvasViewportWidget::initializeConnections()
{
    // Depth
    connect(m_depth, &QLineEdit::editingFinished, this,
        [this]() {
        // FIXME: See if depth needs to be normalized to clip space anywhere
        int depth = m_depth->text().toInt();
        m_canvasComponent->viewport().setDepth(depth);
    });

    // x coordinate
    connect(m_xCoordinate, &QLineEdit::editingFinished, this,
        [this]() {
        double x = m_xCoordinate->text().toDouble();
        m_canvasComponent->viewport().m_xn = x;
    });

    // y coordinate 
    connect(m_yCoordinate, &QLineEdit::editingFinished, this,
        [this]() {
        double y = m_yCoordinate->text().toDouble();
        m_canvasComponent->viewport().m_yn = y;
    });

    // width
    connect(m_width, &QLineEdit::editingFinished, this,
        [this]() {
        double width = m_width->text().toDouble();
        m_canvasComponent->viewport().m_width = width;
    });

    // height
    connect(m_height, &QLineEdit::editingFinished, this,
        [this]() {
        double height = m_height->text().toDouble();
        m_canvasComponent->viewport().m_height = height;
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasViewportWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    //m_mainLayout->setSpacing(0);

    float res = Renderable::screenDPI();
    QBoxLayout* coordinateLayout = new QHBoxLayout;
    AddLabel(SAIcon("arrows-alt"), coordinateLayout, { res * 0.25f, res * 0.25f });
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
// CanvasProjectionWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasProjectionWidget::CanvasProjectionWidget(CoreEngine* core,
    CanvasComponent* c,
    QWidget* parent) :
    CanvasSubWidget(core, c, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasProjectionWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasProjectionWidget::initializeWidgets()
{
    RenderProjection& projection = m_canvasComponent->renderProjection();
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
void CanvasProjectionWidget::initializeConnections()
{
    // Projection type
    connect(m_projectionType, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int index) {
        RenderProjection::ProjectionType type = RenderProjection::ProjectionType(index);
        m_canvasComponent->renderProjection().setProjectionType(type);
        m_stackedWidget->setCurrentIndex(index);
    });

    // FOV
    connect(m_fov, &QLineEdit::editingFinished, this,
        [this]() {
        double fov = m_fov->text().toDouble();
        m_canvasComponent->renderProjection().setFOV(fov);
    });

    // Near plane
    connect(m_near, &QLineEdit::editingFinished, this,
        [this]() {
        double nearPlane = m_near->text().toDouble();
        m_canvasComponent->renderProjection().setNearClipPlane(nearPlane);
    });

    // Far plane
    connect(m_far, &QLineEdit::editingFinished, this,
        [this]() {
        double farPlane = m_far->text().toDouble();
        m_canvasComponent->renderProjection().setFarClipPlane(farPlane);
    });

    // Left
    connect(m_left, &QLineEdit::editingFinished, this,
        [this]() {
        double left = m_left->text().toDouble();
        m_canvasComponent->renderProjection().setLeftBound(left);
    });

    // Top
    connect(m_top, &QLineEdit::editingFinished, this,
        [this]() {
        double top = m_top->text().toDouble();
        m_canvasComponent->renderProjection().setTopBound(top);
    });

    // Right
    connect(m_right, &QLineEdit::editingFinished, this,
        [this]() {
        double right = m_right->text().toDouble();
        m_canvasComponent->renderProjection().setRightBound(right);
    });

    // Bottom
    connect(m_bottom, &QLineEdit::editingFinished, this,
        [this]() {
        double bottom = m_bottom->text().toDouble();
        m_canvasComponent->renderProjection().setBottomBound(bottom);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasProjectionWidget::layoutWidgets()
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
    m_stackedWidget->setCurrentIndex(m_canvasComponent->renderProjection().getProjectionType());
    m_mainLayout->addWidget(m_stackedWidget);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CanvasComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponentWidget::CanvasComponentWidget(CoreEngine* core,
    Component* component, 
    QWidget *parent) :
    ComponentWidget(core, component, parent),
    m_canvasComponent(static_cast<CanvasComponent*>(m_component))
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponent* CanvasComponentWidget::canvasComponent() const {
    return m_canvasComponent;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasComponentWidget::~CanvasComponentWidget()
{
    delete m_renderable;
    delete m_viewport;
    delete m_projection;
    delete m_glyphs;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponentWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();

    m_glyphs = new CanvasGlyphWidget(m_engine, m_canvasComponent);
    m_renderable = new RenderableWidget<CanvasComponent>(m_engine, m_canvasComponent);
    m_viewport = new CanvasViewportWidget(m_engine, m_canvasComponent);
    m_projection = new CanvasProjectionWidget(m_engine, m_canvasComponent);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasComponentWidget::layoutWidgets()
{
    ComponentWidget::layoutWidgets();

    m_mainLayout->addWidget(new QLabel("Viewport:"));
    m_mainLayout->addWidget(m_viewport);
    m_mainLayout->addWidget(new QLabel("Projection:"));
    m_mainLayout->addWidget(m_projection);
    m_mainLayout->addWidget(m_glyphs);

    m_mainLayout->addWidget(m_renderable);
}




///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb
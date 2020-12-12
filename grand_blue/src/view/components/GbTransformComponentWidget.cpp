#include "GbTransformComponentWidget.h"

#include "../../core/GbCoreEngine.h"
#include "../../core/loop/GbSimLoop.h"
#include "../tree/GbComponentWidget.h"
#include "../../core/resource/GbResourceCache.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/readers/GbJsonReader.h"

#include "../../core/components/GbTransformComponent.h"
#include "../style/GbFontIcon.h"
#include "../../core/geometry/GbEulerAngles.h"
#include "../parameters/GbVectorWidget.h"
#include "GbTransformComponentWidget.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// TranslationWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
TranslationWidget::TranslationWidget(CoreEngine * core,
    Transform * transform,
    QWidget * parent) :
    VectorWidget<real_g, 3>(core,
        transform->translation().position(),
        parent),
    m_transform(transform)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TranslationWidget::update()
{
    if (!isVisible()) return;

    //pauseSimulation();

    VectorWidget<real_g, 3>::update();

    //resumeSimulation();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TranslationWidget::setTransform(Transform & transform)
{
    // TODO: Finish implementing
    reinitialize(transform.translation().position());
    m_transform = &transform;
    //update();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TranslationWidget::updateVector()
{
    VectorWidget<real_g, 3>::updateVector();
    m_transform->translation().setPosition(m_vector, true);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RotationWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
RotationWidget::RotationWidget(CoreEngine * core, Transform * transform, QWidget * parent) :
    ParameterWidget(core, parent),
    m_transform(transform)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::update()
{
    if (!isVisible()) return;
    updateWidgetRotation();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::setTransform(Transform & transform)
{
    Q_UNUSED(transform);
    // TODO: Implement
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::initializeWidgets()
{
    m_isInertial = new QCheckBox("Inertial");
    m_isInertial->setChecked(false);
    m_isInertial->setToolTip("Whether or not rotations are with respect to inertial space");

    m_axisBox1 = new QComboBox();
    m_axisBox1->setMaximumWidth(40);
    m_axisBox1->addItem("x");
    m_axisBox1->addItem("y");
    m_axisBox1->addItem("z");
    m_axisBox1->setToolTip("First axis of rotation");

    m_axisBox2 = new QComboBox();
    m_axisBox2->setMaximumWidth(40);
    m_axisBox2->addItem("x");
    m_axisBox2->addItem("y");
    m_axisBox2->addItem("z");
    m_axisBox2->setCurrentIndex(1);
    m_axisBox2->setToolTip("Second axis of rotation");

    m_axisBox3 = new QComboBox();
    m_axisBox3->setMaximumWidth(40);
    m_axisBox3->addItem("x");
    m_axisBox3->addItem("y");
    m_axisBox3->addItem("z");
    m_axisBox3->setCurrentIndex(2);
    m_axisBox3->setToolTip("Third axis of rotation");

    EulerAngles rot = EulerAngles(m_transform->rotation().getQuaternion(), EulerAngles::kXYZ);
    m_lastDegreeAngles = rot.angles()* Constants::RAD_TO_DEG;

    m_rotEdit1 = new QLineEdit(QString::number(m_lastDegreeAngles[0]));
    m_rotEdit1->setMaximumWidth(50);
    m_rotEdit1->setValidator(new QDoubleValidator(-180.0, 180.0, 10));
    m_rotEdit1->setToolTip("In degrees");

    m_rotEdit2 = new QLineEdit(QString::number(m_lastDegreeAngles[1]));
    m_rotEdit2->setMaximumWidth(50);
    m_rotEdit2->setValidator(new QDoubleValidator(-180.0, 180.0, 10));
    m_rotEdit2->setToolTip("In degrees");

    m_rotEdit3 = new QLineEdit(QString::number(m_lastDegreeAngles[2]));
    m_rotEdit3->setMaximumWidth(50);
    m_rotEdit3->setValidator(new QDoubleValidator(-180.0, 180.0, 10));
    m_rotEdit3->setToolTip("In degrees");

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::initializeConnections()
{
    // Rotation
    connect(m_isInertial, &QCheckBox::clicked,
        [this](bool checked) {
        Q_UNUSED(checked);
        updateRotation();
    }
    );
    connect(m_axisBox1, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        if (m_axisBox2->currentIndex() == index) {
            m_axisBox2->setCurrentIndex(ensureDifferentAxis(index));
        }
        updateRotation();
    }
    );
    connect(m_axisBox2, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        if (m_axisBox1->currentIndex() == index) {
            m_axisBox1->setCurrentIndex(ensureDifferentAxis(index));
        }
        if (m_axisBox3->currentIndex() == index) {
            m_axisBox3->setCurrentIndex(ensureDifferentAxis(index));
        }
        updateRotation();
    }
    );
    connect(m_axisBox3, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        if (m_axisBox2->currentIndex() == index) {
            m_axisBox2->setCurrentIndex(ensureDifferentAxis(index));
        }
        updateRotation();
    }
    );
    connect(m_rotEdit1, &QLineEdit::editingFinished,
        [=]() { updateRotation(); }
    );
    connect(m_rotEdit2, &QLineEdit::editingFinished,
        [=]() { updateRotation(); }
    );
    connect(m_rotEdit3, &QLineEdit::editingFinished,
        [=]() { updateRotation(); }
    );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(0);

    QHBoxLayout* rotationFirstLayout = new QHBoxLayout;
    QHBoxLayout* rotationSecondLayout = new QHBoxLayout;

    rotationFirstLayout->addWidget(m_isInertial);

    rotationSecondLayout->addWidget(m_axisBox1);
    rotationSecondLayout->addWidget(m_rotEdit1);
    rotationSecondLayout->addSpacing(15);
    rotationSecondLayout->addWidget(m_axisBox2);
    rotationSecondLayout->addWidget(m_rotEdit2);
    rotationSecondLayout->addSpacing(15);
    rotationSecondLayout->addWidget(m_axisBox3);
    rotationSecondLayout->addWidget(m_rotEdit3);

    m_mainLayout->addLayout(rotationFirstLayout);
    m_mainLayout->addLayout(rotationSecondLayout);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::updateRotation()
{
    //pauseSimulation();

    EulerAngles::EulerType type = EulerAngles::EulerType(getRotationType());
    if (type == EulerAngles::kInvalid) {
        throw("Invalid type");
    }
    RotationSpace space = getRotationSpace();
    m_lastDegreeAngles = Vector3(m_rotEdit1->text().toDouble(), m_rotEdit2->text().toDouble(), m_rotEdit3->text().toDouble());
    EulerAngles eulerAngles = EulerAngles(m_lastDegreeAngles[0] * Constants::DEG_TO_RAD,
        m_lastDegreeAngles[1] * Constants::DEG_TO_RAD,
        m_lastDegreeAngles[2] * Constants::DEG_TO_RAD,
        type,
        space);
    m_transform->rotation().setRotation(eulerAngles.toQuaternion(), true);

    //resumeSimulation();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void RotationWidget::updateWidgetRotation()
{
    EulerAngles::EulerType type = EulerAngles::EulerType(getRotationType());
    if (type == EulerAngles::kInvalid) {
        throw("Invalid type");
    }

    RotationSpace space = getRotationSpace();
    RotationComponent& rot = m_transform->rotation();
    EulerAngles eulerAngles = EulerAngles(
        rot.getQuaternion(),
        type,
        space);

    // Check if euler angles have been shifted by 180 degrees, and correct back
    Vector3 degAngles = eulerAngles.angles() * Constants::RAD_TO_DEG;
    Vector3 upShiftDiff = degAngles + 180 - m_lastDegreeAngles;
    for (size_t i = 0; i < 3; i++) {
        upShiftDiff[i] = fmod(upShiftDiff[i], 360.0f);
    }
    Vector3 downShiftDiff = degAngles - 180 - m_lastDegreeAngles;
    for (size_t i = 0; i < 3; i++) {
        downShiftDiff[i] = fmod(downShiftDiff[i], 360.0f);
    }

    if (upShiftDiff.lengthSquared() < 1e-6) {
        degAngles = m_lastDegreeAngles;
    }
    else if (downShiftDiff.lengthSquared() < 1e-6) {
        degAngles = m_lastDegreeAngles;
    }
    if (!m_rotEdit1->hasFocus()) {
        m_rotEdit1->setText(QString::number(degAngles[0]));
    }
    if (!m_rotEdit2->hasFocus()) {
        m_rotEdit2->setText(QString::number(degAngles[1]));
    }
    if (!m_rotEdit3->hasFocus()) {
        m_rotEdit3->setText(QString::number(degAngles[2]));
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int RotationWidget::getRotationType() const
{
    Vector<Axis, 3> axes = { Axis(m_axisBox1->currentIndex()),
        Axis(m_axisBox2->currentIndex()),
        Axis(m_axisBox3->currentIndex()) };

    return (int)EulerAngles::axesToType(axes, getRotationSpace());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
RotationSpace RotationWidget::getRotationSpace() const
{
    if (m_isInertial->isChecked()) {
        return RotationSpace::kInertial;
    }
    else {
        return RotationSpace::kBody;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int RotationWidget::ensureDifferentAxis(int index)
{
    if (index == 0) {
        return 1;
    }
    else if (index == 1) {
        return 0;
    }
    else {
        return 0;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// ScaleWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
ScaleWidget::ScaleWidget(CoreEngine * core, 
    Transform* transform,
    QWidget * parent) :
    VectorWidget<real_g, 3>(core,
        transform->scale().scale(),
        parent),
    m_transform(transform)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ScaleWidget::update()
{
    if (!isVisible()) return;

    //pauseSimulation();

    VectorWidget<real_g, 3>::update();

    //resumeSimulation();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScaleWidget::setTransform(Transform & transform)
{
    Q_UNUSED(transform);
    // TODO: Implement
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void ScaleWidget::updateVector()
{
    VectorWidget<real_g, 3>::updateVector();
    m_transform->scale().setScale(m_vector, true);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// TransformWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
TransformWidget::TransformWidget(CoreEngine* core, Transform* transform, QWidget *parent) :
    ParameterWidget(core, parent),
    m_transform(transform)
{
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TransformWidget::~TransformWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformWidget::setTransform(Transform & transform)
{
    // Set transform
    m_transform = &transform;

    // Delete and replace main layout and all contained widgets
    delete m_mainLayout;
    initialize();
    //m_translationWidget->setTransform(transform);
    //m_scaleWidget->setTransform(transform);
    //m_rotationWidget->setTransform(transform);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Transform * TransformWidget::transform() const
{
    return m_transform;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformWidget::initializeWidgets()
{
    // Main tab widget
    m_tabWidget = new QTabWidget();

    // Inheritance type
    QComboBox* inheritanceWidget = new QComboBox;
    inheritanceWidget->addItem(SAIcon("dot-circle"), tr("All"));
    inheritanceWidget->addItem(SAIcon("cubes"), tr("Translation"));
    inheritanceWidget->addItem(SAIcon("object-group"), tr("PreserveOrientation"));
    m_inheritanceTypeWidget = inheritanceWidget;

    // Translation
    m_translationWidget = new TranslationWidget(m_engine, m_transform);
    m_tabWidget->addTab(m_translationWidget, SAIcon("arrows-alt"), "Translation");

    // Rotation
    m_rotationWidget = new RotationWidget(m_engine, m_transform);
    m_tabWidget->addTab(m_rotationWidget, SAIcon("redo"), "Rotation");

    // Scaling
    m_scaleWidget = new ScaleWidget(m_engine, m_transform);
    m_tabWidget->addTab(m_scaleWidget, SAIcon("expand-arrows-alt"), QStringLiteral("Scale"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformWidget::initializeConnections()
{
    connect((QComboBox*)m_inheritanceTypeWidget, 
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {

        Transform::InheritanceType type = Transform::InheritanceType(index);
        if (m_transform->inheritanceType() != type) {
            //pauseSimulation();

            m_transform->setInheritanceType(type);

            //resumeSimulation();
        }
    }
    );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformWidget::layoutWidgets()
{
    // Create base widget layout
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    QBoxLayout* itLayout = new QHBoxLayout;
    itLayout->addWidget(new QLabel("Inheritance Type:"));
    itLayout->addWidget(m_inheritanceTypeWidget);

    m_mainLayout->addLayout(itLayout);
    m_mainLayout->addWidget(m_tabWidget);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// TransformComponentWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponentWidget::TransformComponentWidget(CoreEngine* core, Component* component, QWidget *parent) :
    ComponentWidget(core, component, parent) {
    initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponentWidget::~TransformComponentWidget()
{
    delete m_transformWidget;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TransformComponent* TransformComponentWidget::transformComponent() const {
    return static_cast<TransformComponent*>(m_component);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();
    m_transformWidget = new TransformWidget(m_engine, transformComponent());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TransformComponentWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Add transform widget to main layout
    m_mainLayout->addWidget(m_transformWidget);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces        
} // View
} // Gb
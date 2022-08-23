#include "geppetto/qt/widgets/components/GTransformComponentWidget.h"

#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/constants/GConstants.h"
#include "fortress/containers/math/GEulerAngles.h"
#include "fortress/containers/math/GQuaternion.h"
#include "fortress/json/GJson.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/components/GComponentWidget.h"
#include "geppetto/qt/widgets/types/GVectorWidget.h"

namespace rev {

TranslationWidget::TranslationWidget(WidgetManager * wm, json& transformJson, QWidget * parent) :
    VectorWidget<Real_t, 3>(wm, m_vector, parent),
    m_transformJson(transformJson),
    m_vector(transformJson["translation"]["translation"]["position"])
{
    setIsChild(true);
}

void TranslationWidget::update()
{
    if (!isVisible()) return;

    // Make sure internal vector is up-to-date with JSON, and render in widget
    m_vector = m_transformJson["translation"]["translation"]["position"];
    VectorWidget<Real_t, 3>::update();
}


void TranslationWidget::updateVectorFromWidget()
{
    // Update transform JSON as well to reflect vector contents
    VectorWidget<Real_t, 3>::updateVectorFromWidget();
    m_transformJson["translation"]["translation"]["position"] = m_vector;
    
    // Send request to main application to update transform
    QWidget* qParent = parentWidget();
    TransformWidget* parent = dynamic_cast<TransformWidget*>(qParent);

    // If added to tab widget, parent will be the tab widget
    if (!parent) {
        parent = dynamic_cast<TransformWidget*>(qParent->parentWidget()->parentWidget());
    }

    if (parent) {
        parent->updateTransformFromJson();
    }
}




RotationWidget::RotationWidget(WidgetManager* wm, json& transformJson, QWidget * parent) :
    ParameterWidget(wm, parent),
    m_transformJson(transformJson)
{
    setIsChild(true);
    initialize();
}

void RotationWidget::update()
{
    if (!isVisible()) return;

    updateWidgetRotation();
}


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

    EulerAngles rot = EulerAngles(m_transformJson["rotation"]["quaternion"].get<Quaternion>(), EulerAngles::kXYZ);
    m_lastDegreeAngles = rot.angles()* Constants::RadToDeg;

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

void RotationWidget::initializeConnections()
{
    // Rotation
    connect(m_isInertial, &QCheckBox::clicked,
        [this](bool checked) {
        Q_UNUSED(checked);
        updateRotationFromWidget();
    }
    );
    connect(m_axisBox1, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        if (m_axisBox2->currentIndex() == index) {
            m_axisBox2->setCurrentIndex(ensureDifferentAxis(index));
        }
        updateRotationFromWidget();
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
        updateRotationFromWidget();
    }
    );
    connect(m_axisBox3, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
        if (m_axisBox2->currentIndex() == index) {
            m_axisBox2->setCurrentIndex(ensureDifferentAxis(index));
        }
        updateRotationFromWidget();
    }
    );
    connect(m_rotEdit1, &QLineEdit::editingFinished,
        [=]() { updateRotationFromWidget(); }
    );
    connect(m_rotEdit2, &QLineEdit::editingFinished,
        [=]() { updateRotationFromWidget(); }
    );
    connect(m_rotEdit3, &QLineEdit::editingFinished,
        [=]() { updateRotationFromWidget(); }
    );
}

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

void RotationWidget::updateRotationFromWidget()
{
    // Obtain the euler angles representing the rotation
    EulerAngles::EulerType type = EulerAngles::EulerType(getRotationType());
#ifdef DEBUG_MODE
    assert(type != EulerAngles::kInvalid && "Invalid type");
#endif
    RotationSpace space = getRotationSpace();
    m_lastDegreeAngles = Vector3(m_rotEdit1->text().toDouble(), m_rotEdit2->text().toDouble(), m_rotEdit3->text().toDouble());
    EulerAngles eulerAngles = EulerAngles(m_lastDegreeAngles[0] * Constants::DegToRad,
        m_lastDegreeAngles[1] * Constants::DegToRad,
        m_lastDegreeAngles[2] * Constants::DegToRad,
        type,
        space);
#ifdef DEBUG_MODE
    assert(m_transformJson["rotation"].contains("quaternion"));
#endif

    // Modify the transform JSON
    m_transformJson["rotation"]["quaternion"] = eulerAngles.toQuaternion();

    // Send request to main application to update transform
    QWidget* qParent = parentWidget();
    TransformWidget* parent = dynamic_cast<TransformWidget*>(qParent);

    // If added to tab widget, parent will be the tab widget
    if (!parent) {
        parent = dynamic_cast<TransformWidget*>(qParent->parentWidget()->parentWidget());
    }

    if (parent) {
        parent->updateTransformFromJson();
    }
}

void RotationWidget::updateWidgetRotation()
{
    EulerAngles::EulerType type = EulerAngles::EulerType(getRotationType());
#ifdef DEBUG_MODE
    assert(type != EulerAngles::kInvalid && "Invalid type");
#endif

    RotationSpace space = getRotationSpace();
    Quaternion rotation = m_transformJson["rotation"]["quaternion"];
    EulerAngles eulerAngles = EulerAngles(
        rotation,
        type,
        space);

    // Check if euler angles have been shifted by 180 degrees, and correct back
    Vector3 degAngles = eulerAngles.angles() * Constants::RadToDeg;
    Vector3 upShiftDiff = (degAngles + 180) - m_lastDegreeAngles;
    for (size_t i = 0; i < 3; i++) {
        upShiftDiff[i] = fmod(upShiftDiff[i], 360.0f);
    }
    Vector3 downShiftDiff = (degAngles - 180) - m_lastDegreeAngles;
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

EulerAngles::EulerType RotationWidget::getRotationType() const
{
    EulerAngles::Axes axes = { Axis(m_axisBox1->currentIndex()),
        Axis(m_axisBox2->currentIndex()),
        Axis(m_axisBox3->currentIndex()) };

    return EulerAngles::axesToType(axes, getRotationSpace());
}

RotationSpace RotationWidget::getRotationSpace() const
{
    if (m_isInertial->isChecked()) {
        return RotationSpace::kInertial;
    }
    else {
        return RotationSpace::kBody;
    }
}

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




ScaleWidget::ScaleWidget(WidgetManager* wm, 
    json& transformJson,
    QWidget * parent) :
    VectorWidget<Real_t, 3>(wm, m_vector, parent),
    m_transformJson(transformJson),
    m_vector(transformJson["scale"]["scaling"])
{
    setIsChild(true);
}


void ScaleWidget::update()
{
    if (!isVisible()) return;

    // Make sure internal vector is up-to-date with JSON, and render in widget
    m_vector = m_transformJson["scale"]["scaling"];
    VectorWidget<Real_t, 3>::update();

}

void ScaleWidget::updateVectorFromWidget()
{
    // Update transform JSON as well to reflect vector contents
    VectorWidget<Real_t, 3>::updateVectorFromWidget();
    m_transformJson["scale"]["scaling"] = m_vector;

    // Send request to main application to update transform
    QWidget* qParent = parentWidget();
    TransformWidget* parent = dynamic_cast<TransformWidget*>(qParent);

    // If added to tab widget, parent will be the tab widget
    if (!parent) {
        parent = dynamic_cast<TransformWidget*>(qParent->parentWidget()->parentWidget());
    }

    if (parent) {
        parent->updateTransformFromJson();
    }
}



TransformWidget::TransformWidget(WidgetManager* wm, json& transformJson, json metadata, QWidget *parent) :
    ParameterWidget(wm, parent),
    m_transformJson(transformJson)
{
    if (!metadata.empty()) {
        m_requestTransformMessage.setMetadataJsonBytes(GJson::ToBytes(metadata));
        m_transformUpdateMessage.setMetadataJsonBytes(GJson::ToBytes(metadata));

        /// @todo Don't reference scene object from this widget, move to TransformComponentWidget
        if (metadata.contains("sceneObjectId")) {
            Uint32_t sceneObjectId = metadata["sceneObjectId"].get<Uint32_t>();
            m_requestTransformMessage.setSceneOrObjectId(sceneObjectId);
            m_transformUpdateMessage.setSceneOrObjectId(sceneObjectId);
        }

    }
    m_requestUpdateTimer.restart();
    initialize();
}


TransformWidget::~TransformWidget()
{
    //m_widgetManager->messageGateway()->clearMessageFromSendQueue(m_requestTransformMessage.header().m_uniqueId);
}

void TransformWidget::update()
{
    // Since these are marked as child parameter widgets, they are updated manually here
    m_translationWidget->update();
    m_rotationWidget->update();
    m_scaleWidget->update();

    /// Request updated transform from main applicaiton
    if (m_requestUpdateTimer.isExpired()) {
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_requestTransformMessage);
        m_requestUpdateTimer.restart();
    }
}

void TransformWidget::updateTransformFromJson()
{
    m_transformUpdateMessage.setTransformJsonBytes(GJson::ToBytes(m_transformJson));

    /// Send updated transform JSON to the main application
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_transformUpdateMessage);
}

void TransformWidget::update(const GTransformMessage& message)
{
    // Check that metadata is correct, and update if it matches
    if (message.getMetadataJsonBytes() == m_requestTransformMessage.getMetadataJsonBytes()) {
        m_transformJson = GJson::FromBytes(message.getTransformJsonBytes());
    }

    // Update child widgets
    update();
}


void TransformWidget::initializeWidgets()
{
    // Main tab widget
    m_tabWidget = new QTabWidget(this);

    // Inheritance type
    QComboBox* inheritanceWidget = new QComboBox;
    inheritanceWidget->addItem(SAIcon("dot-circle"), tr("All"));
    inheritanceWidget->addItem(SAIcon("cubes"), tr("Translation"));
    inheritanceWidget->addItem(SAIcon("object-group"), tr("PreserveOrientation"));
    m_inheritanceTypeWidget = inheritanceWidget;

    // Translation
    m_translationWidget = new TranslationWidget(m_widgetManager, m_transformJson, this);
    m_tabWidget->addTab(m_translationWidget, SAIcon("arrows-alt"), "Translation");

    // Rotation
    m_rotationWidget = new RotationWidget(m_widgetManager, m_transformJson, this);
    m_tabWidget->addTab(m_rotationWidget, SAIcon("redo"), "Rotation");

    // Scaling
    m_scaleWidget = new ScaleWidget(m_widgetManager, m_transformJson, this);
    m_tabWidget->addTab(m_scaleWidget, SAIcon("expand-arrows-alt"), QStringLiteral("Scale"));
}

void TransformWidget::initializeConnections()
{
    // Connection to update inheritance type of the transform
    connect((QComboBox*)m_inheritanceTypeWidget,
        QOverload<Int32_t>::of(&QComboBox::currentIndexChanged),
        [=](Int32_t index) {

#ifdef DEBUG_MODE
            assert(m_transformJson["inheritanceType"].is_number_integer());
#endif
            if (Int32_t(m_transformJson["inheritanceType"]) != index) {
                m_transformJson["inheritanceType"] = index;
                updateTransformFromJson();
            }
        }
    );

    // Connection to update when transform is received
    connect(m_widgetManager, &WidgetManager::receivedTransformMessage, this,
        Signal<const GTransformMessage&>::Cast(&TransformWidget::update));
}

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





TransformComponentWidget::TransformComponentWidget(WidgetManager* wm, const json& transformJson, Int32_t sceneObjectId, QWidget *parent) :
    ComponentWidget(wm, transformJson, sceneObjectId, parent) {
    initialize();
}

TransformComponentWidget::~TransformComponentWidget()
{
    delete m_transformWidget;
}

void TransformComponentWidget::initializeWidgets()
{
    ComponentWidget::initializeWidgets();
    m_transformWidget = new TransformWidget(m_widgetManager, m_componentJson, { {"sceneObjectId", m_sceneOrObjectId} });
}

void TransformComponentWidget::initializeConnections()
{
    ComponentWidget::initializeConnections();
}

void TransformComponentWidget::layoutWidgets()
{
    // Create base widget layout
    ComponentWidget::layoutWidgets();

    // Add transform widget to main layout
    m_mainLayout->addWidget(m_transformWidget);
}


} // rev
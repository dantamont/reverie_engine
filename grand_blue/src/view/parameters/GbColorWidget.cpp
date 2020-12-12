#include "GbColorWidget.h"
#include "../style/GbFontIcon.h"
#include "../../core/mixins/GbRenderable.h"
#include "../../core/resource/GbImage.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ColorWidget
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ColorWidget::ColorWidget(CoreEngine* core, Color& color, QWidget* parent):
    ParameterWidget(core, parent),
    m_color(&color)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ColorWidget::ColorWidget(CoreEngine* core, Vector4& color, QWidget* parent) :
    ParameterWidget(core, parent),
    m_colorVector(&color)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ColorWidget::~ColorWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::update()
{
    if (!m_colorDialog->hasFocus()) {
        // Will crash on widget delete if modifying a deleted underlying value
        m_colorDialog->blockSignals(true);
        if (m_color) {
            m_colorDialog->setCurrentColor(*m_color);
        }
        else {
            m_colorDialog->setCurrentColor(Color(*m_colorVector));
        }
        m_colorDialog->blockSignals(false);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::onColorChanged()
{
    updateColor();
    setLabelColor();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::initializeWidgets()
{
    m_labelPixmap = SAIcon("fill-drip").pixmap(0.5 * Renderable::screenDPIX(),
        0.5 * Renderable::screenDPIY());

    m_colorButton = new QPushButton(SAIcon("fill-drip"), "");
    setLabelColor();
    m_colorButton->setMaximumWidth(0.5 * Renderable::screenDPIX());
    m_colorButton->setMinimumHeight(0.35 * Renderable::screenDPIX());

    m_colorDialog = new QColorDialog();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::initializeConnections()
{
    // Dialog
    connect(m_colorDialog, &QColorDialog::currentColorChanged, this,
        &ColorWidget::onColorChanged);

    // Base widget
    connect(m_colorButton, &QPushButton::clicked,
        this,
        [this](bool checked) {
        Q_UNUSED(checked);
        m_colorDialog->show();
        m_colorDialog->activateWindow();
        m_colorDialog->raise();
    }
    );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::layoutWidgets()
{
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_colorButton);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::updateColor()
{
    if (m_color) {
        *m_color = m_colorDialog->currentColor();
    }
    else {
        *m_colorVector = Color(m_colorDialog->currentColor()).toVector4g();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ColorWidget::setLabelColor()
{
    if (m_color) {
        m_labelPixmap = Image::SetPixmapColor(m_labelPixmap, *m_color);
    }
    else {
        m_labelPixmap = Image::SetPixmapColor(m_labelPixmap, Color(*m_colorVector));
    }
    //size_t borderWidth = 0.025 * Renderable::screenDPIX();
    //QString colorStr = QString("QPushButton { border:%1 solid %2;}").arg(
    //    QString::number(borderWidth), m_color.name());
    //m_colorButton->setStyleSheet(colorStr);
    m_colorButton->setIcon(QIcon(m_labelPixmap));
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
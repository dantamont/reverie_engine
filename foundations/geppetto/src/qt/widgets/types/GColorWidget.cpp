#include "geppetto/qt/widgets/types/GColorWidget.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/layer/types/GQtConverter.h"

#include <QScreen>

namespace rev {


ColorWidget::ColorWidget(WidgetManager* wm, Color& color, QWidget* parent):
    ParameterWidget(wm, parent),
    m_color(&color)
{
    initialize();
}

ColorWidget::ColorWidget(WidgetManager* wm, Vector4& color, QWidget* parent) :
    ParameterWidget(wm, parent),
    m_colorVector(&color)
{
    initialize();
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::update()
{
    if (!m_colorDialog->hasFocus()) {
        // Will crash on widget delete if modifying a deleted underlying value
        m_colorDialog->blockSignals(true);
        if (m_color) {
            m_colorDialog->setColor(QConverter::ToQt(*m_color));
        }
        else {
            m_colorDialog->setColor(QConverter::ToQt(Color(*m_colorVector)));
        }
        m_colorDialog->blockSignals(false);
    }
}

void ColorWidget::onColorChanged()
{
    updateColor();
    setLabelColor();
    emit colorChanged(*m_color);
}

void ColorWidget::initializeWidgets()
{
    /// @todo Replace with Viewport::ScreenDPI calls
    QScreen* screen = QGuiApplication::primaryScreen();
    Float32_t screenDpiX = screen->logicalDotsPerInchX();
    Float32_t screenDpiY = screen->logicalDotsPerInchY();

    m_labelPixmap = SAIcon("fill-drip").pixmap(0.5 * screenDpiX, 0.5 * screenDpiY);

    m_colorButton = new QPushButton(SAIcon("fill-drip"), "");
    setLabelColor();
    m_colorButton->setMaximumWidth(0.5 * screenDpiX);
    m_colorButton->setMinimumHeight(0.35 * screenDpiY);

    m_colorDialog = new color_widgets::ColorDialog();
}

void ColorWidget::initializeConnections()
{
    // Dialog
    connect(m_colorDialog, &color_widgets::ColorDialog::colorChanged, this,
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

void ColorWidget::layoutWidgets()
{
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_colorButton);
}

void ColorWidget::updateColor()
{
    if (m_color) {
        *m_color = QConverter::FromQt(m_colorDialog->color());
    }
    else {
        *m_colorVector = Color(QConverter::FromQt(m_colorDialog->color())).toVector<Real_t, 4>();
    }
}

void ColorWidget::setLabelColor()
{
    if (m_color) {
        m_labelPixmap = QConverter::SetPixmapColor(m_labelPixmap, *m_color);
    }
    else {
        m_labelPixmap = QConverter::SetPixmapColor(m_labelPixmap, Color(*m_colorVector));
    }
    //size_t borderWidth = 0.025 * Viewport::ScreenDPIX();
    //QString colorStr = QString("QPushButton { border:%1 solid %2;}").arg(
    //    QString::number(borderWidth), m_color.name());
    //m_colorButton->setStyleSheet(colorStr);
    m_colorButton->setIcon(QIcon(m_labelPixmap));
}


} // rev
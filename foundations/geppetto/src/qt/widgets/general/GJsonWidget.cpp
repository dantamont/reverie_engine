#include "geppetto/qt/widgets/general/GJsonWidget.h"

#include "fortress/json/GJson.h"

#include "ripple/network/gateway/GMessageGateway.h"
#include "ripple/network/messages/GUpdateJsonMessage.h"
#include "ripple/network/messages/GOnUpdateJsonMessage.h"

#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GJsonWidget.h"

namespace rev {

JsonWidgetInterface::JsonWidgetInterface(WidgetManager* wm, const json& j, const json& metadata, QWidget* parent):
    ParameterWidget(wm, parent)
{
    setObjectJson(j);
    setMetadata(metadata);
}

JsonWidgetInterface::~JsonWidgetInterface()
{
    if (m_toggle) {
        m_toggle->deleteLater();
    }
    if (m_typeLabel) {
        m_typeLabel->deleteLater();
    }
    if (m_textEdit) {
        m_textEdit->deleteLater();
    }
    if (m_confirmButton) {
        m_confirmButton->deleteLater();
    }
}

void JsonWidgetInterface::updateText()
{
    // Block signals to avoid infinite loop of signal sends
    m_textEdit->blockSignals(true);

    // Get text and color if not valid JSON
    QString text = m_textEdit->toPlainText();

    json textJson = json::parse(text.toStdString(), nullptr, false);
    QPalette palette;
    QColor badColor = QApplication::palette().color(QPalette::BrightText);
    QColor goodColor = QApplication::palette().color(QPalette::Highlight);
    if (textJson.is_null() || textJson.is_discarded()) {
        palette.setColor(QPalette::Text, badColor);
        m_textEdit->setPalette(palette);
    }
    else {
        palette.setColor(QPalette::Text, goodColor);
        m_textEdit->setPalette(palette);
    }

    m_textEdit->blockSignals(false);
}


JsonWidget::JsonWidget(WidgetManager* wm, const json& j, const json& metadata, QWidget* parent, bool init) :
    JsonWidgetInterface(wm, j, metadata, parent)
{
    m_json["widgetId"] = m_uuid;

    if (init) {
        initialize();
    }
}

void JsonWidget::update(GMessage* message)
{
    GOnUpdateJsonMessage* updateMessage = static_cast<GOnUpdateJsonMessage*>(message);
    m_json = GJson::FromBytes(updateMessage->getJsonBytes());

    m_textEdit->setText(GJson::ToString<QString>(getObjectJson(), true));
}

void JsonWidget::wheelEvent(QWheelEvent* event)
{
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}

void JsonWidget::initializeWidgets()
{
    m_textEdit = new QTextEdit();
    m_textEdit->setText(GJson::ToString<QString>(getObjectJson(), true));
    m_confirmButton = new QPushButton();
    m_confirmButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}

void JsonWidget::initializeConnections()
{
    // Make component to recolor text on change
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        updateText();
        });

    // Make connection to resize serializable 
    connect(m_confirmButton, &QPushButton::clicked, this, [this](bool checked) {
        Q_UNUSED(checked);

        // Get text and return if not valid JSON
        const QString& text = m_textEdit->toPlainText();
        setObjectJson(json::parse(text.toStdString()));
        if (getObjectJson().is_null() || getObjectJson().is_discarded()) return;

        // Send message to edit serializable object via JSON
        m_message.setJsonBytes(GJson::ToBytes(m_json));
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_message);

        // Update widget text with updated JSON
        //m_textEdit->setText(
        //    GJson::ToString<QString>(json{ *m_serializable }, true)
        //);

        emit updatedJson(m_json);

        });

}

void JsonWidget::layoutWidgets()
{
    // Format widget sizes
    m_textEdit->setMaximumHeight(350);

    // Create base widget layout
    m_mainLayout = new QVBoxLayout;

    // Create new widgets
    QBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(m_textEdit);
    layout->addWidget(m_confirmButton);

    // Add widgets to main layout
    m_mainLayout->addLayout(layout);

    ///// @note cannot call again without deleting previous layout
    //// https://doc.qt.io/qt-5/qwidget.html#setLayout
    //setLayout(m_mainLayout);
}

} // rev
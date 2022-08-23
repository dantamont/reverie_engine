#include "geppetto/qt/widgets/types/GMaterialWidgets.h"

#include "fortress/json/GJson.h"
#include "fortress/system/memory/GPointerTypes.h"

#include "ripple/network/gateway/GMessageGateway.h"

#include "geppetto/qt/actions/GActionManager.h"
#include "geppetto/qt/style/GFontIcon.h"
#include "geppetto/qt/widgets/GWidgetManager.h"
#include "geppetto/qt/widgets/general/GFileLoadWidget.h"

namespace rev {


LoadTextureWidget::LoadTextureWidget(WidgetManager* wm, QWidget * parent):
    ParameterWidget(wm, parent)
{
    initialize();
}

LoadTextureWidget::~LoadTextureWidget()
{
    delete m_fileLoadWidget;
    delete m_confirmButtons;
}

void LoadTextureWidget::initializeWidgets()
{
    // File load widget
    m_fileLoadWidget = new FileLoadWidget(m_widgetManager,
        "",
        "Load Texture",
        "Images (*.png *.jpg *.tiff *.jpeg *.tga *.bmp)");

    // Dialog buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);
}

void LoadTextureWidget::initializeConnections()
{
    // Cache file name locally
    connect(m_fileLoadWidget->lineEdit(), &QLineEdit::textChanged,
        this,
        [&](const QString& text) {
        m_fileName = text;
    });

    // Dialog buttons
    connect(m_confirmButtons, &QDialogButtonBox::accepted,
        this,
        [this]() {

        m_loadMessage.setFilePath(m_fileName.toStdString().c_str());
        m_textureHandleID = Uuid();
        m_loadMessage.setUuid(m_textureHandleID);
        m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_loadMessage);

        // Close this widget
        close();
    });
    connect(m_confirmButtons, &QDialogButtonBox::rejected,
        this,
        [this]() {
        // Close this widget
        close();
    });
}

void LoadTextureWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);

    // Model load widgets
    QBoxLayout* fileLoadLayout = LabeledLayout("Texture File:", m_fileLoadWidget);
    fileLoadLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->addLayout(fileLoadLayout);
    m_mainLayout->addWidget(m_confirmButtons);

    setWindowTitle("Load Texture");
}



} // rev
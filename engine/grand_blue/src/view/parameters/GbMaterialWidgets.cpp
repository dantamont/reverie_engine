#include "GbMaterialWidgets.h"
#include "../style/GbFontIcon.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/components/GbModelComponent.h"
#include "../../core/rendering/models/GbModel.h"
#include "../../core/rendering/materials/GbMaterial.h"
#include "../../core/utils/GbMemoryManager.h"
#include "../../core/resource/GbResourceCache.h"
#include "../../core/resource/GbResource.h"
#include "../../core/GbCoreEngine.h"

namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LoadTextureWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadTextureWidget::LoadTextureWidget(CoreEngine * core, QWidget * parent):
    ParameterWidget(core, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LoadTextureWidget::~LoadTextureWidget()
{
    delete m_fileLoadWidget;
    delete m_confirmButtons;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LoadTextureWidget::initializeWidgets()
{
    // File load widget
    m_fileLoadWidget = new View::FileLoadWidget(m_engine,
        "",
        "Load Texture",
        "Images (*.png *.jpg *.tiff *.jpeg *.tga *.bmp)");

    // Dialog buttons
    QDialogButtonBox::StandardButtons dialogButtons = QDialogButtonBox::Ok |
        QDialogButtonBox::Cancel;
    m_confirmButtons = new QDialogButtonBox(dialogButtons);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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

        // Load model from file
        QFile textureFile(m_fileName);
        if (textureFile.exists()) {
            // Load texture file if path exists
            m_textureHandleID = m_engine->resourceCache()->guaranteeHandleWithPath(m_fileName,
                Resource::kTexture)->getUuid();
        }

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
///////////////////////////////////////////////////////////////////////////////////////////////////
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





///////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb
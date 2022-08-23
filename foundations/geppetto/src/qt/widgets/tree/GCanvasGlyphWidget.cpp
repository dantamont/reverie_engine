#include "geppetto/qt/widgets/tree/GCanvasGlyphWidget.h"

#include "fortress/system/memory/GPointerTypes.h"
#include "enums/GGlyphTypeEnum.h"

#include "geppetto/qt/fonts/GFontManager.h"
#include "geppetto/qt/widgets/components/GTransformComponentWidget.h"
#include "geppetto/qt/widgets/types/GVectorWidget.h"
#include "geppetto/qt/widgets/types/GColorWidget.h"
#include "geppetto/qt/widgets/GWidgetManager.h"

namespace rev {

GlyphWidget::GlyphWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent) :
    JsonWidgetInterface(wm, glyphJson, { {"isGlyphWidget", true} }, parent),
    m_glyphJson(glyphJson),
    m_sceneObjectId(sceneObjectId)
{
    m_alignmentMessage.setSceneObjectId(sceneObjectId);
    m_alignmentMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
}

GlyphWidget::~GlyphWidget()
{
}

void GlyphWidget::initializeWidgets()
{
    m_verticalAlignment = new QComboBox();
    m_verticalAlignment->addItems({"Top", "Center", "Bottom"});
    m_verticalAlignment->setCurrentIndex(m_glyphJson["vertAlign"].get<Int32_t>());
    m_verticalAlignment->setToolTip("Vertical Alignment");

    m_horizontalAlignment = new QComboBox();
    m_horizontalAlignment->addItems({ "Right", "Center", "Left" });
    m_horizontalAlignment->setCurrentIndex(m_glyphJson["horAlign"].get<Int32_t>());
    m_horizontalAlignment->setToolTip("Horizontal Alignment");

    m_transform = new TransformWidget(m_widgetManager, 
        m_glyphJson["transform"],
        { 
            {"glyphType", m_glyphJson["type"]}, 
            {"glyphId", m_glyphJson["uuid"]},
            {"sceneObjectId", m_sceneObjectId}
        }
    );
}

void GlyphWidget::initializeConnections()
{
    // Vertical alignment
    connect(m_verticalAlignment,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
            requestAlignmentUpdate(idx, m_horizontalAlignment->currentIndex());
        }
    );

    // Horizontal alignment
    connect(m_horizontalAlignment,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
            requestAlignmentUpdate(m_verticalAlignment->currentIndex(), idx);
        }
    );
}

void GlyphWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    auto* alignBox = new QGroupBox("Anchor Side");
    auto* alignmentLayout = new QHBoxLayout();
    QBoxLayout* horLayout = LabeledLayout("Horizontal", m_horizontalAlignment);
    horLayout->setSpacing(0);
    QBoxLayout* vertLayout = LabeledLayout("Vertical", m_verticalAlignment);
    vertLayout->setSpacing(0);
    alignmentLayout->addLayout(horLayout);
    alignmentLayout->addSpacing(50);
    alignmentLayout->addLayout(vertLayout);
    alignmentLayout->setSpacing(5);
    alignBox->setToolTip("The alignment of the glyph in its bounding box");
    alignBox->setLayout(alignmentLayout);
    m_mainLayout->addWidget(alignBox);

    auto* transformBox = new QGroupBox("Glyph Transform");
    auto* transformLayout = new QVBoxLayout();
    transformLayout->addWidget(m_transform);
    transformLayout->setSpacing(0);
    transformBox->setToolTip("The transform of the glyph in world space");
    transformBox->setLayout(transformLayout);

    m_mainLayout->addWidget(transformBox);
}

void GlyphWidget::requestAlignmentUpdate(Int32_t vertAlign, Int32_t horAlign)
{
    m_alignmentMessage.setVerticalAlignment(vertAlign);
    m_alignmentMessage.setHorizontalAlignment(horAlign);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_alignmentMessage);
}



// LabelWidget

LabelWidget::LabelWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent) :
    GlyphWidget(wm, glyphJson, sceneObjectId, parent)
{
    initialize();
    m_textMessage.setSceneObjectId(m_sceneObjectId);
    m_textMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
    m_fontMessage.setSceneObjectId(m_sceneObjectId);
    m_fontMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
    m_spacingMessage.setSceneObjectId(m_sceneObjectId);
    m_spacingMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
    m_colorMessage.setSceneObjectId(m_sceneObjectId);
    m_colorMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
}

LabelWidget::~LabelWidget()
{
}

void LabelWidget::requestLabelTextUpdate(const GString& text)
{
    std::vector<Uint8_t> textVec = std::vector<Uint8_t>(text.begin(), text.end());
    m_textMessage.setText(textVec);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_textMessage);
}

void LabelWidget::requestLabelFontUpdate(double fontSize, const GString& text)
{
    m_fontMessage.setFontName(text.c_str());
    m_fontMessage.setFontSize(fontSize);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_fontMessage);
}

void LabelWidget::requestLabelSpacingUpdate(double maxLineWidth, double verticalLineSpacing)
{
    m_spacingMessage.setMaxLineWidth(maxLineWidth);
    m_spacingMessage.setVerticalLineSpacing(verticalLineSpacing);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_spacingMessage);
}

void LabelWidget::requestLabelColorUpdate(const Color& color)
{
    m_cachedColor = color;
    m_colorMessage.setColor(color);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_colorMessage);
}

void LabelWidget::initializeWidgets()
{
    GlyphWidget::initializeWidgets();

    m_text = new QTextEdit();
    m_text->setText(m_glyphJson["text"].get_ref<const std::string&>().c_str());

    QScreen* mainScreen = QGuiApplication::primaryScreen();
    Float32_t dpi = mainScreen->logicalDotsPerInch();
    m_fontSize = new QLineEdit();
    m_fontSize->setValidator(new QDoubleValidator(0.1, 1e7, 2));
    m_fontSize->setText(QString::number(m_glyphJson["fontSize"].get<Float32_t>()));
    m_fontSize->setMaximumWidth(dpi * 0.5);

    m_fontFace = new QComboBox();
    int count = 0;
    const QString currentFontFace = m_glyphJson["font"].get_ref<const std::string&>().c_str();
    int fontFaceIndex = -1;
    for (const FontFace& face : FontManager::FontFaces()) {
        QString key = (QString)face.getName();
        m_fontFace->addItem(key);
        QFont font = QFont(key);
        m_fontFace->setItemData(count, font, Qt::FontRole);

        if (key == currentFontFace) {
            fontFaceIndex = count;
        }
        count++;
    }
    if (fontFaceIndex < 0) {
#ifdef DEBUG_MODE
        assert(false && "Error, font face not found");
#endif

    }
    m_fontFace->setCurrentIndex(fontFaceIndex);

    m_lineMaxSize = new QLineEdit();
    m_lineMaxSize->setValidator(new QDoubleValidator(0.1, 1e7, 7));
    m_lineMaxSize->setText(QString::number(m_glyphJson["maxLineWidth"].get<Float32_t>()));

    m_lineSpacing = new QLineEdit();
    m_lineSpacing->setValidator(new QDoubleValidator(-1e7, 1e7, 7));
    m_lineSpacing->setText(QString::number(m_glyphJson["lineSpacing"].get<Float32_t>()));

    m_cachedColor = m_glyphJson["color"];
    m_color = new ColorWidget(m_widgetManager, m_cachedColor);
}

void LabelWidget::initializeConnections()
{
    GlyphWidget::initializeConnections();

    // Text
    connect(m_text,
        &QTextEdit::textChanged,
        this,
        [this]() {
            requestLabelTextUpdate(m_text->toPlainText().toStdString().c_str());
        }
    );

    // Font size
    connect(m_fontSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
            requestLabelFontUpdate(m_fontSize->text().toDouble(), m_fontFace->currentText().toStdString().c_str());
        }
    );

    // Font face
    connect(m_fontFace,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
            Q_UNUSED(idx);
            requestLabelFontUpdate(m_fontSize->text().toDouble(), m_fontFace->currentText().toStdString().c_str());
        }
    );

    // Line width
    connect(m_lineMaxSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
            requestLabelSpacingUpdate(m_lineMaxSize->text().toDouble(), m_lineSpacing->text().toDouble());
        }
    );

    // Line spacing
    connect(m_lineSpacing,
        &QLineEdit::editingFinished,
        this,
        [this]() {
            requestLabelSpacingUpdate(m_lineMaxSize->text().toDouble(), m_lineSpacing->text().toDouble());
        }
    );

    // Color
    connect(m_color, &ColorWidget::colorChanged, this, &LabelWidget::requestLabelColorUpdate);
}

void LabelWidget::layoutWidgets()
{
    GlyphWidget::layoutWidgets();

    QGroupBox* labelWidgets = new QGroupBox();
    QVBoxLayout* labelLayout = new QVBoxLayout();
    labelLayout->addLayout(LabeledLayout("Text:", m_text));

    QHBoxLayout* fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel("Font:"));
    fontLayout->addSpacing(15);
    fontLayout->addWidget(m_fontFace);
    fontLayout->addWidget(new QLabel("Font Size:"));
    fontLayout->addSpacing(15);
    fontLayout->addWidget(m_fontSize);
    labelLayout->addLayout(fontLayout);

    QHBoxLayout* lineLayout = new QHBoxLayout();
    lineLayout->addWidget(new QLabel("Line Width:"));
    lineLayout->addSpacing(15);
    lineLayout->addWidget(m_lineMaxSize);
    lineLayout->addWidget(new QLabel("Line Spacing:"));
    lineLayout->addSpacing(15);
    lineLayout->addWidget(m_lineSpacing);
    labelLayout->addLayout(lineLayout);

    labelLayout->addWidget(m_color);

    labelWidgets->setLayout(labelLayout);

    m_mainLayout->insertWidget(0, labelWidgets);
}



// IconWidget

IconWidget::IconWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget *parent) :
    GlyphWidget(wm, glyphJson, sceneObjectId, parent)
{
    initialize();
    m_nameMessage.setSceneObjectId(sceneObjectId);
    m_nameMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
    m_sizeMessage.setSceneObjectId(sceneObjectId);
    m_sizeMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
    m_colorMessage.setSceneObjectId(sceneObjectId);
    m_colorMessage.setGlyphId(m_glyphJson["uuid"].get<Uuid>());
}

IconWidget::~IconWidget()
{
}

void IconWidget::initializeWidgets()
{
    GlyphWidget::initializeWidgets();

    // TODO: Use a line edit to filter combobox
    m_fontAwesomeIcon = new QComboBox();
    const json& faInfo = FontManager::FontAwesomeInfo();
    const std::string& currentIcon = m_glyphJson["iconName"];
    int currentIndex = 0;
    int count = 0;
    for (const auto& jsonPair : faInfo.items()) {
        const json& styles = jsonPair.value()["styles"];
        if (!styles.contains("solid") && !styles.contains("regular")) {
            // Skip brands
            continue;
        }
        if (jsonPair.key() == currentIcon) {
            currentIndex = count;
        }
        QString icon = FontManager::FaUnicodeCharacter(jsonPair.key().c_str()).string();
        m_fontAwesomeIcon->addItem(icon + " " + jsonPair.key().c_str(), jsonPair.key().c_str());
        count++;
    }
    m_fontAwesomeIcon->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_fontAwesomeIcon->setCurrentIndex(currentIndex);

    QScreen* mainScreen = QGuiApplication::primaryScreen();
    Float32_t dpi = mainScreen->logicalDotsPerInch();
    m_fontSize = new QLineEdit();
    m_fontSize->setValidator(new QDoubleValidator(0.1, 1e7, 2));
    m_fontSize->setText(QString::number(m_glyphJson["fontSize"].get<Float32_t>()));
    m_fontSize->setMaximumWidth(dpi * 0.5);

    m_cachedColor = m_glyphJson["color"];
    m_color = new ColorWidget(m_widgetManager, m_cachedColor);
}

void IconWidget::initializeConnections()
{
    GlyphWidget::initializeConnections();

    // Font icon
    connect(m_fontAwesomeIcon, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
            Q_UNUSED(idx);
            requestIconUpdate(m_fontAwesomeIcon->currentData().toString().toStdString().c_str());
            // TODO: Emit a signal to canvas component widget to refresh icon entry
        }
    );

    // Font size
    connect(m_fontSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
            requestIconSizeUpdate(m_fontSize->text().toDouble());
        }
    );

    connect(m_color, &ColorWidget::colorChanged, this, &IconWidget::requestIconColorUpdate);

}

void IconWidget::layoutWidgets()
{
    GlyphWidget::layoutWidgets();

    QGroupBox* iconWidgets = new QGroupBox();
    QVBoxLayout* iconLayout = new QVBoxLayout();

    auto* layout = LabeledLayout("Icon:", m_fontAwesomeIcon);
    layout->addSpacing(15);
    layout->addWidget(new QLabel("Font Size:"));
    layout->addWidget(m_fontSize);
    layout->addSpacing(15);
    layout->addWidget(m_color);
    iconLayout->addLayout(layout);

    iconWidgets->setLayout(iconLayout);

    m_mainLayout->insertWidget(0, iconWidgets);
}

void IconWidget::requestIconUpdate(const GString& iconName)
{
    m_nameMessage.setIconName(iconName.c_str());
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_nameMessage);
}

void IconWidget::requestIconSizeUpdate(double fontSize)
{
    m_sizeMessage.setFontSize(fontSize);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_sizeMessage);
}

void IconWidget::requestIconColorUpdate(const Color& color)
{
    m_cachedColor = color;
    m_colorMessage.setColor(color);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_colorMessage);
}




SpriteWidget::SpriteWidget(WidgetManager* wm, const json& glyphJson, Uint32_t sceneObjectId, QWidget* parent) :
    GlyphWidget(wm, glyphJson, sceneObjectId, parent)
{
    initialize();
}

SpriteWidget::~SpriteWidget()
{
}

void SpriteWidget::initializeWidgets()
{
    GlyphWidget::initializeWidgets();

    m_jsonWidget = new JsonWidget(
        m_widgetManager,
        m_glyphJson, 
        { {"isSpriteWidget", true}, {"sceneObjectId", m_sceneObjectId} },
        this,
        true);
}

void SpriteWidget::initializeConnections()
{
    GlyphWidget::initializeConnections();
}

void SpriteWidget::layoutWidgets()
{
    GlyphWidget::layoutWidgets();

    // Create groupbox with layout containing JSON widget
    QVBoxLayout* spriteLayout = new QVBoxLayout();
    QGroupBox* spriteWidgets = new QGroupBox();
    auto* layout = LabeledLayout("Sprite:", m_jsonWidget);
    spriteLayout->addLayout(layout);
    spriteWidgets->setLayout(spriteLayout);

    // Add widget to main layout
    m_mainLayout->insertWidget(0, spriteWidgets);
}


// GlyphItem

GlyphItem::GlyphItem(const json& glyphJson, GlyphItemType type, Int32_t id) :
    TreeItem(id, type, glyphJson)
{
    initializeItem();
}

GlyphItem::~GlyphItem()
{
}

void GlyphItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) assert(false && "Error, item already has a widget");

    // Get parent tree widget
    CanvasGlyphWidget * parentWidget = static_cast<CanvasGlyphWidget*>(treeWidget());

    // Set widget
    /// @todo See if serializing text to JSON from QString messes with display and encoding
    QString text;
    switch ((GlyphItemType)type()) {
    case kLabel:
        text = ("Label: " + m_data.getRef<const std::string&>("text")).c_str();
        break;
    case kIcon:
        text = ("Icon: " + m_data.getRef<const std::string&>("text") + 
            " (" + m_data.getRef<const std::string&>("iconName")  + ")").c_str();
        break;
    case kSprite:
        text = "Sprite: " + QString(m_data.getRef<const std::string&>("name").c_str());
        break;
    default:
        assert(false && "Unimplemented");
        break;
    }
    m_widget = new QLabel(text);

    // Connect signals and slots
    QObject::connect((GlyphWidget*)m_widget, &GlyphWidget::editedGlyphs,
        parentWidget, &CanvasGlyphWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}

void GlyphItem::removeWidget(int column)
{
    // Only ever one column, so don't need to worry about indexing
    Q_UNUSED(column)
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}

CanvasGlyphWidget * GlyphItem::canvasGlyphWidget() const
{
    return static_cast<CanvasGlyphWidget*>(treeWidget());
}





// CanvasGlyphWidget

CanvasGlyphWidget::CanvasGlyphWidget(WidgetManager* wm, json& canvasComponentJson, Uint32_t sceneObjectId, QWidget* parent) :
    TreeWidget(wm, wm->actionManager(), "Script Execution Order", parent),
    m_canvasComponentJson(canvasComponentJson),
    m_sceneObjectId(sceneObjectId),
    m_selectedGlyphWidget(nullptr)
{
    m_addMessage.setSceneObjectId(m_sceneObjectId);
    m_reparentMessage.setSceneObjectId(m_sceneObjectId);
    m_removeMessage.setSceneObjectId(m_sceneObjectId);
    initializeWidget();
    repopulate();
}

void CanvasGlyphWidget::requestLabel()
{
    m_addMessage.setGlyphType((Int32_t)EGlyphType::eLabel);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addMessage);
}

void CanvasGlyphWidget::requestIcon()
{
    m_addMessage.setGlyphType((Int32_t)EGlyphType::eIcon);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addMessage);
}

void CanvasGlyphWidget::requestSprite()
{
    m_addMessage.setGlyphType((Int32_t)EGlyphType::eSprite);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_addMessage);
}

void CanvasGlyphWidget::requestRemoveGlyph(const Uuid& uuid)
{
    m_reparentMessage.setGlyphId(uuid);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_removeMessage);
}

void CanvasGlyphWidget::requestReparentGlyph(const Uuid& uuid, const Uuid& parentId)
{
    m_reparentMessage.setGlyphId(uuid);
    m_reparentMessage.setParentGlyphId(parentId);
    m_widgetManager->messageGateway()->copyAndQueueMessageForSend(m_reparentMessage);
}

CanvasGlyphWidget::~CanvasGlyphWidget()
{
}

void CanvasGlyphWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    QTreeWidgetItem* headerItem = new QTreeWidgetItem(0);
    headerItem->setText(0, "Glyphs");
    setHeaderItem(headerItem);
    resizeColumns();

    // Add glyphs to widget
    for (json& glyphJson : m_canvasComponentJson["glyphs"]) {
        addItem(glyphJson);
    }

}

void CanvasGlyphWidget::addItem(json& glyphJson)
{
    // Create glyph item
    static Uint32_t count = 0;
    GlyphItem* layerItem = new GlyphItem(
        glyphJson, 
        (GlyphItem::GlyphItemType)(glyphJson["type"].get<Int32_t>() + 2000), 
        count);
    count++;

    Int32_t parentIndex = glyphJson["parent"].get<Int32_t>();
    if (parentIndex >= 0) {
        // If is a child glyph, parent to appropriate item
        GlyphItem* parentItem = (GlyphItem*)getItem(parentIndex);
        parentItem->addChild(layerItem);
        layerItem->setWidget();
        parentItem->setExpanded(true);
    }
    else {
        // Otherwise, add at top level
        TreeWidget::addItem(layerItem);
    }
}

void CanvasGlyphWidget::removeItem(GlyphItem * GlyphItem)
{
    delete GlyphItem;
}

void CanvasGlyphWidget::initializeWidget()
{
    TreeWidget::initializeWidget();

    QTreeWidgetItem* headerItem = new QTreeWidgetItem(0);
    headerItem->setText(0, "Glyphs");
    setHeaderItem(headerItem);

    setColumnCount(0);
    setMinimumSize(50, 100);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setHeaderLabels(QStringList({ "" }));
    setAlternatingRowColors(true);

    //initializeAsList();
    enableDragAndDrop();

    // Initialize actions
    addAction(kNoItemSelected,
        "Add Label",
        "Add a label",
        [this] {
            requestLabel();
        }
    );
    addAction(kNoItemSelected,
        "Add Icon",
        "Add an icon",
        [this] {
            requestIcon();
        }
    );
    addAction(kNoItemSelected,
        "Add Sprite",
        "Add a sprite",
        [this] {
            requestSprite();
        }
    );

    addAction(kItemSelected,
        "Remove Glyph",
        "Remove Glyph",
        [this] {
            // Remove the selected glyph from the canvas
            Uuid glyphId = currentContextItem()->data().get<Uuid>("uuid");
            requestRemoveGlyph(glyphId);
        }
    );

    // Connect to widget manager to received canvas information
    connect(m_widgetManager, &WidgetManager::receivedCanvasDataMessage, this, &CanvasGlyphWidget::repopulate);
}

void CanvasGlyphWidget::onItemDoubleClicked(QTreeWidgetItem * item, int column)
{
    TreeWidget::onItemDoubleClicked(item, column);
    auto* glyphItem = static_cast<GlyphItem*>(item);

    // If a widget exists, delete
    if (m_selectedGlyphWidget) {
        m_selectedGlyphWidget->hide();
        delete m_selectedGlyphWidget;
        m_selectedGlyphWidget = nullptr;
    }

    // Show widget
    switch ((GlyphItem::GlyphItemType)(glyphItem->type())) {
    case GlyphItem::kLabel:
    {
        m_selectedGlyphWidget = new LabelWidget(m_widgetManager, glyphItem->data().m_data.m_json, m_sceneObjectId);
        break;
    }
    case GlyphItem::kIcon:
    {
        m_selectedGlyphWidget = new IconWidget(m_widgetManager, glyphItem->data().m_data.m_json, m_sceneObjectId);
        break;
    }
    case GlyphItem::kSprite:
    {
        m_selectedGlyphWidget = new SpriteWidget(m_widgetManager, glyphItem->data().m_data.m_json, m_sceneObjectId);
        break;
    }
    default:
        assert(false && "Error, unrecognized glyph item type");
    }
    m_selectedGlyphWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    m_selectedGlyphWidget->show();
}

void CanvasGlyphWidget::reparent(GlyphItem * item, GlyphItem * newParent)
{
    // Wasn't working, see: https://stackoverflow.com/questions/25559221/qtreewidgetitem-issue-items-set-using-setwidgetitem-are-dispearring-after-movin
    // TODO: Make a command to redo/undo this. See scene widget

    //// Reparent widget item
    // Remove from current parent in widget
    GlyphItem* takenItem;
    auto* oldParent = item->parent();
    if (oldParent) {
        oldParent->removeChild(item);
        takenItem = item;
    }
    else {
        // Need to take item from top-level or nothing happens
        takenItem = (GlyphItem*)takeTopLevelItem(indexOfTopLevelItem(item));
        if (!takenItem) {
            assert(false && "Failed to take item");
        }
    }

    // Add to new parent in tree widget
    if (newParent) {
        newParent->addChild(takenItem);
        newParent->setExpanded(true);
    }
    else {
        addTopLevelItem(takenItem);
    }

    // Need to add contents again, since Qt deletes for some reason
    //item->setWidget();

    //// Reparent actual glyph
    Uuid glyphId = takenItem->data().get<Uuid>("uuid");
    Uuid parentId = Uuid::NullID();
    if (newParent) {
        parentId = newParent->data().get<Uuid>("uuid");
    }
    requestReparentGlyph(glyphId, parentId);
}

void CanvasGlyphWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Next to item, so add as a sibling (child of destination item's parent)
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, (GlyphItem*)destItem->parent());

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}

GlyphItem* CanvasGlyphWidget::getItem(Int32_t parentIndex)
{
    json& glyphJson = m_canvasComponentJson["glyphs"][parentIndex];
    Uuid glyphId = glyphJson["uuid"];

    QTreeWidgetItemIterator it(this);
    while (*it) {
        GlyphItem* item = static_cast<GlyphItem*>(*it);
        if (item->data().get<Uuid>("uuid") == glyphId) {
            return item;
        }
        ++it;
    }

    assert(false && "Error, no item found that corresponds to the given object");

    return nullptr;
}

void CanvasGlyphWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Next to item, so add as a sibling (child of destination item's parent)
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, (GlyphItem*)destItem->parent());

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}

void CanvasGlyphWidget::onDropViewport(QDropEvent * event, QTreeWidgetItem * source)
{
    // On viewport, so set as root-level item
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    reparent(sourceItem, nullptr);

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}

void CanvasGlyphWidget::onDropOn(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // On an item, so add current item as child
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, destItem);

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}


void CanvasGlyphWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<GlyphItem*>(item)->setWidget();
}


// End namespaces
}

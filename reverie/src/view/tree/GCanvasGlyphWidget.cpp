#include "GCanvasGlyphWidget.h"

#include <core/loop/GSimLoop.h>
#include <core/readers/GJsonReader.h>
#include <core/scene/GScenario.h>
#include <core/components/GCanvasComponent.h>
#include <core/components/GTransformComponent.h>
#include <core/canvas/GFontManager.h>

#include <core/scene/GSceneObject.h>
#include <core/utils/GMemoryManager.h>
#include "../components/GTransformComponentWidget.h"
#include "../parameters/GVectorWidget.h"
#include "../parameters/GColorWidget.h"
#include "../GWidgetManager.h"
#include "../../GMainWindow.h"

namespace rev {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// GlyphWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
GlyphWidget::GlyphWidget(CoreEngine* core, Glyph* glyph, QWidget *parent) :
    JsonWidget(core, glyph, parent)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GlyphWidget::~GlyphWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::wheelEvent(QWheelEvent* event) {
    if (!event->pixelDelta().isNull()) {
        ParameterWidget::wheelEvent(event);
    }
    else {
        // If scrolling has reached top or bottom
        // Accept event and stop propagation if at bottom of scroll area
        event->accept();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::initializeWidgets()
{
    m_verticalAlignment = new QComboBox();
    m_verticalAlignment->addItems({"Top", "Center", "Bottom"});
    m_verticalAlignment->setCurrentIndex(glyph()->verticalAlignment());
    m_verticalAlignment->setToolTip("Vertical Alignment");

    m_horizontalAlignment = new QComboBox();
    m_horizontalAlignment->addItems({ "Right", "Center", "Left" });
    m_horizontalAlignment->setCurrentIndex(glyph()->horizontalAlignment());
    m_horizontalAlignment->setToolTip("Horizontal Alignment");

    //m_screenOffset = new VectorWidget<float, 2>(m_engine, glyph()->coordinates(), nullptr,  -1e7, 1e20, 5);

    m_transform = new TransformWidget(m_engine, &glyph()->transform());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::initializeConnections()
{

    // Vertical alignment
    connect(m_verticalAlignment,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        pauseSimulation();

        m_engine->setGLContext();

        glyph()->setVerticalAlignment(Glyph::VerticalAlignment(idx));

        resumeSimulation();
    });

    // Horizontal alignment
    connect(m_horizontalAlignment,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        pauseSimulation();

        m_engine->setGLContext();

        glyph()->setHorizontalAlignment(Glyph::HorizontalAlignment(idx));

        resumeSimulation();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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

    // Position-related
    //auto* soBox = new QGroupBox("Screen Offset");
    //auto* soLayout = new QVBoxLayout();
    //soLayout->addWidget(m_screenOffset);
    //soLayout->setSpacing(0);
    //soBox->setToolTip("The offset of the glyph in screen space");
    //soBox->setLayout(soLayout);
    //m_mainLayout->addWidget(soBox);

    auto* transformBox = new QGroupBox("Glyph Transform");
    auto* transformLayout = new QVBoxLayout();
    transformLayout->addWidget(m_transform);
    transformLayout->setSpacing(0);
    transformBox->setToolTip("The transform of the glyph in world space");
    transformBox->setLayout(transformLayout);

    m_mainLayout->addWidget(transformBox);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::preLoad()
{
    // Pause scenario to edit component
    SimulationLoop* simLoop = m_engine->simulationLoop();
    m_wasPlaying = simLoop->isPlaying();
    if (m_wasPlaying) {
        simLoop->pause();
    }
    m_engine->simulationLoop()->pause();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::postLoad()
{
    // Unpause scenario
    SimulationLoop* simLoop = m_engine->simulationLoop();
    if (m_wasPlaying) {
        simLoop->play();
    }

    emit editedGlyphs();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// LabelWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
LabelWidget::LabelWidget(CoreEngine* core, Label* label, QWidget *parent) :
    GlyphWidget(core, label, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
LabelWidget::~LabelWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LabelWidget::initializeWidgets()
{
    GlyphWidget::initializeWidgets();

    m_text = new QTextEdit();
    m_text->setText(label()->text());

    m_fontSize = new QLineEdit();
    m_fontSize->setValidator(new QDoubleValidator(0.1, 1e7, 2));
    m_fontSize->setText(QString::number(label()->getFontSize()));
    m_fontSize->setMaximumWidth(Viewport::ScreenDPIX() * 0.5);

    m_fontFace = new QComboBox();
    int count = 0;
    const QString& currentFontFace = label()->getFontFaceName();
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
        throw("Error, font face " + currentFontFace + "not found");
#else
        logError("Error, font face " + currentFontFace + "not found");
#endif

    }
    m_fontFace->setCurrentIndex(fontFaceIndex);

    m_lineMaxSize = new QLineEdit();
    m_lineMaxSize->setValidator(new QDoubleValidator(0.1, 1e7, 7));
    m_lineMaxSize->setText(QString::number(label()->lineMaxWidth()));

    m_lineSpacing = new QLineEdit();
    m_lineSpacing->setValidator(new QDoubleValidator(-1e7, 1e7, 7));
    m_lineSpacing->setText(QString::number(label()->lineSpacing()));

    m_color = new ColorWidget(m_engine, label()->color());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void LabelWidget::initializeConnections()
{
    GlyphWidget::initializeConnections();

    // Text
    connect(m_text,
        &QTextEdit::textChanged,
        this,
        [this]() {
        pauseSimulation();

        m_engine->setGLContext();

        label()->setText(m_text->toPlainText());

        resumeSimulation();
    });

    // Font size
    connect(m_fontSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        pauseSimulation();

        m_engine->setGLContext();

        float fontSize = m_fontSize->text().toDouble();

        label()->setFontSize(fontSize);

        resumeSimulation();
    });

    // Font face
    connect(m_fontFace,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {

        Q_UNUSED(idx)

        pauseSimulation();

        m_engine->setGLContext();

        label()->setFontFace(m_fontFace->currentText());

        resumeSimulation();
    });

    // Line width
    connect(m_lineMaxSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        pauseSimulation();

        m_engine->setGLContext();

        float lineSize = m_lineMaxSize->text().toDouble();

        label()->setLineMaxSize(lineSize);

        resumeSimulation();
    });

    // Line spacing
    connect(m_lineSpacing,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        pauseSimulation();

        m_engine->setGLContext();

        float lineSpacing = m_lineSpacing->text().toDouble();

        label()->setLineSpacing(lineSpacing);

        resumeSimulation();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
bool LabelWidget::isValidObject(const QJsonObject & object)
{
    Q_UNUSED(object);
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// IconWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
IconWidget::IconWidget(CoreEngine* core, Icon* icon, QWidget *parent) :
    GlyphWidget(core, icon, parent)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
IconWidget::~IconWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void IconWidget::initializeWidgets()
{
    GlyphWidget::initializeWidgets();

    // TODO: Use a line edit to filter combobox
    m_fontAwesomeIcon = new QComboBox();
    const QJsonObject& faInfo = FontManager::FontAwesomeInfo();
    const QString& currentIcon = icon()->iconName();
    int currentIndex = 0;
    int count = 0;
    for (const QString& key : faInfo.keys()) {
        QJsonArray styles = faInfo[key].toObject()["styles"].toArray();
        if (!styles.contains("solid") && !styles.contains("regular")) {
            // Skip brands
            continue;
        }
        if (key == currentIcon) {
            currentIndex = count;
        }
        QString icon = FontManager::FaUnicodeCharacter(key);
        m_fontAwesomeIcon->addItem(icon + " " + key, key);
        count++;
    }
    m_fontAwesomeIcon->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_fontAwesomeIcon->setCurrentIndex(currentIndex);

    m_fontSize = new QLineEdit();
    m_fontSize->setValidator(new QDoubleValidator(0.1, 1e7, 2));
    m_fontSize->setText(QString::number(icon()->getFontSize()));
    m_fontSize->setMaximumWidth(Viewport::ScreenDPIX() * 0.5);

    m_color = new ColorWidget(m_engine, icon()->color());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void IconWidget::initializeConnections()
{
    GlyphWidget::initializeConnections();

    // Font icon
    connect(m_fontAwesomeIcon, qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {

        // Set icon
        Q_UNUSED(idx);
        m_engine->setGLContext();
        icon()->setFontAwesomeIcon(m_fontAwesomeIcon->currentData().toString());

        // TODO: Emit a signal to canvas component widget to refresh icon entry
    });

    // Font size
    connect(m_fontSize,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        pauseSimulation();

        float fontSize = m_fontSize->text().toDouble();

        m_engine->setGLContext();
        icon()->setFontSize(fontSize);

        resumeSimulation();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
bool IconWidget::isValidObject(const QJsonObject & object)
{
    Q_UNUSED(object);
    return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// GlyphItem
///////////////////////////////////////////////////////////////////////////////////////////////////
GlyphItem::GlyphItem(Glyph * layer, GlyphItemType type) :
    TreeItem(layer, type)
{
    initializeItem();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
GlyphItem::~GlyphItem()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphItem::setWidget()
{
    // Throw error if the widget already exists
    if (m_widget) throw("Error, item already has a widget");

    // Get parent tree widget
    CanvasGlyphWidget * parentWidget = static_cast<CanvasGlyphWidget*>(treeWidget());

    // Set widget
    QString text;
    switch ((GlyphItemType)type()) {
    case kLabel:
    {
        text = "Label: " + static_cast<Label*>(glyph())->text();
        break;
    }
    case kIcon:
    {
        auto* icon = static_cast<Icon*>(glyph());
        text = "Icon: " + icon->text() + " (" + icon->iconName() + ")";
        break;
    }
    }
    m_widget = new QLabel(text);

    // Connect signals and slots
    QObject::connect((GlyphWidget*)m_widget, &GlyphWidget::editedGlyphs,
        parentWidget, &CanvasGlyphWidget::repopulate);

    // Assign widget to item in tree widget
    parentWidget->setItemWidget(this, 0, m_widget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphItem::removeWidget(int column)
{
    // Only ever one column, so don't need to worry about indexing
    Q_UNUSED(column)
    treeWidget()->removeItemWidget(this, 0);
    m_widget = nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
View::CanvasGlyphWidget * GlyphItem::canvasGlyphWidget() const
{
    return static_cast<View::CanvasGlyphWidget*>(treeWidget());
}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// CanvasGlyphWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasGlyphWidget::CanvasGlyphWidget(CoreEngine* engine, 
    CanvasComponent* canvas,
    QWidget* parent) : 
    TreeWidget(engine, "Script Execution Order", parent),
    m_canvas(canvas),
    m_selectedGlyphWidget(nullptr)
{
    initializeWidget();
    repopulate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CanvasGlyphWidget::~CanvasGlyphWidget()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::repopulate()
{
    // Clear the widget
    clear();

    // Set header item 
    QTreeWidgetItem* headerItem = new QTreeWidgetItem(0);
    headerItem->setText(0, "Glyphs");
    setHeaderItem(headerItem);
    resizeColumns();

    // Get render layers
    const std::vector<std::shared_ptr<Glyph>>& glyphs = m_canvas->glyphs();

    // Add glyphs to widget
    for (const std::shared_ptr<Glyph>& glyph : glyphs) {
        addItem(glyph.get());
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::addItem(Glyph * glyph)
{
    // Create resource item
    GlyphItem* layerItem = new View::GlyphItem(glyph,
        (GlyphItem::GlyphItemType)(glyph->glyphType() + 2000));

    Glyph* parent = glyph->getParent();
    if (parent) {
        // If is a child glyph, parent to appropriate item
        GlyphItem* parentItem = (GlyphItem*)getItem(*parent);
        parentItem->addChild(layerItem);
        layerItem->setWidget();
        parentItem->setExpanded(true);
    }
    else {
        // Otherwise, add at top level
        TreeWidget::addItem(layerItem);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::removeItem(GlyphItem * GlyphItem)
{
    delete GlyphItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::removeItem(Glyph* itemObject)
{
    GlyphItem* item = static_cast<GlyphItem*>(getItem(*itemObject));
    delete item;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
            // Add label to the canvas
            auto label = std::make_shared<Label>(m_canvas);  
            label->reload(); // Ensure text displays
            m_canvas->addGlyph(label);
            repopulate();
    });
    addAction(kNoItemSelected,
        "Add Icon",
        "Add an icon",
        [this] {
        // Add icon to the canvas
        auto icon = std::make_shared<Icon>(m_canvas);
        icon->reload(); // Ensure icon displays
        m_canvas->addGlyph(icon);
        repopulate();
    });

    addAction(kItemSelected,
        "Remove Glyph",
        "Remove Glyph",
        [this] {
        // Remove the selected glyph from the canvas
        Glyph* glyph = currentContextItem()->glyph();
        m_canvas->removeGlyph(*glyph);
        repopulate();
    });

}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
        m_selectedGlyphWidget = new LabelWidget(m_engine,
            dynamic_cast<Label*>(glyphItem->glyph()));
        break;
    }
    case GlyphItem::kIcon:
    {
        m_selectedGlyphWidget = new IconWidget(m_engine,
            dynamic_cast<Icon*>(glyphItem->glyph()));
        break;
    }
    default:
        throw("Error, unrecognized glyph item type");
    }
    m_selectedGlyphWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    m_selectedGlyphWidget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
            throw("Failed to take item");
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
    if (newParent) {
        takenItem->object()->setParent(newParent->object());
    }
    else {
        takenItem->object()->setParent(m_canvas);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::onDropAbove(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Next to item, so add as a sibling (child of destination item's parent)
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, (GlyphItem*)destItem->parent());

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::onDropBelow(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // Next to item, so add as a sibling (child of destination item's parent)
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, (GlyphItem*)destItem->parent());

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::onDropViewport(QDropEvent * event, QTreeWidgetItem * source)
{
    // On viewport, so set as root-level item
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    reparent(sourceItem, nullptr);

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::onDropOn(QDropEvent * event, QTreeWidgetItem * source, QTreeWidgetItem * destination)
{
    // On an item, so add current item as child
    GlyphItem* sourceItem = static_cast<GlyphItem*>(source);
    GlyphItem* destItem = static_cast<GlyphItem*>(destination);
    reparent(sourceItem, destItem);

    // If not set, Qt will do its own reparenting, which DOESN'T WORK
    event->setDropAction(Qt::IgnoreAction);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void CanvasGlyphWidget::dropEvent(QDropEvent * event)
//{
//    CanvasGlyphWidget* widget = static_cast<CanvasGlyphWidget*>(event->source());
//    if (widget == this) {
//        GlyphItem* sourceItem = static_cast<GlyphItem*>(widget->currentItem());
//        QModelIndex destIndex = indexAt(event->pos());
//        if (destIndex.isValid()) {
//            // Dropping onto an item
//            DropIndicatorPosition dropIndicator = dropIndicatorPosition();
//            switch (dropIndicator) {
//            case QAbstractItemView::AboveItem:
//                // Dropping above an item
//                // TODO: Implement reordering
//                event->setDropAction(Qt::IgnoreAction);
//                break;
//            case QAbstractItemView::BelowItem:
//                // Dropping below an item
//                // TODO: Implement reordering
//                event->setDropAction(Qt::IgnoreAction);
//            case QAbstractItemView::OnItem:
//            {
//                // On an item, so add current item as child
//                GlyphItem* destItem = static_cast<GlyphItem*>(itemFromIndex(destIndex));
//                reparent(sourceItem, destItem);
//                event->setDropAction(Qt::IgnoreAction);
//                break;
//            }
//            case QAbstractItemView::OnViewport:
//                // Not on the tree 
//                event->setDropAction(Qt::IgnoreAction);
//                break;
//            }
//        }
//        else {
//            // Dropping above an item
//            event->setDropAction(Qt::IgnoreAction);
//        }
//    }
//    else {
//        // Ignore drops from another widget
//        event->setDropAction(Qt::IgnoreAction);
//    }
//    QTreeWidget::dropEvent(event);
//}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<GlyphItem*>(item)->setWidget();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}
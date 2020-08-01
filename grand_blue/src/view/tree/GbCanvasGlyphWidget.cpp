#include "GbCanvasGlyphWidget.h"

#include "../../core/loop/GbSimLoop.h"
#include "../../core/readers/GbJsonReader.h"
#include "../../core/scene/GbScenario.h"
#include "../../core/components/GbCanvasComponent.h"
#include "../../core/components/GbTransformComponent.h"
#include "../../core/canvas/GbFonts.h"

#include "../../core/scene/GbSceneObject.h"
#include "../../core/utils/GbMemoryManager.h"
#include "../components/GbTransformComponentWidget.h"
#include "../parameters/GbVectorWidget.h"
#include "../parameters/GbColorWidget.h"
#include "../GbWidgetManager.h"
#include "../../GbMainWindow.h"

namespace Gb {
namespace View {


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// BillboardFlagsWidget
///////////////////////////////////////////////////////////////////////////////////////////////////
BillboardFlagsWidget::BillboardFlagsWidget(CoreEngine* core,
    Glyph* g,
    QWidget* parent) :
    ParameterWidget(core, parent),
    m_glyph(g)
{
    initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void BillboardFlagsWidget::update()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void BillboardFlagsWidget::initializeWidgets()
{
    m_areaWidget = new QWidget();
    QVBoxLayout* areaLayout = new QVBoxLayout;
    m_checkBoxes.push_back(new QCheckBox("Constant Screen Size"));
    m_checkBoxes.push_back(new QCheckBox("Always Face Camera"));
    m_checkBoxes.push_back(new QCheckBox("Always On Top"));

    m_checkBoxes[0]->setToolTip("Whether or not to preserve screen size on camera zoom.");

    m_checkBoxes[0]->setChecked(
        m_glyph->flags().testFlag(Glyph::kScale));
    m_checkBoxes[1]->setChecked(
        m_glyph->flags().testFlag(Glyph::kFaceCamera));
    m_checkBoxes[2]->setChecked(
        m_glyph->flags().testFlag(Glyph::kAlwaysOnTop));

    areaLayout->addWidget(m_checkBoxes[0]);
    areaLayout->addWidget(m_checkBoxes[1]);
    areaLayout->addWidget(m_checkBoxes[2]);
    m_areaWidget->setLayout(areaLayout);

    // Toggle checkboxes based on display mode
    bool enabled;
    if (m_glyph->getGlyphMode() == Glyph::kGUI) {
        enabled = false;
    }
    else {
        enabled = true;
    }
    m_checkBoxes[0]->setEnabled(enabled);
    m_checkBoxes[1]->setEnabled(enabled);

    m_area = new QScrollArea();
    m_area->setWidget(m_areaWidget);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void BillboardFlagsWidget::initializeConnections()
{
    // Flag options
    connect(m_checkBoxes[0], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_glyph->flags().setFlag(Glyph::kScale, checked);
    });

    connect(m_checkBoxes[1], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_glyph->flags().setFlag(Glyph::kFaceCamera, checked);
    });

    connect(m_checkBoxes[2], &QCheckBox::stateChanged,
        this,
        [&](int state) {
        bool checked = state == 0 ? false : true;
        m_glyph->flags().setFlag(Glyph::kAlwaysOnTop, checked);
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void BillboardFlagsWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_area);
}



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
    m_billboardFlags = new BillboardFlagsWidget(m_engine, glyph());
    
    m_glyphMode = new QComboBox();
    m_glyphMode->addItems({"Screen-space GUI", "World-space Billboard"});
    m_glyphMode->setCurrentIndex(glyph()->getGlyphMode());

    m_movementMode = new QComboBox();
    m_movementMode->addItems({"Manually Set", "Track Scene Object"});
    m_movementMode->setCurrentIndex(glyph()->moveMode());

    m_verticalAlignment = new QComboBox();
    m_verticalAlignment->addItems({"Top", "Middle", "Bottom"});
    m_verticalAlignment->setCurrentIndex(glyph()->verticalAlignment());
    m_verticalAlignment->setToolTip("Vertical Alignment");

    m_horizontalAlignment = new QComboBox();
    m_horizontalAlignment->addItems({ "Right", "Center", "Left" });
    m_horizontalAlignment->setCurrentIndex(glyph()->horizontalAlignment());
    m_horizontalAlignment->setToolTip("Horizontal Alignment");

    m_screenOffset = new VectorWidget<float, 2>(m_engine, glyph()->coordinates(), nullptr,  -1e7, 1e20, 5);

    std::shared_ptr<Transform> glyphTransform = glyph()->transform();
    m_transform = new TransformWidget(m_engine, glyphTransform.get());
    if (glyph()->moveMode() == Glyph::kTrackObject) {
        m_transform->setDisabled(true);
    }

    m_trackedSceneObject = new QLineEdit();
    m_trackedSceneObject->setToolTip("The name of the scene object to share a transform with this glyph");
    if (glyph()->trackedObject()) {
        m_trackedSceneObject->setText(glyph()->trackedObject()->getName());
    }
    if (glyph()->moveMode() == Glyph::kIndependent) {
        m_trackedSceneObject->setDisabled(true);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::initializeConnections()
{
    // Glyph mode
    connect(m_glyphMode, 
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        pauseSimulation();

        // Switch between screen-space and billboard modes
        Glyph::GlyphMode mode = Glyph::GlyphMode(idx);
        glyph()->setGlyphMode(mode);

        // Enable or disable glyph flags
        bool enabled;
        if (mode == Glyph::kGUI) {
            enabled = false;
        }
        else {
            enabled = true;
        }
        m_billboardFlags->m_checkBoxes[0]->setEnabled(enabled);
        m_billboardFlags->m_checkBoxes[1]->setEnabled(enabled);

        resumeSimulation();
    });

    // Movement mode
    connect(m_movementMode,
        qOverload<int>(&QComboBox::currentIndexChanged),
        this,
        [this](int idx) {
        pauseSimulation();

        Glyph::MovementMode mode = Glyph::MovementMode(idx);
        glyph()->setMoveMode(mode);

        // Replace transform widget
        switch (mode) {
        case Glyph::kIndependent:
        {
            // Create new transform for glyph
            glyph()->setTrackedObject(nullptr);
            const std::shared_ptr<Transform>& transform = glyph()->transform();

            // Replace transform widget's transform
            m_transform->setTransform(*transform);
            break;
        }
        case Glyph::kTrackObject:
        {
            // Set glyph transform based on current scene object name
            setGlyphTransform(m_trackedSceneObject->text());
            break; 
        }
        default:
            break;
        }

        resumeSimulation();
    });

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


    // Transform
    // On scene object select, transform changes,
    // so need to replace transform widget
    connect(m_trackedSceneObject,
        &QLineEdit::editingFinished,
        this,
        [this]() {
        pauseSimulation();

        m_engine->setGLContext();

        setGlyphTransform(m_trackedSceneObject->text());

        resumeSimulation();

    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void GlyphWidget::layoutWidgets()
{
    m_mainLayout = new QVBoxLayout;

    auto* flagsBox = new QGroupBox("Behavior Flags");
    auto* flagLayout = new QVBoxLayout();
    flagLayout->addWidget(m_billboardFlags);
    flagLayout->setSpacing(0);
    flagsBox->setToolTip("Flags for the behavior of this glyph");
    flagsBox->setLayout(flagLayout);
    m_mainLayout->addWidget(flagsBox);

    m_mainLayout->addLayout(LabeledLayout("Glyph Mode: ", m_glyphMode));
    m_mainLayout->addLayout(LabeledLayout("Movement Mode: ", m_movementMode));
    
    auto* alignBox = new QGroupBox("Alignment");
    auto* alignmentLayout = new QHBoxLayout();
    alignmentLayout->addWidget(m_horizontalAlignment);
    alignmentLayout->addWidget(m_verticalAlignment);
    alignmentLayout->setSpacing(0);
    alignBox->setToolTip("The alignment of the glyph in its bounding box");
    alignBox->setLayout(alignmentLayout);
    m_mainLayout->addWidget(alignBox);

    // Position-related
    auto* soBox = new QGroupBox("Screen Offset");
    auto* soLayout = new QVBoxLayout();
    soLayout->addWidget(m_screenOffset);
    soLayout->setSpacing(0);
    soBox->setToolTip("The offset of the glyph in screen space");
    soBox->setLayout(soLayout);

    auto* transformBox = new QGroupBox("World Transform");
    auto* transformLayout = new QVBoxLayout();
    transformLayout->addWidget(m_transform);
    transformLayout->setSpacing(0);
    transformBox->setToolTip("The transform of the glyph in world space");
    transformBox->setLayout(transformLayout);

    auto* posLayout = new QVBoxLayout();
    posLayout->addWidget(soBox);
    posLayout->addWidget(transformBox);
    posLayout->addLayout(
        LabeledLayout("Tracked Scene Object: ", m_trackedSceneObject));
    QGroupBox* positionControls = new QGroupBox();
    positionControls->setTitle("Position Controls");
    positionControls->setLayout(posLayout);

    m_mainLayout->addWidget(positionControls);
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
void GlyphWidget::setGlyphTransform(const QString& sceneObjectName)
{
    // If no scene object found with specified name
    std::shared_ptr<SceneObject> widgetObject =
        SceneObject::getByName(sceneObjectName);
    if (!widgetObject) {
        QMessageBox::warning(m_engine->mainWindow(),
            "Object Not Found",
            "Warning, no scene object found with the name '"
            + m_trackedSceneObject->text() + "'.");
        return;
    }

    // Set new transform for glyph
    std::shared_ptr<Transform> transform = S_CAST<Transform>(widgetObject->transform());
    glyph()->setTrackedObject(widgetObject);

    // Replace transform widget's transform
    m_transform->setTransform(*transform);
    //QLayoutItem* item = m_mainLayout->replaceWidget(m_transform,
    //    to);
    //delete item;
    //delete from;
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
    m_fontSize->setMaximumWidth(Renderable::screenDPIX() * 0.5);

    m_fontFace = new QComboBox();
    int count = 0;
    for (const auto& facePair : FontManager::FontFaces()) {
        const QString& key = facePair.first;
        m_fontFace->addItem(key);
        QFont font = QFont(key);
        m_fontFace->setItemData(count, font, Qt::FontRole);
        count++;
    }

    m_lineMaxSize = new QLineEdit();
    m_lineMaxSize->setValidator(new QDoubleValidator(0.1, 1e7, 7));
    m_lineMaxSize->setText(QString::number(label()->lineMaxSize()));

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
        QString icon = FontManager::faUnicodeCharacter(key);
        m_fontAwesomeIcon->addItem(icon + " " + key, key);
        count++;
    }
    m_fontAwesomeIcon->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_fontAwesomeIcon->setCurrentIndex(currentIndex);

    m_fontSize = new QLineEdit();
    m_fontSize->setValidator(new QDoubleValidator(0.1, 1e7, 2));
    m_fontSize->setText(QString::number(icon()->getFontSize()));
    m_fontSize->setMaximumWidth(Renderable::screenDPIX() * 0.5);

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
    m_canvas(canvas)
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
    for (std::shared_ptr<Glyph> glyph : glyphs) {
        addItem(glyph.get());
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::addItem(Glyph * glyph)
{
    // Create resource item
    GlyphItem* layerItem = new View::GlyphItem(glyph,
        (GlyphItem::GlyphItemType)(glyph->glyphType() + 2000));

    // Add resource item
    TreeWidget::addItem(layerItem);
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

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

    setMinimumSize(50, 100);
    //setMaximumSize(500, 800);

    initializeAsList();
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

    // Show widget
    GlyphWidget* widget;
    switch ((GlyphItem::GlyphItemType)(glyphItem->type())) {
    case GlyphItem::kLabel:
    {
        widget = new LabelWidget(m_engine, 
            dynamic_cast<Label*>(glyphItem->glyph()));
        break;
    }
    case GlyphItem::kIcon:
    {
        widget = new IconWidget(m_engine,
            dynamic_cast<Icon*>(glyphItem->glyph()));
        break;
    }
    default:
        throw("Error, unrecognized glyph item type");
    }
    widget->show();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CanvasGlyphWidget::initializeItem(QTreeWidgetItem * item)
{
    static_cast<GlyphItem*>(item)->setWidget();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
}
}
#ifndef GB_CANVAS_COMPONENT
#define GB_CANVAS_COMPONENT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// Qt
#include <QString>
#include <QJsonValue>
#include <QSize>

// Project
#include "GCameraComponent.h"
#include "../mixins/GRenderable.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Uniform;
class ModelComponent;
class ShaderProgram;
class Label;
class Icon;
class Glyph;
class CameraComponent;
class DrawCommand;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// See: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
enum class GlyphMode {
    kGUI = 0, // 2D elements on a menu, always in screen space
    kBillboard // 2D Element specified in world-space
};

/// @brief Describe options for billboard display
enum class BillboardFlag : unsigned int {
    kScale = 1, // Scale with zoom, i.e. if on, glyph is constant screen size
    kFaceCamera = 2, // Always face camera
    kAlwaysOnTop = 4, // Render on top of all components
};

/// @class CanvasComponent
class CanvasComponent: public Component {
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static Methods
    /// @{

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    CanvasComponent();
    CanvasComponent(const std::shared_ptr<SceneObject>& object);
    ~CanvasComponent();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{


    const std::vector<std::shared_ptr<Glyph>>& glyphs() const { return m_glyphs; }

    /// @brief Viewport settings
    //Viewport& viewport() { return m_viewport; }

    /// @brief Holds information about projection
    //RenderProjection& renderProjection() { return m_renderProjection; }

    /// @property Glyph Mode
    GlyphMode glyphMode() const { return m_glyphMode; }

    /// @brief Flags
    Flags<BillboardFlag>& flags() { return m_flags; }

    bool scalesWithDistance() const {
        return m_flags.testFlag(BillboardFlag::kScale);
    }
    void setScaleWithDistance(bool scale) {
        m_flags.setFlag(BillboardFlag::kScale, scale);
    }
    bool facesCamera() const {
        return m_flags.testFlag(BillboardFlag::kFaceCamera);
    }
    void setFaceCamera(bool face) {
        m_flags.setFlag(BillboardFlag::kFaceCamera, face);
    }
    bool alwaysOnTop() const {
        return m_flags.testFlag(BillboardFlag::kAlwaysOnTop);
    }
    void setOnTop(bool onTop) {
        m_flags.setFlag(BillboardFlag::kAlwaysOnTop, onTop);
    }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    void setGlyphMode(GlyphMode mode);

    /// @brief Create the draw commands for the canvas component
    void createDrawCommands(
        std::vector<std::shared_ptr<DrawCommand>>& outDrawCommands,
        SceneCamera& camera,
        ShaderProgram& shaderProgram,
        ShaderProgram* prepassProgram = nullptr);

    void addGlyph(const std::shared_ptr<Glyph>& glyph, bool setParent = true);
    void addGlyph(const std::shared_ptr<Glyph>& glyph, const std::shared_ptr<Glyph>& parent);

    void removeGlyph(const Glyph& glyph);
    void removeGlyph(const std::shared_ptr<Glyph>& glyph);

    QSize mainGLWidgetDimensions() const;

    /// @brief Resize viewport for the canvas component
    void resizeViewport();

    /// @brief Clear the canvas
    void clear();

    ///// @brief Draw the canvas with the given shader program
    //void draw(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    /// @note Limiting to one because it would be a poor design to allow more than one canvas per object
    virtual int maxAllowed() const override { return 1; }

    virtual void addRequiredComponents() override;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name GB object Properties
    /// @{
    /// @property className
    const char* className() const override { return "CanvasComponent"; }

    /// @property namespaceName
    const char* namespaceName() const override { return "rev::CanvasComponent"; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class Glyph;

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Get index of the specified glyph in the internal glyph vector
    size_t getIndex(const Glyph& glyph) const;

    /// @brief Reload all glyphs if screen dimensions changed
    // TODO: see if this is necessary
    void resize();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Most recently cached widget dimensions
    QSize m_dimensions;

    /// @brief The rendering mode of the glyph
    GlyphMode m_glyphMode;

    /// @brief The flags for the glyph
    Flags<BillboardFlag> m_flags;

    /// @brief Viewport settings, deprecated
    //  TODO: Enabling framebuffers with transparency to add GUI-only cameras
    //Viewport m_viewport;

    /// @brief Holds information about projection
    //RenderProjection m_renderProjection;

    /// @brief All of the glyphs on this canvas
    std::vector<std::shared_ptr<Glyph>> m_glyphs;

    /// @brief Vector of deleted glyph indices to avoid resizing vector on removal
    std::vector<size_t> m_deletedGlyphIndices;

    /// @}


};
Q_DECLARE_METATYPE(CanvasComponent)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing

#endif 

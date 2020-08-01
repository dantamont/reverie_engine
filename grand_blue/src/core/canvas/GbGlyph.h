/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_GLYPH_H
#define GB_GLYPH_H

// QT

// Internal
#include "../GbObject.h"
#include "../mixins/GbLoadable.h"
#include "../mixins/GbRenderable.h"
#include "../geometry/GbTransform.h"
#include "GbFonts.h"

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
class ShaderProgram;
class ResourceHandle;
class Mesh;
class VertexArrayData;
struct Uniform;
class CoreEngine;
class CanvasComponent;
class SceneObject;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @class Glyph
/// @brief Represents a 2-dimensional entity to display on a canvas
class Glyph : public Object, public Renderable {

public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum GlyphType {
        kLabel = 0,
        kIcon
    };

    // See: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
    enum GlyphMode {
        kGUI = 0, // 2D elements on a menu above everything else (for canvas)
        kBillboard // 2D Element specified in world-space, but rendered on top of everything
    };

    // TODO: Add a movement mode flag to move glyphs with the parent canvas
    // Would just need to parent to canvas transform (i.e. scene object transform)
    enum MovementMode {
        kIndependent, // Transform is set manually, default
        kTrackObject // Tracks a scene object
    };

    /// @brief Describe options for billboard display
    enum BillboardFlags: unsigned int {
        kScale = 1, // Scale with zoom, i.e. if on, glyph is constant screen size
        kFaceCamera = 2, // Always face camera
        kAlwaysOnTop = 4, // Render on top of all components
    };

    enum VerticalAlignment {
        kTop,
        kMiddle,
        kBottom
    };

    enum HorizontalAlignment {
        kRight,
        kCenter,
        kLeft
    };

    static std::shared_ptr<Glyph> createGlyph(CanvasComponent* canvas, const QJsonObject& object);

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{

    Glyph(CanvasComponent* canvas, GlyphType type, GlyphMode mode = kGUI);
	virtual ~Glyph();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Properties
	/// @{

    GlyphType glyphType() const { return m_type; }


    MovementMode moveMode() const { return m_moveMode; }
    void setMoveMode(MovementMode mode) { m_moveMode = mode; }

    const std::shared_ptr<Transform>& transform() { return m_transform; }

    QFlags<BillboardFlags>& flags() { return m_flags; }

    VerticalAlignment verticalAlignment() const { return m_verticalAlignment; }
    void setVerticalAlignment(VerticalAlignment alignment) { 
        m_verticalAlignment = alignment;
        reload();
    }

    HorizontalAlignment horizontalAlignment() const { return m_horizontalAlignment; }
    void setHorizontalAlignment(HorizontalAlignment alignment) { 
        m_horizontalAlignment = alignment;
        reload();
    }

    void setAlignment(VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment);

    Vector2g& coordinates() { return m_coordinates; }
    const Vector2g& coordinates() const { return m_coordinates; }
    void setCoordinates(const Vector2g& coordinates) {
        m_coordinates = coordinates;
    }

    /// @property Glyph Mode
    GlyphMode getGlyphMode() const { return m_glyphMode; }
    void setToBillboard() { m_glyphMode = kBillboard; }
    void setToGui() { m_glyphMode = kGUI; }
    void setGlyphMode(GlyphMode mode) { m_glyphMode = mode; }

    CanvasComponent* canvas() const { return m_canvas; }

    /// @brief Flags
    bool scalesWithDistance() const {
        return m_flags.testFlag(kScale);
    }
    void setScaleWithDistance(bool scale) {
        m_flags.setFlag(kScale, scale);
    }
    bool facesCamera() const {
        return m_flags.testFlag(kFaceCamera);
    }
    void setFaceCamera(bool face) {
        m_flags.setFlag(kFaceCamera, face);
    }
    bool onTop() const {
        return m_flags.testFlag(kAlwaysOnTop);
    }
    void setOnTop(bool onTop) {
        m_flags.setFlag(kAlwaysOnTop, onTop);
    }

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Update the glyph data
    virtual void reload() override {}

    /// @brief Return the screen (GL widget) position of the glyph
    //Vector4g screenPos() const;
    
    /// @brief Set the scene object for this glyph to track
    const std::shared_ptr<SceneObject> trackedObject() const {
        if (const std::shared_ptr<SceneObject>& obj = m_trackedObject.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }
    void setTrackedObject(const std::shared_ptr<SceneObject>& object);

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void preDraw() override;

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Ensures that this glyph has a valid transform
    void checkTransform();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Type of glyph
    GlyphType m_type;

    /// @brief The rendering mode of the glyph
    GlyphMode m_glyphMode;

    VerticalAlignment m_verticalAlignment;
    HorizontalAlignment m_horizontalAlignment;

    /// @brief The flags for the glyph
    QFlags<BillboardFlags> m_flags;

    MovementMode m_moveMode;

    /// @brief The transform of the glyph
    std::shared_ptr<Transform> m_transform;

    /// @brief GUI mode: Screen-coordinate offset for the glyph, with bottom-left of screen at (0, 0)
    /// @details The z-value representes the world-coordinate z-value of the glyph
    Vector2g m_coordinates;

    /// @brief The name of the scene object that this is glyph sharing a transform with
    /// @details Is set at load time, so may not accurately reflect scene object name changes
    std::weak_ptr<SceneObject> m_trackedObject;

    /// @brief Pointer back to the canvas component
    CanvasComponent* m_canvas;

    /// @brief Map of uniforms to restore on releaseUniforms call
    std::vector<Uniform> m_uniformCache;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
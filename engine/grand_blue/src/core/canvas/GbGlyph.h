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
        kLabel,
        kIcon
    };

    // See: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
    enum GlyphMode {
        kGUI = 0, // 2D elements on a menu above everything else (for canvas)
        kBillboard // 2D Element specified in world-space, but rendered on top of everything
    };

    /// @brief Describe options for billboard display
    enum BillboardFlags: unsigned int {
        kScale = 1, // Scale with zoom
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

    void setAlignment(VerticalAlignment verticalAlignment, HorizontalAlignment horizontalAlignment);

    const Vector2g& coordinates() const { return m_coordinates; }
    void setCoordinates(const Vector2g& coordinates) {
        m_coordinates = coordinates;
    }

    /// @property Glyph Mode
    GlyphMode getGlyphMode() const { return m_mode; }
    void setToBillboard() { m_mode = kBillboard; }
    void setToGui() { m_mode = kGUI; }
    void setGlyphMode(GlyphMode mode) { m_mode = mode; }

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

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    virtual void releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures() override {}

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures() override {}

    const std::shared_ptr<SceneObject> sceneObject() const{
        if (const std::shared_ptr<SceneObject>& obj = m_object.lock()) {
            return obj;
        }
        else {
            return nullptr;
        }
    }

    /// @brief Ensures that this glyph has a valid transform
    void checkTransform();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Type of glyph
    GlyphType m_type;

    /// @brief The rendering mode of the glyph
    GlyphMode m_mode;

    VerticalAlignment m_verticalAlignment;
    HorizontalAlignment m_horizontalAlignment;

    /// @brief The flags for the glyph
    QFlags<BillboardFlags> m_flags;

    QString m_objectName;

    /// @brief The transform of the glyph
    std::shared_ptr<Transform> m_transform;

    /// @brief GUI mode: Screen-coordinates for the glyph, with bottom-left of screen at (0, 0)
    /// @details The z-value representes the world-coordinate z-value of the glyph
    Vector2g m_coordinates;

    /// @brief The name of the scene object that this is glyph sharing a transform with
    /// @details Is set at load time, so may not accurately reflect scene object name changes
    std::weak_ptr<SceneObject> m_object;

    /// @brief Pointer back to the canvas component
    CanvasComponent* m_canvas;

    /// @brief Map of uniforms to restore on releaseUniforms call
    std::vector<Uniform> m_uniformCache;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
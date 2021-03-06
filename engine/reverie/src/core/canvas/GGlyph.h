/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_GLYPH_H
#define GB_GLYPH_H

// QT

// Internal
#include "../GObject.h"
#include "../mixins/GLoadable.h"
#include "../mixins/GRenderable.h"
#include "../geometry/GTransform.h"

namespace rev {


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
class Glyph : public Object, public Identifiable, public Renderable {

public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum GlyphType {
        kLabel = 0,
        kIcon,
        kImage
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

    Glyph(CanvasComponent* canvas);
	virtual ~Glyph();

	/// @}

	//--------------------------------------------------------------------------------------------
	/// @name Properties
	/// @{

    virtual GlyphType glyphType() const = 0;

    const Transform& transform() const { return m_transform; }
    Transform& transform() { return m_transform; }

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


    CanvasComponent* canvas() const { return m_canvas; }

	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Return glyph as a subclass
    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Glyph, T>, "Error, can only convert to glyph type");
        return dynamic_cast<T*>(this);
    }

    /// @brief Update the glyph data
    virtual void reload() override {}

    Glyph* getParent() const;
    void setParent(int idx);
    void setParent(Glyph* glyph);
    void setParent(CanvasComponent* canvas);

    /// @brief Return the screen (GL widget) position of the glyph
    //Vector4g screenPos() const;

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    virtual void preDraw() override;

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Pointer back to the canvas component
    CanvasComponent* m_canvas;

    VerticalAlignment m_verticalAlignment;
    HorizontalAlignment m_horizontalAlignment;

    /// @brief The index of the parent to this glyph in the canvas glyph vector. 
    /// @note If negative, has no parent
    int m_parentIndex = -1;

    /// @brief The transform of the glyph
    Transform m_transform;

    /// @brief Map of uniforms to restore on releaseUniforms call
    std::vector<Uniform> m_uniformCache;

    /// @}
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
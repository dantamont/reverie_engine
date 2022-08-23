#pragma once

// QT

// Internal
#include "fortress/types/GLoadable.h"
#include "core/mixins/GRenderable.h"
#include "fortress/containers/math/GTransform.h"
#include "core/rendering/buffers/GUniformBufferObject.h"

namespace rev {

class ShaderProgram;
class ResourceHandle;
class Mesh;
class VertexArrayData;
class Uniform;
class CoreEngine;
class CanvasComponent;
class SceneObject;

/// @brief Indices of the uniforms for this glyph in the governing canvas component
struct CanvasUniformIndices {
    static constexpr Uint32_t s_invalidIndex = Uint32_t(-1);
    Uint32_t m_worldMatrix{ s_invalidIndex }; ///< The canvas index of the world matrix for the glyph
    Uint32_t m_textureOffset{ s_invalidIndex }; ///< The canvas index of the two dimensional offset for the glyph's texture
    Uint32_t m_textureScale{ s_invalidIndex }; ///< The canvas index of the two-dimensional scaling for the glyph's texture
    Uint32_t m_textColor{ s_invalidIndex }; ///< The canvas index of the text color, used for labels
};

/// @class Glyph
/// @brief Represents a 2-dimensional entity to display on a canvas
class Glyph : public IdentifiableInterface, public Renderable {
public:
    /// @name Static
    /// @{

    enum GlyphType {
        kLabel = 0,
        kIcon,
        kSprite
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

    static std::shared_ptr<Glyph> CreateGlyph(CanvasComponent* canvas, const json& korJson);

    /// @brief Create a glyph of the specified type
    template<typename GlyphType, typename ...Args>
    static std::shared_ptr<GlyphType> Create(CanvasComponent* canvas, const Args&... args)
    {
        auto engine = canvas->sceneObject()->scene()->engine();

        // Set OpenGL context to main context to avoid VAO struggles
        engine->setGLContext();

        // Create world matrix uniform
        RenderContext& context = engine->openGlRenderer()->renderContext();
        UniformContainer& uc = context.uniformContainer();
        CanvasUniformIndices indices;
        indices.m_worldMatrix = canvas->m_glyphUniforms.m_worldMatrix.push(Matrix4x4::Identity(), uc);
        const UniformData& uniformData = canvas->m_glyphUniforms.m_worldMatrix[indices.m_worldMatrix];
#ifdef DEBUG_MODE
        assert(!uniformData.isEmpty() && "Invalid uniform data");
#endif
        std::vector<Matrix4x4>& matrixUniformVector = uc.getUniformVector<Matrix4x4>().m_uniforms;

        auto glyph = prot_make_shared<GlyphType>(canvas, matrixUniformVector, uniformData.m_storageIndex, args...);
        glyph->postConstruct(indices);
        return glyph;
    }

    /// @}

	/// @name Constructors/Destructor
	/// @{

    Glyph(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex);
	virtual ~Glyph();

	/// @}

	/// @name Public methods
	/// @{

    /// @brief Indices of the uniforms for this glyph in the parent canvas
    const CanvasUniformIndices& canvasUniformIndices() const { return m_canvasIndices; }

    virtual GlyphType glyphType() const = 0;

    const IndexedTransform& transform() const { return m_transform; }
    IndexedTransform& transform() { return m_transform; }

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

    /// @brief Return glyph as a subclass
    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Glyph, T>, "Error, can only convert to glyph type");
        return dynamic_cast<T*>(this);
    }

    /// @brief Update the glyph data
    virtual void reload() override {}

    /// @brief Return glyph as JSON, casting to appropriate subclass before serialization
    json asJson() const;

    Glyph* getParent() const;
    void setParent(int idx);
    void setParent(Glyph* glyph);
    void setParent(CanvasComponent* canvas);

    void postConstruct(const CanvasUniformIndices& indices);

    /// @brief Return the screen (GL widget) position of the glyph
    //Vector4g screenPos() const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Glyph& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Glyph& orObject);


    /// @}

protected:
    /// @name Protected Methods
    /// @{

    virtual void preDraw() override;

    /// @brief Set uniforms for the glyph
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;

    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @}

    /// @name Protected members
    /// @{

    CanvasUniformIndices m_canvasIndices; ///< The indices of the uniforms for this glyph in the governing canvas component

    CanvasComponent* m_canvas{ nullptr }; ///< Pointer back to the canvas component

    VerticalAlignment m_verticalAlignment;
    HorizontalAlignment m_horizontalAlignment;

    /// @note If negative, has no parent
    int m_parentIndex = -1; ///< The index of the parent to this glyph in the canvas glyph vector. 
    IndexedTransform m_transform; ///< The transform of the glyph
    std::vector<BufferUniformData> m_cameraBufferUniformCache; ///< Map of uniforms to restore on releaseUniforms call

    /// @}
};

} // End namespaces

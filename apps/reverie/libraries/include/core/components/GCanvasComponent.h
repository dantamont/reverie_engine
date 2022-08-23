#pragma once

// Qt
#include <QString>
#include <QJsonValue>
#include <QSize>

// Project
#include "fortress/containers/GStrictGrowContainer.h"
#include "GCameraComponent.h"
#include "core/mixins/GRenderable.h"

namespace rev {

class Uniform;
class ModelComponent;
class ShaderProgram;
class Label;
class Icon;
class Glyph;
class CameraComponent;
class DrawCommand;

/// @see http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
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

    /// @name Constructors/Destructor
    /// @{
    CanvasComponent();
    CanvasComponent(const std::shared_ptr<SceneObject>& object);
    ~CanvasComponent();

    /// @}

    /// @name Public Methods
    /// @{

    const std::vector<std::shared_ptr<Glyph>>& glyphs() const { return m_glyphs; }

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

    /// @brief  Obtain the glyph with the matching Uuid
    template<typename T>
    T* getGlyph(const Uuid& uuid) {
        return static_cast<T*>(getGlyph(uuid));
    }
    Glyph* getGlyph(const Uuid& uuid) const;

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

    virtual void addRequiredComponents(std::vector<Uuid>& outDependencyIds, std::vector<json>& outDependencies) override;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const CanvasComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, CanvasComponent& orObject);


    /// @}

protected:
    friend class Glyph;
    friend class Label;
    friend class Sprite;

    /// @name Protected Methods
    /// @{

    /// @brief Get index of the specified glyph in the internal glyph vector
    uint32_t getIndex(const Glyph& glyph) const;

    /// @brief Reload all glyphs if screen dimensions changed
    // TODO: see if this is necessary
    void resize();

    const UniformData& orthoProjectionBufferUniformData() const {
        return m_orthoProjectionBufferUniformData;
    }

    const UniformData& viewMatrixBufferUniformData() const {
        return m_viewMatrixBufferUniformData;
    }

    /// @}

    /// @name Protected Members
    /// @{

    /// @brief Uniform data for all glyphs on the canvas
    struct GlyphUniformData {
        /// @todo Update when camera buffer updates instead of in Glyph::bindUniforms call
        UniformData m_perspectiveInverseScale; ///< Common to all glyphs. The inverse of the top-left 3x3 of the projection matrix
        UniformData m_onTop; ///< Common to all glyphs. Whether or not to always render on top
        UniformData m_faceCamera; ///< Common to all glyphs. Whether or not to always face the camera
        UniformData m_scaleWithDistance; ///< Common to all glyphs. Whether or not to scale with distance from the camera
        StrictGrowContainer<std::vector<UniformData>> m_worldMatrix; ///< The world matrix for the glyph
        StrictGrowContainer<std::vector<UniformData>> m_textureOffset; ///< The two dimensional offset for the glyph's texture
        StrictGrowContainer<std::vector<UniformData>> m_textureScale; ///< The two-dimensional scaling for the glyph's texture
        StrictGrowContainer<std::vector<UniformData>> m_textColor; ///< The text color, used for labels
        std::array<UniformData, (Uint32_t)TextureUsageType::kMAX_TEXTURE_TYPE> m_textureUnits; ///< Common to all glyphs. Texture units to use for each texture type
    };
    GlyphUniformData m_glyphUniforms; ///< The uniform data for this canvas

    QSize m_dimensions; ///< Most recently cached widget dimensions
    GlyphMode m_glyphMode; ///< The rendering mode of the glyphs
    Flags<BillboardFlag> m_flags; ///< The flags for the glyphs
    std::vector<std::shared_ptr<Glyph>> m_glyphs; ///< All of the glyphs on this canvas
    std::vector<size_t> m_deletedGlyphIndices; ///< Vector of deleted glyph indices to avoid resizing vector on removal
    UniformData m_orthoProjectionBufferUniformData; ///< Non-static to initialize after uniform container
    UniformData m_viewMatrixBufferUniformData; ///< Non-static to initialize after uniform container

    /// @}


};

} // end namespacing


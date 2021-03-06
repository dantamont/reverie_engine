/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_RENDERABLE_H
#define GB_RENDERABLE_H

// Standard
#include <map>

// QT
#include <QString>

// Internal
#include "GLoadable.h"
#include "../rendering/renderer/GRenderSettings.h"
#include "../containers/GContainerExtensions.h"
#include "../containers/GStringView.h"
#include "../resource/GResource.h"
#include "../geometry/GCollisions.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class ShaderProgram;
struct Uniform;
class RenderContext;
class Transform;
class Mesh;
class Material;
class ResourceCache;
class DrawCommand;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class Shadable
/// @brief Class representing an object that carries uniforms
class Shadable : public Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Shadable() {}
    virtual ~Shadable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    const std::vector<Uniform>& uniforms() const { return m_uniforms; }
    std::vector<Uniform>& uniforms() { return m_uniforms; }

    RenderSettings& renderSettings() { return m_renderSettings; }
    const RenderSettings& renderSettings() const { return m_renderSettings; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Add a uniform to the uniforms to be set
    void addUniform(const Uniform& uniform);

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(const GStringView& uniformName, int* outIndex = nullptr);
    bool hasUniform(const Uniform& uniform, int* outIndex = nullptr);

    /// @brief Clear all uniforms
    inline void clearUniforms() { m_uniforms.clear(); }

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

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Uniforms corresponding to this renderable
    std::vector<Uniform> m_uniforms;

    /// @brief GL Render settings corresponding to this renderable
    RenderSettings m_renderSettings;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Flags relating to ignoring render settings during draw pass
enum class RenderableIgnoreFlag {
    kIgnoreSettings = 1 << 0, // avoid binding render settings
    kIgnoreTextures = 1 << 1, // skip texture binding
    kIgnoreUniforms = 1 << 2, // skip uniform binding
    kIgnoreUniformMismatch = 1 << 3, // ignore errors raised by unrecognized uniforms
};

/// @brief Flags related to render passes
enum class RenderablePassFlag {
    // Flag is set if world bounds are ignored since geometry is determined at draw time.
    // If this flag is set, unique color IDs are used for mouse picking calculations
    kDeferredGeometry = 1 << 0 // Set automatically for glyphs
};

/// @brief Renderable types
// Was thinking of doing this to delineate between renderables for mouse picking
//enum class RenderableType {
//    kModelChunk,
//    kGlyph
//};

/// @class Renderable
/// @brief Class representing an object that generates a draw item
/// @details This is not necessarily an "instantiation" of something that is drawable, as a renderable
/// object may be drawn multiple times in a render pass with different transforms
class Renderable: public Shadable{
public:
    typedef Flags<RenderableIgnoreFlag> IgnoreFlags;
    typedef Flags<RenderablePassFlag> PassFlags;

    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{
    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Renderable();
    virtual ~Renderable();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief The mesh associated with this renderable
    Mesh* mesh() const;

    /// @brief The material associated with this renderable
    Material* material() const;

    const std::shared_ptr<ResourceHandle>& meshHandle() const { return m_meshHandle; }
    const std::shared_ptr<ResourceHandle>& materialHandle() const { return m_matHandle; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set uniforms in the given draw command from this renderable
    /// @note This is used by the Lines and Points renderables for debug rendering
    virtual void setUniforms(DrawCommand& drawCommand) const;

    /// @brief Draw the renderable given a shader program
    virtual void draw(ShaderProgram& shaderProgram, RenderContext* context = nullptr,
        RenderSettings* settings = nullptr, 
        size_t drawFlags = 0);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    /// @details Should generate bounding boxes in this routine
    virtual void reload();

    /// @brief Verify that the renderable is valid
    //virtual void verify() const;

    /// @brief Obtain the world-space bounds of the renderable given a transform
    void addWorldBounds(const Transform& world, BoundingBoxes& outBounds) const;

    /// @brief Get ID to sort renderable
    virtual size_t getSortID() = 0;

    /// @brief Initialize an empty mesh handle for the renderable
    void initializeEmptyMesh(ResourceCache& cache, ResourceBehaviorFlags flags);

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

    /// @brief Miscellaneous actions to be performed before drawing
    /// @details Is a good place for any viewport changes to be made
    virtual void preDraw(){}

    /// @brief Set uniforms for the renderable in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram);
    virtual void releaseUniforms(ShaderProgram& shaderProgram);

    /// @brief Bind the textures used by this renderable
    virtual void bindTextures(ShaderProgram* shaderProgram = nullptr, RenderContext* context = nullptr);

    /// @brief Release the textures used by this renderable
    virtual void releaseTextures(ShaderProgram* shaderProgram = nullptr, RenderContext* context = nullptr);

    /// @brief Draw geometry associated with this renderable
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) 
    {
        Q_UNUSED(settings);
        Q_UNUSED(shaderProgram);
    }

    /// @brief Print GL error
    void printError(const GStringView& error);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief The mesh associated with the renderable
    std::shared_ptr<ResourceHandle> m_meshHandle;

    /// @brief The material associated with the renderable
    std::shared_ptr<ResourceHandle> m_matHandle;

    /// @}

};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
#pragma once 

// Standard
#include <map>

// QT
#include <QString>

// Internal
#include "fortress/types/GLoadable.h"
#include "core/rendering/renderer/GRenderSettings.h"
#include "fortress/containers/GContainerExtensions.h"
#include "fortress/types/GStringView.h"
#include "core/resource/GResource.h"
#include "core/geometry/GCollisions.h"

namespace rev {

class ShaderProgram;
class Uniform;
class RenderContext;
class Mesh;
class Material;
class ResourceCache;
class DrawCommand;

class TransformInterface;
template<typename WorldMatrixType>
class TransformTemplate;
typedef TransformTemplate<Matrix4x4> Transform;

/// @class Shadable
/// @brief Class representing an object that carries uniforms
class Shadable  {
public:
    /// @name Constructors/Destructor
    /// @{
    Shadable() {}
    virtual ~Shadable() {}
    /// @}

    /// @name Public methods
    /// @{

    const std::vector<Uniform>& uniforms() const { return m_uniforms; }

    const RenderSettings& renderSettings() const { return m_renderSettings; }
    void setRenderSettings(const RenderSettings& settings) { m_renderSettings = settings; }

    /// @brief Add a uniform to the uniforms to be set
    template<typename ValueType>
    void addUniform(Uint32_t id, const ValueType& value) {
        addUniform(Uniform(id, value));
    }
    void addUniform(const Uniform& uniform);

    /// @brief Whether or not the command has the specified uniform
    bool hasUniform(Uint32_t uniformId, int* outIndex = nullptr);
    bool hasUniform(const Uniform& uniform, int* outIndex = nullptr);

    /// @brief Clear all uniforms
    inline void clearUniforms() { m_uniforms.clear(); }

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Shadable& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Shadable& orObject);


    /// @}

protected:

    /// @name Protected Members
    /// @{

    std::vector<Uniform> m_uniforms; ///< Uniforms corresponding to this renderable
    RenderSettings m_renderSettings; ///< GL Render settings corresponding to this renderable

    /// @}

};


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

/// @class Renderable
/// @brief Class representing an object that generates a draw item
/// @details This is not necessarily an "instantiation" of something that is drawable, as a renderable
/// object may be drawn multiple times in a render pass with different transforms
class Renderable: public Shadable{
public:
    typedef Flags<RenderableIgnoreFlag> IgnoreFlags;
    typedef Flags<RenderablePassFlag> PassFlags;

	/// @name Constructors/Destructor
	/// @{
    Renderable();
    virtual ~Renderable();
	/// @}

    /// @name Properties
    /// @{

    /// @brief Return as subclass of the renderable
    template<typename T>
    T* as() {
        static_assert(std::is_base_of_v<Renderable, T>, "Error, can only convert to renderable type");
        return dynamic_cast<T*>(this);
    }

    /// @brief The mesh associated with this renderable
    Mesh* mesh() const;

    /// @brief The material associated with this renderable
    Material* material() const;

    const std::shared_ptr<ResourceHandle>& meshHandle() const { return m_meshHandle; }
    const std::shared_ptr<ResourceHandle>& materialHandle() const { return m_matHandle; }

    /// @}
	
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
    void addWorldBounds(const TransformInterface& world, BoundingBoxes& outBounds) const;

    /// @brief Get ID to sort renderable
    virtual size_t getSortID() = 0;

    /// @brief Initialize an empty mesh handle for the renderable
    void initializeEmptyMesh(ResourceCache& cache, ResourceBehaviorFlags flags);

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Renderable& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Renderable& orObject);


    /// @}

protected:
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
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr);

    /// @brief Print GL error
    void printError(const GStringView& error);

    /// @}

    /// @name Protected Members
    /// @{

    std::shared_ptr<ResourceHandle> m_meshHandle; ///< The mesh associated with the renderable
    std::shared_ptr<ResourceHandle> m_matHandle; ///< The material associated with the renderable

    /// @}

};



} // End namespaces

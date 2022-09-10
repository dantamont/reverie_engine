#pragma once

// Internal
#include "core/mixins/GRenderable.h"
#include "fortress/layer/framework/GFlags.h"
#include "core/rendering/geometry/GVertexData.h"

namespace rev {

class Color;
class Mesh;
class ResourceCache;
class RenderContext;
enum class ResourceBehaviorFlag;

/// @brief Class representing set of line geometry
/// @see https://mattdesl.svbtle.com/drawing-lines-is-hard
/// @see https://blog.mapbox.com/drawing-antialiased-lines-with-opengl-8766f34192dc
class Lines: public Renderable{
public:
    /// @name Static
    /// @{

    enum class MeshGenerationFlag {
        kTriangulate = 1 << 0 // Whether or not to generate triangles from the points given
    };
    typedef Flags<MeshGenerationFlag> MeshGenerationFlags;

    /// @brief Obtain lines to render an arbitrary shape
    static std::shared_ptr<Lines> GetShape(RenderContext& context, const std::vector<Vector3>& meshPoints, const std::vector<Uint32_t>& indices, ResourceBehaviorFlags handleFlags = 0);
    
    static std::shared_ptr<Lines> GetCube(ResourceCache& cache, ResourceBehaviorFlags handleFlags = 0);
    static std::shared_ptr<Lines> GetSphere(ResourceCache& cache, int latSize = 20, int lonSize = 30, ResourceBehaviorFlags handleFlags = 0);
    static std::shared_ptr<Lines> GetGridCube(ResourceCache& cache, float spacing = 1.0f, int numHalfSpaces = 10, ResourceBehaviorFlags handleFlags = 0);
    static std::shared_ptr<Lines> GetPlane(ResourceCache& cache, float spacing = 1.0f, int numHalfSpaces = 10, ResourceBehaviorFlags handleFlags = 0);
    static std::shared_ptr<Lines> GetPrism(ResourceCache& cache, float baseRadius = 1.0, float topRadius = 1.0,
        float height = 1.0, int sectorCount = 36, int stackCount = 1, ResourceBehaviorFlags handleFlags = 0);
    static std::shared_ptr<Lines> GetCapsule(ResourceCache& cache, float radius = 1.0, float halfHeight = 1.0, ResourceBehaviorFlags handleFlags = 0);

    enum ShapeOptions {
        kUseMiter = 1, //Whether or not to use miter joins on line segments(makes lines prettier, with minimal cost)
        kConstantScreenThickness = 2 // Whether or not to use perspective divide on line thickness
    };

    enum EffectOptions {
        kFadeWithDistance = 1
    };

    /// @brief Settings to toggle for a line
    //struct LineSettings {
    //    ShapeOptions m_shapeOptions;
    //    EffectOptions m_effectOptions;
    //    Float32_t m_lineThickness;
    //    Vector4 m_lineColor;
    //};

    /// @}

	/// @name Constructors/Destructor
	/// @{
    virtual ~Lines();
	/// @}

    /// @name Properties
    /// @{

    bool useMiter() const {
        return m_shapeOptions.testFlag(kUseMiter);
    }

    void setUseMiter(bool useMiter, UniformContainer& uc);

    void setConstantScreenThickness(bool constant, UniformContainer& uc);

    void setFadeWithDistance(bool fade, UniformContainer& uc);

    float lineThickness() const { return m_lineThickness; }
    void setLineThickness(float thickness, UniformContainer& uc);

    const Vector4& lineColor() const { return m_lineColor; }
    void setLineColor(const Vector4& lineColor, UniformContainer& uc);

    /// @}

    /// @name Public Methods
	/// @{

    /// @brief Set uniform values, using the given container
    void initializeUniformValues(UniformContainer& uc);

    /// @brief Set uniforms in the given draw command from this renderable
    /// @note This is used by the Lines and Points renderables for debug rendering
    virtual void setUniforms(DrawCommand& drawCommand) const override;

    /// @brief Load an existing vao as a Lines object
    void loadVertexData(ResourceCache& cache, const MeshVertexAttributes& data, Flags<ResourceBehaviorFlag> flags, MeshGenerationFlags meshFlags = 0);
    
    /// @brief Load a vector of points as a Lines object
    void loadPointData(ResourceCache& cache, const std::vector<Vector3>& data, const std::vector<Uint32_t>& indices, Flags<ResourceBehaviorFlag> flags, MeshGenerationFlags meshFlags = 0);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() override;

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

	/// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const MeshGenerationFlag& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, MeshGenerationFlag& orObject);


    /// @}

protected:
    friend class Points;

    /// @name Protected Methods
    /// @{

    Lines(UniformContainer& uc);

    /// @brief Set uniforms for the Lines in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;
    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry associated with this Lines
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(const Vector3& newPoint);
    void addPoint(MeshVertexAttributes& vertexData, const Vector3& newPoint);

    /// @brief Add a triangle from three points
    void addTriangle(const Vector3& p1, const Vector3& p2, const Vector3& p3);


    /// @}

    /// @name Protected Members
    /// @{

    /// @todo Eliminate redundant members, since uniforms store data
    struct LineUniformData {
        UniformData m_hasConstantScreenThickness;
        UniformData m_lineColor;
        UniformData m_lineThickness;
        UniformData m_useMiter;
        UniformData m_fadeWithDistance;
    };

    LineUniformData m_uniforms;
    MeshVertexAttributes m_vertexData; ///< Vertex data for the lines. Duplicated on GPU
    float m_lineThickness; ///< Line thickness (in NDC)
    Flags<ShapeOptions> m_shapeOptions;
    Flags<EffectOptions> m_effectOptions;
    Vector4 m_lineColor; ///< Line color

    /// @}

};


} // End namespaces

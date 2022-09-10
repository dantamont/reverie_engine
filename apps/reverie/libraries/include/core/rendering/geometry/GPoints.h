#pragma once

// Internal
#include "core/mixins/GRenderable.h"
#include "core/rendering/geometry/GVertexData.h"

namespace rev {

class Mesh;
class Lines;
class Skeleton;
class ResourceCache;
class UniformContainer;

/// @brief Class representing set of point geometry
class Points: public Renderable{
public:

	/// @name Constructors/Destructor
	/// @{
    Points(UniformContainer& uc);
    Points(ResourceCache& cache, const Skeleton& skeleton, Flags<ResourceBehaviorFlag> flags);
    Points(ResourceCache& cache, size_t numPoints, Flags<ResourceBehaviorFlag> flags);
    Points(const Lines& lines);
    virtual ~Points();
	/// @}

    /// @name Properties
    /// @{

    /// @brief Normalized screen-space point size
    float pointSize() const { return m_pointSize; }
    void setPointSize(float size, UniformContainer& uc);

    const Vector4& pointColor() const { return m_pointColor; }
    void setPointColor(const Vector4& color, UniformContainer& uc);

    /// @}

    /// @name Public Methods
	/// @{

    /// @brief Set uniforms in the given draw command from this renderable
    /// @note This is used by the Lines and Points renderables for debug rendering
    virtual void setUniforms(DrawCommand& drawCommand) const override;

    /// @brief The number of points
    size_t numPoints() const;

    /// @brief Load an existing vao as a Points object
    void loadVertexData(ResourceCache& cache, const MeshVertexAttributes& data, Flags<ResourceBehaviorFlag> flags);

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
    friend void to_json(nlohmann::json& orJson, const Points& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Points& orObject);


    /// @}

protected:
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the Points in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;
    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry associated with this Points
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(MeshVertexAttributes& vertexData, const Vector3& newPoint);

    /// @}

    /// @name Protected Members
    /// @{

    struct PointUniforms {
        UniformData m_pointSize; ///< Point size
        UniformData m_pointColor; ///< Color of points
        mutable UniformData m_screenPixelWidth; ///< Float of screen pixel width
    };

    PointUniforms m_uniforms;
    MeshVertexAttributes m_vertexData; ///< Vertex data for the points. Duplicated on GPU
    float m_pointSize; ///< Point size
    Vector4 m_pointColor; ///< Point color

    /// @}

};


} // End namespaces

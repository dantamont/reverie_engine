/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_POINTS_H
#define GB_POINTS_H

// Internal
#include "../../mixins/GRenderable.h"
#include "../../GObject.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
struct VertexAttributes;
class Mesh;
class Transform;
class Lines;
class Skeleton;
class ResourceCache;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class representing set of point geometry
class Points: public Object, public Renderable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    //enum class ShapeOptions {
    //};

    //enum class EffectOptions {
    //};

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Points();
    Points(ResourceCache& cache, const Skeleton& skeleton, Flags<ResourceBehaviorFlag> flags);
    Points(ResourceCache& cache, size_t numPoints, Flags<ResourceBehaviorFlag> flags);
    Points(const Lines& lines);
    virtual ~Points();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Normalized screen-space point size
    float pointSize() const { return m_pointSize; }
    void setPointSize(float size) { m_pointSize = size; }

    const Vector4& pointColor() const { return m_pointColor; }
    void setPointColor(const Vector4& color) { m_pointColor = color; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set uniforms in the given draw command from this renderable
    /// @note This is used by the Lines and Points renderables for debug rendering
    virtual void setUniforms(DrawCommand& drawCommand) const override;

    /// @brief The number of points
    size_t numPoints() const;

    /// @brief Load an existing vao as a Points object
    void loadVertexArrayData(ResourceCache& cache, const VertexArrayData& data, Flags<ResourceBehaviorFlag> flags);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() override;

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

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

    /// @brief Set uniforms for the Points in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;
    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry associated with this Points
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(VertexArrayData& vertexData, const Vector3& newPoint);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Point size
    float m_pointSize;

    /// @brief Point color
    Vector4 m_pointColor;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
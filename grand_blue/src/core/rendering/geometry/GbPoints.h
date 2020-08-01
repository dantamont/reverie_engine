/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_POINTS_H
#define GB_POINTS_H

// Internal
#include "../../mixins/GbRenderable.h"
#include "../../GbObject.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
struct VertexAttributes;
class Mesh;
class Transform;
class Lines;
class Skeleton;

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
    Points(const Skeleton& skeleton);
    Points(size_t numPoints);
    Points(const Lines& lines);
    virtual ~Points();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @brief Normalized screen-space point size
    float pointSize() const { return m_pointSize; }
    void setPointSize(float size) { m_pointSize = size; }

    const Vector4g& pointColor() const { return m_pointColor; }
    void setPointColor(const Vector4g& color) { m_pointColor = color; }

    Transform& transform() { return *m_transform; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief The number of points
    size_t numPoints() const;

    /// @brief Load in a mesh as a Points object
    void loadMesh(std::shared_ptr<Mesh> mesh);

    /// @brief Load an existing vao as a Points object
    void loadVertexArrayData(const VertexArrayData& data);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() override;

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

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

    /// @brief Set uniforms for the Points in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;
    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry associated with this Points
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(const Vector3g& newPoint);
    void addPoint(std::shared_ptr<VertexArrayData> vertexData, const Vector3g& newPoint);

    /// @brief Add a new line to the Points
    void addPointSet();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Geometry data corresponding to each point group
    std::vector<std::shared_ptr<VertexArrayData>> m_vertexData;

    /// @brief Point size
    float m_pointSize;

    std::unique_ptr<Transform> m_transform;

    //QFlags<ShapeOptions> m_shapeOptions;
    //QFlags<EffectOptions> m_effectOptions;

    /// @brief Point color
    Vector4g m_pointColor;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
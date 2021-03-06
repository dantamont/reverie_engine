/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LINES_H
#define GB_LINES_H

// Internal
#include "../../mixins/GRenderable.h"
#include "../../GObject.h"
#include "../../containers/GFlags.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
struct VertexAttributes;
class Mesh;
class Transform;
class ResourceCache;
enum class ResourceBehaviorFlag;
/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class representing set of line geometry
class Lines: public Object, public Renderable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    enum class MeshGenerationFlag {
        kTriangulate = 1 << 0 // Whether or not to generate triangles from the points given
    };
    typedef Flags<MeshGenerationFlag> MeshGenerationFlags;

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


    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Lines();
    virtual ~Lines();
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool useMiter() const {
        return m_shapeOptions.testFlag(kUseMiter);
    }

    inline void setUseMiter(bool useMiter) {
        m_shapeOptions.setFlag(kUseMiter, useMiter);
    }

    inline void setConstantScreenThickness(bool constant) {
        m_shapeOptions.setFlag(kConstantScreenThickness, constant);
    }

    inline void setFadeWithDistance(bool fade) {
        m_effectOptions.setFlag(kFadeWithDistance, fade);
    }


    float lineThickness() const { return m_lineThickness; }
    void setLineThickness(float thickness) { m_lineThickness = thickness; }

    const Vector4& lineColor() const { return m_lineColor; }
    void setLineColor(const Vector4& lineColor) { m_lineColor = lineColor; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Set uniforms in the given draw command from this renderable
    /// @note This is used by the Lines and Points renderables for debug rendering
    virtual void setUniforms(DrawCommand& drawCommand) const override;

    /// @brief Load an existing vao as a Lines object
    void loadVertexArrayData(ResourceCache& cache, const VertexArrayData& data, Flags<ResourceBehaviorFlag> flags, MeshGenerationFlags meshFlags = 0);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() override;

    // TODO: Implement some control over sorting
    virtual size_t getSortID() override { return 0; }

	/// @}

    //---------------------------------------------------------------------------------------l--------------------------
    /// @name Serializable Overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}

protected:
    friend class Points;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the Lines in the given shader
    virtual void bindUniforms(ShaderProgram& shaderProgram) override;
    virtual void releaseUniforms(ShaderProgram& shaderProgram) override;

    /// @brief Draw geometry associated with this Lines
    virtual void drawGeometry(ShaderProgram& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(const Vector3& newPoint);
    void addPoint(VertexArrayData& vertexData, const Vector3& newPoint);

    /// @brief Add a triangle from three points
    void addTriangle(const Vector3& p1, const Vector3& p2, const Vector3& p3);


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Line thickness (in NDC)
    float m_lineThickness;

    Flags<ShapeOptions> m_shapeOptions;
    Flags<EffectOptions> m_effectOptions;

    /// @brief Line color
    Vector4 m_lineColor;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
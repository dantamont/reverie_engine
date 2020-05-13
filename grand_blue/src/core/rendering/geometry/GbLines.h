/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LINES_H
#define GB_LINES_H

// Internal
#include "../../mixins/GbRenderable.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class VertexArrayData;
struct VertexAttributes;
class Mesh;
class Transform;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Class representing set of line geometry
class Lines: public Object, public Renderable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    static std::shared_ptr<Lines> getCube();
    static std::shared_ptr<Lines> getSphere(int latSize = 20, int lonSize = 30);
    static std::shared_ptr<Lines> getGridCube(float spacing = 1.0f, int numHalfSpaces = 10);
    static std::shared_ptr<Lines> getPlane(float spacing = 1.0f, int numHalfSpaces = 10);
    static std::shared_ptr<Lines> getPrism(float baseRadius = 1.0, float topRadius = 1.0,
        float height = 1.0, int sectorCount = 36, int stackCount = 1);
    static std::shared_ptr<Lines> getCapsule(float radius = 1.0, float halfHeight = 1.0);

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

    const Vector4g& lineColor() const { return m_lineColor; }
    void setLineColor(const Vector4g& lineColor) { m_lineColor = lineColor; }

    Transform& transform() { return *m_transform; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Load in a mesh as a Lines object
    void loadMesh(std::shared_ptr<Mesh> mesh);
    void loadTriMesh(std::shared_ptr<Mesh> mesh);

    /// @brief Load an existing vao as a Lines object
    void loadVertexArrayData(const VertexArrayData& data);
    void loadTriVertexArrayData(const VertexArrayData& data);

    /// @brief Update any data needed for rendering, e.g. vertex data, render settings, etc.
    virtual void reload() override;

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
    friend class Points;

    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    /// @brief Set uniforms for the Lines in the given shader
    virtual void bindUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;
    virtual void releaseUniforms(const std::shared_ptr<ShaderProgram>& shaderProgram) override;

    /// @brief Bind the textures used by this Lines
    virtual void bindTextures() override;

    /// @brief Release the textures used by this Lines
    virtual void releaseTextures() override;

    /// @brief Draw geometry associated with this Lines
    virtual void drawGeometry(const std::shared_ptr<ShaderProgram>& shaderProgram, RenderSettings* settings = nullptr) override;

    /// @brief Add point
    void addPoint(const Vector3g& newPoint);
    void addPoint(std::shared_ptr<VertexArrayData> vertexData, const Vector3g& newPoint);

    /// @brief Add a triangle from three points
    void addTriangle(const Vector3g& p1, const Vector3g& p2, const Vector3g& p3);

    /// @brief Add a new line to the lines
    void addLine();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{

    /// @brief Line data corresponding to each line object
    std::vector<std::shared_ptr<VertexArrayData>> m_vertexData;

    /// @brief Line thickness (in NDC)
    float m_lineThickness;

    std::unique_ptr<Transform> m_transform;

    QFlags<ShapeOptions> m_shapeOptions;
    QFlags<EffectOptions> m_effectOptions;

    /// @brief Line color
    Vector4g m_lineColor;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
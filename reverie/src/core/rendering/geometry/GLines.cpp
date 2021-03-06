#include "GLines.h"
#include "../../GCoreEngine.h"
#include "../../resource/GResourceCache.h"
#include "../../rendering/geometry/GPolygon.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"
#include "../../resource/GResource.h"
#include "../geometry/GMesh.h"
#include "../geometry/GPolygon.h"
#include "../../geometry/GTransform.h"
#include "../../containers/GFlags.h"
#include "../renderer/GRenderCommand.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetCube(ResourceCache& cache, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getCube();
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags, MeshGenerationFlag::kTriangulate);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetSphere(ResourceCache& cache, int latSize, int lonSize, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getSphere(latSize, lonSize);
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags, MeshGenerationFlag::kTriangulate);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetGridCube(ResourceCache& cache, float spacing, int numHalfSpaces, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getGridCube(spacing, numHalfSpaces);
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetPlane(ResourceCache& cache, float spacing, int numHalfSpaces, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getGridPlane(spacing, numHalfSpaces);
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetPrism(ResourceCache& cache, float baseRadius, float topRadius, float height,
    int sectorCount, int stackCount, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getCylinder(baseRadius, topRadius, height, sectorCount, stackCount);
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::GetCapsule(ResourceCache& cache, float radius, float halfHeight, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh*mesh = cache.polygonCache()->getCapsule(radius, halfHeight);
    mesh->handle()->behaviorFlags() |= handleFlags;
    auto lines = std::make_shared<Lines>();
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Lines::Lines() :
    m_lineThickness(0.01f),
    m_lineColor(0.8f, 0.8f, 0.1f, 1.0f)
{
    //m_mesh = std::make_shared<Mesh>();
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addDefaultBlend();
    m_shapeOptions.setFlag(kUseMiter, true);
    m_shapeOptions.setFlag(kConstantScreenThickness, true);
    m_effectOptions.setFlag(kFadeWithDistance, false);

}
/////////////////////////////////////////////////////////////////////////////////////////////
Lines::~Lines()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::setUniforms(DrawCommand & drawCommand) const
{
    drawCommand.addUniform("constantScreenThickness", m_shapeOptions.testFlag(kConstantScreenThickness));
    drawCommand.addUniform("lineColor", m_lineColor);
    drawCommand.addUniform("thickness", m_lineThickness);
    drawCommand.addUniform("useMiter", m_shapeOptions.testFlag(kUseMiter));
    drawCommand.addUniform("fadeWithDistance", m_effectOptions.testFlag(kFadeWithDistance));
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadVertexArrayData(ResourceCache& cache, const VertexArrayData& data, Flags<ResourceBehaviorFlag> flags, MeshGenerationFlags meshFlags)
{
    // Initialize a new mesh
    initializeEmptyMesh(cache, flags);

    // Load data
    m_meshHandle->setIsLoading(true);
    if (meshFlags.testFlag(MeshGenerationFlag::kTriangulate)) {
        // Generate triangles from points
        for (size_t i = 0; i < data.m_indices.size(); i += 3) {
            int point1 = data.m_indices[i];
            int point2 = data.m_indices[i + 1];
            int point3 = data.m_indices[i + 2];
            addTriangle(data.m_attributes.m_vertices[point1],
                data.m_attributes.m_vertices[point2],
                data.m_attributes.m_vertices[point3]);
        }
    }
    else {
        // Add points as they are
        // TODO: In this case, see if the points need to be copied at all. Maybe just use the same mesh
        for (size_t i = 0; i < data.m_indices.size(); i++) {
            int point = data.m_indices[i];
            addPoint(data.m_attributes.m_vertices[point]);
        }
    }
    Mesh* mesh = m_meshHandle->resourceAs<Mesh>();
    mesh->vertexData().loadIntoVAO();
    mesh->generateBounds();
    m_meshHandle->setIsLoading(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::reload()
{    
    m_meshHandle->resourceAs<Mesh>()->vertexData().loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Lines::asJson(const SerializationContext& context) const
{
    QJsonObject object = Renderable::asJson(context).toObject();
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::bindUniforms(ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
	
	if (!UBO::getCameraBuffer()) return;
    //auto cameraBuffer = UBO::getCameraBuffer();

    //// Check that correct uniforms were set
    //if (!cameraBuffer->hasUniform("projectionMatrix")) {
    //    logWarning("Warning, projection matrix uniform not set for lines");
    //}
    //if (!cameraBuffer->hasUniform("viewMatrix")) {
    //    logWarning("Warning, view matrix uniform not set for lines");
    //}

    // Set remaining uniforms
    //shaderProgram.setUniformValue("constantScreenThickness", 
    //    m_shapeOptions.testFlag(kConstantScreenThickness));
    //shaderProgram.setUniformValue("lineColor", m_lineColor);
    //shaderProgram.setUniformValue("thickness", m_lineThickness);
    //shaderProgram.setUniformValue("useMiter", m_shapeOptions.testFlag(kUseMiter));
    //shaderProgram.setUniformValue("fadeWithDistance", m_effectOptions.testFlag(kFadeWithDistance));
    //
    //shaderProgram.updateUniforms();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::drawGeometry(ShaderProgram& shaderProgram, 
    RenderSettings * settings)
{
    Q_UNUSED(shaderProgram)
        Mesh* mesh = Lines::mesh();
    if (!mesh) {
        return;
    }
    mesh->vertexData().drawGeometry(settings->shapeMode());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addPoint(const Vector3 & point)
{
    addPoint(m_meshHandle->resourceAs<Mesh>()->vertexData(), point);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addPoint(VertexArrayData& vertexData, const Vector3 & point)
{
    // Set next point to this point for vertex attributes already stored
    unsigned int size = (unsigned int)vertexData.m_attributes.m_vertices.size();
    Vector3 previousPoint = point;// If first point, no previous point to sample
    if (size) {
        // Tangent attr is used to store next point
        vertexData.m_attributes.m_tangents[size - 1] = point;
        vertexData.m_attributes.m_tangents[size - 2] = point;
        
        // Cache previous point if there is one
        previousPoint = vertexData.m_attributes.m_vertices.back();
    }

    // Add vertex position
    // Adding in pairs, since two points in width-direction of line
    Vec::EmplaceBack(vertexData.m_attributes.m_vertices, point);
    Vec::EmplaceBack(vertexData.m_attributes.m_vertices, point);

    // Add previous point attribute
    // Normals attr is used to store previous point
    Vec::EmplaceBack(vertexData.m_attributes.m_normals, previousPoint);
    Vec::EmplaceBack(vertexData.m_attributes.m_normals, previousPoint);

    // Add next point attribute (same as this point until another added)
    // Tangent attr is used to store next point
    Vec::EmplaceBack(vertexData.m_attributes.m_tangents, point);
    Vec::EmplaceBack(vertexData.m_attributes.m_tangents, point);

    // Add direction attributes, room for three more int attributes in buffer
    Vec::EmplaceBack(vertexData.m_attributes.m_miscInt, Vector4i(-1, 0, 0, 0));
    Vec::EmplaceBack(vertexData.m_attributes.m_miscInt, Vector4i(1, 0, 0, 0));

    // Add indices if this is the second point added
    // REMEMBER, this is creating triangles, since lines are not actually lines at all
    unsigned int halfSize = size / 2;
    if (halfSize % 2 == 1) {
        // Indices must be CCW (right-hand normal rule)
        unsigned int startIndex = size - 2;
        vertexData.m_indices.insert(vertexData.m_indices.end(), 
            {startIndex,
             startIndex + 3,
             startIndex + 2,
             startIndex,
             startIndex + 1,
             startIndex + 3 }
        );
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addTriangle(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3)
{
    // First line segment (two triangles)
    addPoint(p1);
    addPoint(p2);

    // Second line segment (two more triangles)
    addPoint(p3);
    addPoint(p1);

    // Third line segment (two final triangles)
    addPoint(p2);
    addPoint(p3);
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#include "core/rendering/geometry/GLines.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/buffers/GUniformBufferObject.h"
#include "core/resource/GResource.h"
#include "core/rendering/geometry/GMesh.h"
#include "core/rendering/geometry/GPolygon.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GRenderContext.h"

#include "fortress/containers/GColor.h"
#include "fortress/containers/math/GTransform.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {

std::shared_ptr<Lines> Lines::GetShape(RenderContext& context, const std::vector<Vector3>& meshPoints, const std::vector<Uint32_t>& indices, ResourceBehaviorFlags handleFlags)
{
    auto lines = prot_make_shared<Lines>(context.uniformContainer());
    lines->loadPointData(ResourceCache::Instance(), meshPoints, indices, handleFlags, 0);
    return lines;
}

std::shared_ptr<Lines> Lines::GetCube(ResourceCache& cache, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getCube();
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags, MeshGenerationFlag::kTriangulate);
    return lines;
}

std::shared_ptr<Lines> Lines::GetSphere(ResourceCache& cache, int latSize, int lonSize, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getSphere(latSize, lonSize);
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags, MeshGenerationFlag::kTriangulate);
    return lines;
}

std::shared_ptr<Lines> Lines::GetGridCube(ResourceCache& cache, float spacing, int numHalfSpaces, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getGridCube(spacing, numHalfSpaces);
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}

std::shared_ptr<Lines> Lines::GetPlane(ResourceCache& cache, float spacing, int numHalfSpaces, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getGridPlane(spacing, numHalfSpaces);
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}

std::shared_ptr<Lines> Lines::GetPrism(ResourceCache& cache, float baseRadius, float topRadius, float height,
    int sectorCount, int stackCount, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh* mesh = cache.polygonCache()->getCylinder(baseRadius, topRadius, height, sectorCount, stackCount);
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}

std::shared_ptr<Lines> Lines::GetCapsule(ResourceCache& cache, float radius, float halfHeight, ResourceBehaviorFlags handleFlags)
{
    // Flags need to be set in both the source mesh, and the generated lines mesh
    Mesh*mesh = cache.polygonCache()->getCapsule(radius, halfHeight);
    mesh->handle()->setBehaviorFlags(mesh->handle()->behaviorFlags() | handleFlags);
    auto lines = prot_make_shared<Lines>(cache.engine()->openGlRenderer()->renderContext().uniformContainer());
    lines->loadVertexArrayData(cache, mesh->vertexData(), handleFlags);
    return lines;
}

Lines::Lines(UniformContainer& uc) :
    m_lineThickness(0.01f),
    m_lineColor(0.8f, 0.8f, 0.1f, 1.0f)
{
    //m_mesh = std::make_shared<Mesh>();
    m_renderSettings.setShapeMode(PrimitiveMode::kTriangles);
    m_renderSettings.addDefaultBlend();
    m_shapeOptions.setFlag(kUseMiter, true);
    m_shapeOptions.setFlag(kConstantScreenThickness, true);
    m_effectOptions.setFlag(kFadeWithDistance, false);

    initializeUniformValues(uc);
}

Lines::~Lines()
{
}

void Lines::setUseMiter(bool useMiter, UniformContainer& uc)
{
    m_shapeOptions.setFlag(kUseMiter, useMiter);
    m_uniforms.m_useMiter.setValue(useMiter, uc);
}

void Lines::setConstantScreenThickness(bool constant, UniformContainer& uc)
{
    m_shapeOptions.setFlag(kConstantScreenThickness, constant);
    m_uniforms.m_hasConstantScreenThickness.setValue(constant, uc);
}

void Lines::setFadeWithDistance(bool fade, UniformContainer& uc)
{
    m_effectOptions.setFlag(kFadeWithDistance, fade);
    m_uniforms.m_fadeWithDistance.setValue(fade, uc);
}

void Lines::setLineThickness(float thickness, UniformContainer& uc)
{
    m_lineThickness = thickness;
    m_uniforms.m_lineThickness.setValue(thickness, uc);
}

void Lines::setLineColor(const Vector4& lineColor, UniformContainer& uc)
{
    m_lineColor = lineColor;
    m_uniforms.m_lineColor.setValue(lineColor, uc);
}

void Lines::setUniforms(DrawCommand & drawCommand) const
{
    ShaderProgram* shader = drawCommand.shaderProgram();
    ShaderProgram* prepassShader = drawCommand.prepassShaderProgram();

    drawCommand.addUniform(
        m_uniforms.m_hasConstantScreenThickness,
        shader->uniformMappings().m_constantScreenThickness,
        prepassShader ? prepassShader->uniformMappings().m_constantScreenThickness : -1
    );
    drawCommand.addUniform(
        m_uniforms.m_lineColor,
        shader->uniformMappings().m_lineColor,
        prepassShader ? prepassShader->uniformMappings().m_lineColor : -1
    );
    drawCommand.addUniform(
        m_uniforms.m_lineThickness,
        shader->uniformMappings().m_lineThickness,
        prepassShader ? prepassShader->uniformMappings().m_lineThickness : -1
    );
    drawCommand.addUniform(
        m_uniforms.m_useMiter,
        shader->uniformMappings().m_useMiter,
        prepassShader ? prepassShader->uniformMappings().m_useMiter : -1
    );
    drawCommand.addUniform(
        m_uniforms.m_fadeWithDistance,
        shader->uniformMappings().m_fadeWithDistance,
        prepassShader ? prepassShader->uniformMappings().m_fadeWithDistance : -1
    );
}

void Lines::initializeUniformValues(UniformContainer& uc)
{
    m_uniforms.m_hasConstantScreenThickness.setValue(m_shapeOptions.testFlag(kConstantScreenThickness), uc);
    m_uniforms.m_lineColor.setValue(m_lineColor, uc);
    m_uniforms.m_lineThickness.setValue(m_lineThickness, uc);
    m_uniforms.m_useMiter.setValue(m_shapeOptions.testFlag(kUseMiter), uc);
    m_uniforms.m_fadeWithDistance.setValue(m_effectOptions.testFlag(kFadeWithDistance), uc);
}


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

void Lines::loadPointData(ResourceCache& cache, const std::vector<Vector3>& data, const std::vector<Uint32_t>& indices, Flags<ResourceBehaviorFlag> flags, MeshGenerationFlags meshFlags)
{
    // Initialize a new mesh
    initializeEmptyMesh(cache, flags);

    // Load data
    m_meshHandle->setIsLoading(true);
    if (meshFlags.testFlag(MeshGenerationFlag::kTriangulate)) {
        // Generate triangles from points
        for (size_t i = 0; i < indices.size(); i += 3) {
            int point1 = indices[i];
            int point2 = indices[i + 1];
            int point3 = indices[i + 2];
            addTriangle(data[point1],
                data[point2],
                data[point3]);
        }
    }
    else {
        // Add points as they are
        // TODO: In this case, see if the points need to be copied at all. Maybe just use the same mesh
        for (size_t i = 0; i < indices.size(); i++) {
            int point = indices[i];
            addPoint(data[point]);
        }
    }
    Mesh* mesh = m_meshHandle->resourceAs<Mesh>();
    mesh->vertexData().loadIntoVAO();
    mesh->generateBounds();
    m_meshHandle->setIsLoading(false);
}

void Lines::reload()
{
    m_meshHandle->resourceAs<Mesh>()->vertexData().loadIntoVAO();
}

void Lines::reload(const std::vector<Vector3>& data, const std::vector<Uint32_t>& indices)
{
    Mesh* mesh = m_meshHandle->resourceAs<Mesh>();
    VertexArrayData& vertexData = mesh->vertexData();
    vertexData.destroyBuffers(); // Destroy buffers in OpenGL and clear locally
    vertexData.vao().destroy(); ///< Destroy the vao itself
    vertexData.m_attributes.clear();
    vertexData.m_attributeBuffers.resize((size_t)BufferAttributeType::kMAX_ATTRIBUTE_TYPE);

    /// @todo Try not to be redundant with Lines::LoadPointData
    // Load data
    m_meshHandle->setIsLoading(true);
    for (size_t i = 0; i < indices.size(); i++) {
        int point = indices[i];
        addPoint(data[point]);
    }
    vertexData.loadIntoVAO();
    mesh->generateBounds();
    m_meshHandle->setIsLoading(false);
}

void to_json(json& orJson, const Lines& korObject)
{
    ToJson<Renderable>(orJson, korObject);
}

void from_json(const json& korJson, Lines& orObject)
{
    FromJson<Renderable>(korJson, orObject);
}

void Lines::bindUniforms(ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
	
	if (!Ubo::GetCameraBuffer()) return;
}

void Lines::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}

void Lines::drawGeometry(ShaderProgram& shaderProgram, 
    RenderSettings * settings)
{
    Q_UNUSED(shaderProgram)
        Mesh* mesh = Lines::mesh();
    if (!mesh) {
        return;
    }
    mesh->vertexData().drawGeometry(settings->shapeMode(), 1);
}

void Lines::addPoint(const Vector3 & point)
{
    addPoint(m_meshHandle->resourceAs<Mesh>()->vertexData(), point);
}

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

} // End namespaces

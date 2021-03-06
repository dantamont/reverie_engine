#include "GPoints.h"

#include "GLines.h"
#include <core/rendering/view/GViewport.h>
#include "../../GCoreEngine.h"
#include "../../rendering/geometry/GPolygon.h"
#include "../shaders/GShaderProgram.h"
#include "../buffers/GUniformBufferObject.h"
#include "../../resource/GResourceCache.h"
#include "../../resource/GResource.h"
#include "../geometry/GMesh.h"
#include "../geometry/GSkeleton.h"
#include "../geometry/GPolygon.h"
#include "../../geometry/GTransform.h"
#include "../renderer/GRenderCommand.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points() :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f)
{
    //m_mesh = std::make_shared<Mesh>();
    m_renderSettings.setShapeMode(PrimitiveMode::kPoints);
    m_renderSettings.addDefaultBlend();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(ResourceCache& cache, const Skeleton & skeleton, Flags<ResourceBehaviorFlag> flags) :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f)
{
    m_renderSettings.setShapeMode(PrimitiveMode::kPoints);
    m_renderSettings.addDefaultBlend();

    // Load vertex data for skeleton bones
    // TODO: Clean this up
    initializeEmptyMesh(cache, flags);
    Mesh* mesh = Points::mesh();

    int numBones = skeleton.boneNodes().size();
    mesh->vertexData().m_attributes.m_vertices.resize(numBones);
    mesh->vertexData().m_indices.resize(numBones);
    mesh->vertexData().m_attributes.m_miscInt.resize(numBones);
    std::iota(mesh->vertexData().m_indices.begin(), mesh->vertexData().m_indices.end(), 0);
    int count = 0;
    for (auto& vec : mesh->vertexData().m_attributes.m_miscInt) {
        vec[0] = count;
        count++;
    }
    mesh->vertexData().loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(ResourceCache& cache, size_t numPoints, Flags<ResourceBehaviorFlag> flags) :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f)
    //m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(PrimitiveMode::kPoints);
    m_renderSettings.addDefaultBlend();

    // Load vertex data for skeleton bones
    // TODO: Clean this up
    // Note, only really need the number of points (bones) in the skeleton)
    initializeEmptyMesh(cache, flags);
    Mesh* mesh = Points::mesh();

    //mesh = std::make_shared<Mesh>();
    mesh->vertexData().m_attributes.m_vertices.resize(numPoints);
    mesh->vertexData().m_indices.resize(numPoints);
    mesh->vertexData().m_attributes.m_miscInt.resize(numPoints);
    std::iota(mesh->vertexData().m_indices.begin(), mesh->vertexData().m_indices.end(), 0);
    int count = 0;
    for (auto& vec : mesh->vertexData().m_attributes.m_miscInt) {
        vec[0] = count;
        count++;
    }
    mesh->vertexData().loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(const Lines & lines):
    m_pointColor(lines.m_lineColor),
    m_pointSize(0.01f)
    //m_transform(std::make_unique<Transform>())
{
    m_meshHandle = lines.m_meshHandle;
    m_renderSettings.setShapeMode(PrimitiveMode::kPoints);
    m_renderSettings.addDefaultBlend();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::~Points()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::setUniforms(DrawCommand & drawCommand) const
{
    drawCommand.addUniform("color", m_pointColor);
    drawCommand.addUniform("pointSize", m_pointSize);
    drawCommand.addUniform("screenPixelWidth", Viewport::ScreenDimensionsVec().x());
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Points::numPoints() const
{
    Mesh* mesh = Points::mesh();
    return mesh->vertexData().m_attributes.m_vertices.size();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::loadVertexArrayData(ResourceCache& cache, const VertexArrayData& data, Flags<ResourceBehaviorFlag> flags)
{
    // Initialize a new mesh
    initializeEmptyMesh(cache, flags);
    m_meshHandle->setIsLoading(true);
    
    // Load data
    Mesh* mesh = Points::mesh();
    VertexArrayData& vdata = mesh->vertexData();
    for (size_t i = 0; i < data.m_indices.size(); i++) {
        int point = data.m_indices[i];
        addPoint(vdata, data.m_attributes.m_vertices[point]);
    }
    vdata.loadIntoVAO();
    mesh->generateBounds();
    m_meshHandle->setIsLoading(false);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::reload()
{    
    Mesh* mesh = Points::mesh();
    mesh->vertexData().loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Points::asJson(const SerializationContext& context) const
{
    QJsonObject object = Renderable::asJson(context).toObject();
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)
    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::bindUniforms(ShaderProgram& shaderProgram)
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
    //shaderProgram.setUniformValue("color", m_pointColor);
    //shaderProgram.setUniformValue("pointSize", m_pointSize);
    //shaderProgram.setUniformValue("screenPixelWidth", screenDimensionsVec().x());

    //shaderProgram.updateUniforms();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::releaseUniforms(ShaderProgram& shaderProgram)
{
    Q_UNUSED(shaderProgram)
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::drawGeometry(ShaderProgram& shaderProgram, 
    RenderSettings * settings)
{
    Q_UNUSED(shaderProgram)

    // TODO: Move this to a RenderSetting
    glEnable(GL_PROGRAM_POINT_SIZE); // Enable point sizing
    //glEnable(GL_POINT_SMOOTH); // Make points actual circles
    //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    Mesh* mesh = Points::mesh();
    if (!mesh) {
        return;
    }
    mesh->vertexData().drawGeometry(settings->shapeMode());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::addPoint(VertexArrayData& vertexData, const Vector3 & point)
{
    // Set next point to this point for vertex attributes already stored
    int size = (int)vertexData.m_attributes.m_vertices.size();

    // Add vertex position
    Vec::EmplaceBack(vertexData.m_attributes.m_vertices, point);

    // Add indices 
    vertexData.m_indices.push_back(size);
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

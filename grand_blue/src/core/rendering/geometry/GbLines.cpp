#include "GbLines.h"
#include "../../GbCoreEngine.h"
#include "../../resource/GbResourceCache.h"
#include "../../rendering/geometry/GbPolygon.h"
#include "../shaders/GbShaders.h"
#include "../shaders/GbUniformBufferObject.h"
#include "../../resource/GbResource.h"
#include "../geometry/GbMesh.h"
#include "../geometry/GbPolygon.h"
#include "../../geometry/GbTransform.h"
#include "../../containers/GbFlags.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getCube(int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getCube();
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadTriMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getSphere(int latSize, int lonSize, int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getSphere(latSize, lonSize);
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadTriMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getGridCube(float spacing, int numHalfSpaces, int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getGridCube(spacing, numHalfSpaces);
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getPlane(float spacing, int numHalfSpaces, int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getGridPlane(spacing, numHalfSpaces);
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getPrism(float baseRadius, float topRadius, float height, 
    int sectorCount, int stackCount, int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getCylinder(
            baseRadius, topRadius, height, sectorCount, stackCount);
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Lines> Lines::getCapsule(float radius, float halfHeight, int handleFlags)
{
    CoreEngine* engine = CoreEngine::engines().begin()->second;
    auto mesh = engine->resourceCache()->polygonCache()->getCapsule(
            radius, halfHeight);
    if (handleFlags > 0) {
        mesh->handle()->behaviorFlags() |= Flags::toFlags<ResourceHandle::BehaviorFlag>(handleFlags);
    }
    auto lines = std::make_shared<Lines>();
    lines->loadMesh(mesh);
    return lines;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Lines::Lines() :
    m_lineThickness(0.01f),
    m_lineColor(0.8f, 0.8f, 0.1f, 1.0f),
    m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(GL_TRIANGLES);
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
void Lines::loadMesh(std::shared_ptr<Mesh> mesh)
{   
    loadVertexArrayData(mesh->vertexData());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadTriMesh(std::shared_ptr<Mesh> mesh)
{
    loadTriVertexArrayData(mesh->vertexData());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadVertexArrayData(const VertexArrayData& data)
{
    addLine();
    for (size_t i = 0; i < data.m_indices.size(); i++) {
        int point = data.m_indices[i];
        addPoint(data.m_attributes.m_vertices[point]);
    }
    m_vertexData.back()->loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadTriVertexArrayData(const VertexArrayData & data)
{
    addLine();
    for (size_t i = 0; i < data.m_indices.size(); i+=3) {
        int point1 = data.m_indices[i];
        int point2 = data.m_indices[i + 1];
        int point3 = data.m_indices[i + 2];
        addTriangle(data.m_attributes.m_vertices[point1],
            data.m_attributes.m_vertices[point2],
            data.m_attributes.m_vertices[point3]);
    }
    m_vertexData.back()->loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::reload()
{    
    for (std::shared_ptr<VertexArrayData>& data : m_vertexData) {
        data->loadIntoVAO();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Lines::asJson() const
{
    QJsonObject object = Renderable::asJson().toObject();
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::bindUniforms(ShaderProgram& shaderProgram)
{
    Renderable::bindUniforms(shaderProgram);
	
	if (!UBO::getCameraBuffer()) return;
    auto cameraBuffer = UBO::getCameraBuffer();

    // Check that correct uniforms were set
    if (!cameraBuffer->hasUniform("projectionMatrix")) {
        logWarning("Warning, projection matrix uniform not set for lines");
    }
    if (!cameraBuffer->hasUniform("viewMatrix")) {
        logWarning("Warning, view matrix uniform not set for lines");
    }

    // Set remaining uniforms
    //auto pm = shaderProgram->getUniformValue("projectionMatrix")->as<Matrix4x4g>();
    //logInfo("aspect " + QString::number(pm(1, 1)/pm(0, 0)));
    if (!m_drawFlags.testFlag(DrawFlag::kIgnoreWorldMatrix)) {
        shaderProgram.setUniformValue("worldMatrix", m_transform->worldMatrix());
    }
    shaderProgram.setUniformValue("constantScreenThickness", 
        m_shapeOptions.testFlag(kConstantScreenThickness));
    shaderProgram.setUniformValue("lineColor", m_lineColor);
    shaderProgram.setUniformValue("thickness", m_lineThickness);
    shaderProgram.setUniformValue("useMiter", m_shapeOptions.testFlag(kUseMiter));
    shaderProgram.setUniformValue("fadeWithDistance", m_effectOptions.testFlag(kFadeWithDistance));
    shaderProgram.updateUniforms();
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
    for (std::shared_ptr<VertexArrayData>& data : m_vertexData) {
        data->drawGeometry(settings->shapeMode());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addPoint(const Vector3g & point)
{
    auto vertexData = m_vertexData.back();
    addPoint(vertexData, point);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addPoint(std::shared_ptr<VertexArrayData> vertexData, const Vector3g & point)
{
    // Set next point to this point for vertex attributes already stored
    int size = (int)vertexData->m_attributes.m_vertices.size();
    Vector3g previousPoint = point;
    if (size) {
        // Tangent attr is used to store next point
        vertexData->m_attributes.m_tangents[size - 1] = point;
        vertexData->m_attributes.m_tangents[size - 2] = point;
        
        // Cache previous point if there is one
        previousPoint = vertexData->m_attributes.m_vertices.back();
    }

    // Add vertex position
    Vec::EmplaceBack(vertexData->m_attributes.m_vertices, point);
    Vec::EmplaceBack(vertexData->m_attributes.m_vertices, point);

    // Add previous point attribute
    // Normals attr is used to store previous point
    Vec::EmplaceBack(vertexData->m_attributes.m_normals, previousPoint);
    Vec::EmplaceBack(vertexData->m_attributes.m_normals, previousPoint);

    // Add next point attribute (same as this point until another added)
    // Tangent attr is used to store next point
    Vec::EmplaceBack(vertexData->m_attributes.m_tangents, point);
    Vec::EmplaceBack(vertexData->m_attributes.m_tangents, point);

    // Add direction attributes, room for three more int attributes in buffer
    Vec::EmplaceBack(vertexData->m_attributes.m_miscInt, Vector4i(-1, 0, 0, 0));
    Vec::EmplaceBack(vertexData->m_attributes.m_miscInt, Vector4i(1, 0, 0, 0));

    // Add indices if this is the second point added
    int halfSize = size / 2;
    if (halfSize % 2 == 1) {
        // Indices must be CCW (right-hand normal rule)
        size_t startIndex = (size_t)(size - 2);
        vertexData->m_indices.insert(vertexData->m_indices.end(), 
            {startIndex,
             startIndex + 2,
             startIndex + 1,
             startIndex + 2,
             startIndex + 3,
             startIndex + 1 }
        );
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addTriangle(const Vector3g & p1, const Vector3g & p2, const Vector3g & p3)
{
    addPoint(p1);
    addPoint(p2);
    addPoint(p3);
    addPoint(p1);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Lines::addLine()
{
    Vec::EmplaceBack(m_vertexData, std::make_shared<VertexArrayData>());
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

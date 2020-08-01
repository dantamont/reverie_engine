#include "GbPoints.h"

#include "GbLines.h"
#include "../../GbCoreEngine.h"
#include "../../rendering/geometry/GbPolygon.h"
#include "../shaders/GbShaders.h"
#include "../shaders/GbUniformBufferObject.h"
#include "../../resource/GbResourceCache.h"
#include "../../resource/GbResource.h"
#include "../geometry/GbMesh.h"
#include "../geometry/GbSkeleton.h"
#include "../geometry/GbPolygon.h"
#include "../../geometry/GbTransform.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points() :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f),
    m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(GL_POINTS);
    m_renderSettings.addDefaultBlend();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(const Skeleton & skeleton) :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f),
    m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(GL_POINTS);
    m_renderSettings.addDefaultBlend();

    // Load vertex data for skeleton bones
    // TODO: Clean this up
    auto data = std::make_shared<VertexArrayData>();
    int numBones = skeleton.boneNodes().size();
    data->m_attributes.m_vertices.resize(numBones);
    data->m_indices.resize(numBones);
    data->m_attributes.m_miscInt.resize(numBones);
    std::iota(data->m_indices.begin(), data->m_indices.end(), 0);
    int count = 0;
    for (auto& vec : data->m_attributes.m_miscInt) {
        vec[0] = count;
        count++;
    }
    data->loadIntoVAO();
    m_vertexData.push_back(data);
    //loadVertexArrayData(data);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(size_t numPoints) :
    m_pointSize(0.01f),
    m_pointColor(0.8f, 0.3f, 0.2f, 1.0f),
    m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(GL_POINTS);
    m_renderSettings.addDefaultBlend();

    // Load vertex data for skeleton bones
    // TODO: Clean this up
    // Note, only really need the number of points (bones) in the skeleton)
    auto data = std::make_shared<VertexArrayData>();
    data->m_attributes.m_vertices.resize(numPoints);
    data->m_indices.resize(numPoints);
    data->m_attributes.m_miscInt.resize(numPoints);
    std::iota(data->m_indices.begin(), data->m_indices.end(), 0);
    int count = 0;
    for (auto& vec : data->m_attributes.m_miscInt) {
        vec[0] = count;
        count++;
    }
    data->loadIntoVAO();
    m_vertexData.push_back(data);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::Points(const Lines & lines):
    m_pointColor(lines.m_lineColor),
    m_pointSize(0.01f),
    m_vertexData(lines.m_vertexData),
    m_transform(std::make_unique<Transform>())
{
    m_renderSettings.setShapeMode(GL_POINTS);
    m_renderSettings.addDefaultBlend();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Points::~Points()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Points::numPoints() const
{
    size_t count = 0;
    for (const std::shared_ptr<VertexArrayData>& data : m_vertexData) {
        count += data->m_attributes.m_vertices.size();
    }

    return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::loadMesh(std::shared_ptr<Mesh> mesh)
{   
    loadVertexArrayData(mesh->vertexData());
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::loadVertexArrayData(const VertexArrayData& data)
{
    addPointSet();
    for (size_t i = 0; i < data.m_indices.size(); i++) {
        int point = data.m_indices[i];
        addPoint(data.m_attributes.m_vertices[point]);
    }
    m_vertexData.back()->loadIntoVAO();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::reload()
{    
    for (std::shared_ptr<VertexArrayData>& data : m_vertexData) {
        data->loadIntoVAO();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue Points::asJson() const
{
    QJsonObject object = Renderable::asJson().toObject();
    return object;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    Renderable::loadFromJson(json);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::bindUniforms(ShaderProgram& shaderProgram)
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
    shaderProgram.setUniformValue("worldMatrix", m_transform->worldMatrix());
    shaderProgram.setUniformValue("color", m_pointColor);
    shaderProgram.setUniformValue("pointSize", m_pointSize);
    shaderProgram.setUniformValue("screenPixelWidth", screenDimensionsVec().x());
    shaderProgram.updateUniforms();
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

    for (std::shared_ptr<VertexArrayData>& data : m_vertexData) {
        data->drawGeometry(settings->shapeMode());
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::addPoint(const Vector3g & point)
{
    auto vertexData = m_vertexData.back();
    addPoint(vertexData, point);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::addPoint(std::shared_ptr<VertexArrayData> vertexData, const Vector3g & point)
{
    // Set next point to this point for vertex attributes already stored
    int size = (int)vertexData->m_attributes.m_vertices.size();

    // Add vertex position
    Vec::EmplaceBack(vertexData->m_attributes.m_vertices, point);

    // Add indices 
    vertexData->m_indices.push_back(size);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Points::addPointSet()
{
    Vec::EmplaceBack(m_vertexData, std::make_shared<VertexArrayData>());
}

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

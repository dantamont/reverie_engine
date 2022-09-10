#include "core/rendering/geometry/GMesh.h"

#include "core/rendering/geometry/GSkeleton.h"
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "fortress/process/GProcess.h"
#include "core/rendering/models/GModel.h"
#include "Core/rendering/renderer/GOpenGlRenderer.h"
#include "Core/rendering/renderer/GRenderContext.h"
#include "core/rendering/materials/GMaterial.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/loop/GSimLoop.h"

#include "core/rendering/renderer/GRenderSettings.h"

namespace rev {

Mesh::Mesh() : Resource()
{
}

Mesh::~Mesh()
{
}

void Mesh::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache);
}

void Mesh::postConstruction(const ResourcePostConstructionData& postConstructData)
{
#ifdef DEBUG_MODE
    gl::OpenGLFunctions functions;
    functions.printGLError("Error prior to mesh post-construction");
#endif

    // Set metadata from vertex attributes
    assert(postConstructData.m_data && "No data specified");
    MeshVertexAttributes* vertexData = static_cast<MeshVertexAttributes*>(postConstructData.m_data);
    m_vertexData.m_sizeInBytes = vertexData->getSizeInBytes();
    m_vertexData.m_vertexCount = vertexData->get<MeshVertexAttributeType::kPosition>().size();
    m_vertexData.m_indexCount = vertexData->get<MeshVertexAttributeType::kIndices>().size();

    // Initialize GL buffers for the mesh
    m_vertexData.loadIntoVAO(m_handle->engine()->openGlRenderer()->renderContext(), *vertexData);

    // Set cost to be the size of the newly initialized mesh data
    m_cost = m_vertexData.sizeInMegaBytes();

    // Generate bounds for the mesh
    generateBounds(*vertexData);

    // Call parent class construction routine
    Resource::postConstruction(postConstructData);

#ifdef DEBUG_MODE
    functions.printGLError("Error after mesh post-construction");
#endif

    // Delete the vertex data
    if (postConstructData.m_deleteAfterConstruction) {
        delete vertexData;
    }
}

void Mesh::generateBounds(const MeshVertexAttributes& data)
{
    // Initialize limits for bounding box of the mesh
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Update min/max vertex data
    std::vector<Vector3> vertexPositions = data.get<MeshVertexAttributeType::kPosition>();
    for (const Vector3& vertex : vertexPositions) {
        minX = std::min(vertex.x(), minX);
        maxX = std::max(vertex.x(), maxX);
        minY = std::min(vertex.y(), minY);
        maxY = std::max(vertex.y(), maxY);
        minZ = std::min(vertex.z(), minZ);
        maxZ = std::max(vertex.z(), maxZ);
    }

    m_objectBounds.boxData().setMinX(minX);
    m_objectBounds.boxData().setMaxX(maxX);
    m_objectBounds.boxData().setMinY(minY);
    m_objectBounds.boxData().setMaxY(maxY);
    m_objectBounds.boxData().setMinZ(minZ);
    m_objectBounds.boxData().setMaxZ(maxZ);
}




void to_json(nlohmann::json& orJson, const Mesh& korObject)
{
}

void from_json(const nlohmann::json& korJson, Mesh& orObject)
{
}

} // End namespacing
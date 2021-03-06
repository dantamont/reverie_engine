#include "GOBJReader.h"

#include "../../resource/GResourceCache.h"
#include "../../rendering/models/GModel.h"
#include "../../rendering/renderer/GMainRenderer.h"
#include <QElapsedTimer>

// Third party
#include "../../../third_party/tiny_obj_loader/tiny_obj_loader.h"
#include "GModelReader.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjReader::ObjReader(ResourceCache* cache, const QString & filepath, const QString & textureFilepath) :
    FileReader(filepath),
    m_textureFilePath(textureFilepath),
    m_resourceCache(cache)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
rev::ObjReader::~ObjReader()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void ObjReader::loadModel(Mesh& outMesh)
//{
//    // Parse file to construct mesh data, storing results as attributes
//    //parseFile();
//
//    // Set mesh data in mesh
//    //loadMesh(outMesh);
//
//    // Load materials used by mesh
//    //loadMaterials();
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjReader::loadMaterials()
{
    if (m_shapes.size() == 0) {
        parseFile();
    }

    //for(auto& materialData: m_materials){
    //    QString mtlName = QString::fromStdString(materialData.m_name).toLower();
    //    // Skip if material is loaded already
    //    if (m_resourceCache->hasMaterial(mtlName)) {
    //        continue;
    //    }

    //    // Create material
    //    std::shared_ptr<Material> mtl = std::make_shared<Material>(
    //        m_resourceCache->engine(), 
    //        materialData);
    //    m_resourceCache->addMaterial(mtl);
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjReader::parseFile()
{
    // Load from file if not yet loaded 
    std::string inputfile = m_filePath.toStdString();

    auto config = tinyobj::ObjReaderConfig();
    bool valid = parseFromFile(inputfile, config);

    if (!valid) {
        throw("Error, OBJ not parsed correctly, likely invalid");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void ObjReader::loadMesh(Mesh& outMesh)
//{
//    // Iterate through shapes to create sub-meshes
//    for (size_t i = 0; i < m_shapes.size(); i++) {
//        const tinyobj::shape_t& shape = m_shapes[i];
//
//        // Create sub-mesh
//        QString meshName = QString::fromStdString(shape.m_name);
//        Mesh* subMesh = new Mesh(meshName, outMesh.m_usagePattern);
//
//        // Create mesh data
//        subMesh->m_meshData = VertexArrayData(m_filePath);
//
//        // Populate indices, texture coordinates, and normals from tiny_obj shapes
//        subMesh->m_meshData.m_attributes.m_vertices = std::vector<Vector3g>(m_vertexAttributes.m_vertices.size());
//        subMesh->m_meshData.m_attributes.m_normals = std::vector<Vector3g>(m_vertexAttributes.m_vertices.size());
//        subMesh->m_meshData.m_attributes.m_texCoords = std::vector<Vector2g>(m_vertexAttributes.m_vertices.size());
//        subMesh->m_meshData.m_attributes.m_tangents = std::vector<Vector3g>(m_vertexAttributes.m_vertices.size());
//
//        // Add material info if present
//        if (m_materials.size() > 0) {
//            // Enforce case insensitivity
//            QString materialName = QString::fromStdString(m_materials[i].m_name).toLower();
//            subMesh->m_meshData.m_materialName = materialName;
//        }
//
//        // Process vertices to load into mesh
//        for (const tinyobj::index_t& index : shape.m_mesh.m_indices) {
//            processVertex(index, subMesh->m_meshData);
//        }
//
//        // Generate tangents for the shape data
//        subMesh->m_meshData.generateTangents();
//
//        if (m_shapes.size() > 1) {
//            // Add sub-mesh to parent mesh
//            outMesh.addChild(subMesh);
//        }
//        else {
//            // Set parent mesh data if there is only one mesh
//            outMesh.m_meshData = std::move(subMesh->m_meshData);
//            outMesh.m_name = subMesh->m_name;
//        }
//    }
//
//    // Check the validity of the output mesh
//    outMesh.checkValidity();
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ObjReader::parseFromFile(const std::string & filename, const tinyobj::ObjReaderConfig & config)
{
    // See: https://computergraphicsguide.blogspot.com/2015/08/wavefront-obj-file-format-and-normal.html
    std::string mtl_search_path;

    m_materials.clear();
    std::string warning;
    std::string error;

    if (config.m_mtlSearchPath.empty()) {
        //
        // split at last '/'(for unixish system) or '\\'(for windows) to get
        // the base directory of .obj file
        //
        if (filename.find_last_of("/\\") != std::string::npos) {
            mtl_search_path = filename.substr(0, filename.find_last_of("/\\"));
        }
    }
    else {
        mtl_search_path = config.m_mtlSearchPath;
    }


    bool valid = tinyobj::loadObj(&m_vertexAttributes, &m_shapes, &m_materials, &warning, &error,
        filename.c_str(), mtl_search_path.c_str(),
        config.m_triangulate, config.m_parseVertexColor);

#ifdef DEBUG_MODE
    if (!warning.empty()) {
        logWarning(warning);
    }

    if (!error.empty()) {
        logError(error);
    }

    if (m_shapes.size() != m_materials.size() && m_materials.size() > 0) {
        // See TODO in tiny_obj_loader.h
        throw("Error, number of shapes does not match number of materials, enforce this in tiny_obj_reader");
    }
#endif

    return valid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ObjReader::parseFromString(const std::string & obj_text,
    const std::string & mtl_text,
    const tinyobj::ObjReaderConfig & config)
{
    std::stringbuf obj_buf(obj_text);
    std::stringbuf mtl_buf(mtl_text);

    std::istream obj_ifs(&obj_buf);
    std::istream mtl_ifs(&mtl_buf);

    tinyobj::MaterialStreamReader mtl_ss(mtl_ifs);

    std::vector<tinyobj::shape_t> shapes;
    std::vector<MaterialData> materials;
    std::string warning;
    std::string error;

    bool valid = tinyobj::loadObj(&m_vertexAttributes, &shapes, &materials, &warning, &error,
        &obj_ifs, &mtl_ss, config.m_triangulate, config.m_parseVertexColor);

    return valid; 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjReader::processVertex(const tinyobj::index_t& index,
    VertexArrayData& outMesh)
{

    // Get vertex coordinate
    const Vector3f& position = m_vertexAttributes.m_vertices[index.m_vertexIndex];

    // Get texture
    const Vector2f& uvCoord = m_vertexAttributes.m_texCoords[index.m_texCoordIndex];

    // Get normal
    const Vector3f& normal = m_vertexAttributes.m_normals[index.m_normalIndex];

    // Create vertex
    outMesh.m_attributes.m_vertices[index.m_vertexIndex] = position;
    outMesh.m_attributes.m_texCoords[index.m_vertexIndex] = uvCoord;
    outMesh.m_attributes.m_normals[index.m_vertexIndex] = normal;

    // Append index to indices list
    outMesh.m_indices.emplace_back(index.m_vertexIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
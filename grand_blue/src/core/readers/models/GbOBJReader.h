/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_OBJ_READER_H
#define GB_OBJ_READER_H

// QT
#include <QObject>
#include <QOpenGLBuffer>

// Internal
#include "../GbFileReader.h"
#include "../../rendering/geometry/GbVertexData.h"
#include "../../rendering/materials/GbMaterial.h"
#include "../../rendering/geometry/GbMesh.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

namespace tinyobj {
struct shape_t;
struct index_t;
struct ObjReaderConfig;
}

namespace Gb {  

class ResourceCache;
class Mesh;
class Model;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class ObjReader
/// @brief For loading in 3D model data from a .obj file
// TODO: Remove, has been replaced by ASSIMP
class ObjReader: public QObject, public FileReader {
     Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{    

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructor/Destructpr
    /// @{    

    ObjReader(ResourceCache* cache, const QString& filepath, const QString& textureFilepath = QString());
    ~ObjReader();

    /// @}   

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Get materials
    std::vector<MaterialData>& materials() { return m_materials; }

    ///// @brief Load obj file into the given mesh
    //void loadModel(Mesh& mesh);

    /// @brief Load materials into the resource cache
    void loadMaterials();

    /// @brief Parse actual file
    void parseFile();

    /// @}   

signals:
    void loadedOBJ(std::shared_ptr<Mesh> mesh);

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private methods
    /// @{    

    /// @brief Construct mesh data after parsing from a file
    //void loadMesh(Mesh& outMesh);

    /// @brief Use tiny_obj_loader to load .obj and .mtl from a file.
    /// @param[in] filename wavefront .obj filename
    /// @param[in] config Reader configuration
    ///
    bool parseFromFile(const std::string &filename, const tinyobj::ObjReaderConfig &config);


    ///
    /// Parse .obj from a text string.
    /// Need to supply .mtl text string by `mtl_text`.
    /// This function ignores `mtllib` line in .obj text.
    ///
    /// @param[in] obj_text wavefront .obj filename
    /// @param[in] mtl_text wavefront .mtl filename
    /// @param[in] config Reader configuration
    ///
    bool parseFromString(const std::string &obj_text, const std::string &mtl_text, const tinyobj::ObjReaderConfig &config);


    /// @brief Create a vertex from a vertex index, texture index, and normal index
    /// @note e.g. "12/7/5"
    void processVertex(const tinyobj::index_t& index, VertexArrayData& outMesh);

    /// @}   

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{    

    /// @brief Pointer to the resource cache used to store model data
    ResourceCache* m_resourceCache;

    /// @brief Filepath to the texture image for this model
    QString m_textureFilePath;

    /// @brief Cached vertex data from .obj file read
    VertexAttributes m_vertexAttributes;
    std::vector<tinyobj::shape_t> m_shapes;

    /// @brief Cached materials from .obj file read
    std::vector<MaterialData> m_materials;

    /// @}   
};

        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
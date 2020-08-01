/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_MODEL_READER_H
#define GB_MODEL_READER_H

// External
#include <assimp/Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

// QT
#include <QObject>
#include <QOpenGLBuffer>

// Internal
#include "../GbFileReader.h"
#include "../../rendering/geometry/GbVertexData.h"
#include "../../rendering/materials/GbMaterial.h"
#include "../../geometry/GbCollisions.h"

/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {  

class ResourceHandle;
class ResourceCache;
class Mesh;
class Model;
class VertexArrayData;
class Bone;
class Animation;
class SkeletonJoint;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct ModelChunkData
struct ModelChunkData {
    Uuid m_meshHandleID;
    Uuid m_matHandleID;
    AABB m_boundingBox;
};


/// @class ModelReader
/// @brief For loading in 3D model data from a .obj file
/// @note See:
/// https://www.ics.com/blog/qt-and-opengl-loading-3d-model-open-asset-import-library-assimp
/// https://learnopengl.com/Model-Loading/Assimp
/// https://learnopengl.com/Model-Loading/Mesh
/// https://learnopengl.com/Model-Loading/Model
/// https://github.com/Jas03x/GL_GameKit/blob/436a7921f0c342de681732f6dd850277626fc03a/src/ColladaLoader.cpp
class ModelReader: public QObject, public FileReader {
     Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Static
    /// @{    

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructor/Destructpr
    /// @{    

    ModelReader(ResourceCache* cache, ResourceHandle& handle);
    ModelReader(const ModelReader& reader);
    ~ModelReader();

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{ 

    const aiScene* scene() const { return m_scene; }

    std::vector<ModelChunkData>& chunks() { return m_chunks; }

    /// @}

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{    

    /// @brief Load in a file
    void loadFile();
    
    /// @brief Retrieve a model at the given filepath (loading only if not loaded)
    std::shared_ptr<Model> loadModel();

    /// @}   

signals:

protected:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private methods
    /// @{    

    /// @brief Load materials into the resource cache
    void loadMaterials();

    /// @brief Construct mesh data after parsing from a file
    void loadMeshes();

    /// @brief load animations
    void loadAnimations();

    /// @brief Process mesh data from a assimp mesh
    const Mesh& processMesh(aiMesh* mesh);

    /// @brief Process material
    MaterialData processMaterial(aiMaterial* mat);

    /// @brief Calculate the object-space transform of the node
    Matrix4x4g calculateGlobalNodeTransform(const aiNode* root);

    /// @brief Process texture type from material
    void processTexture(aiMaterial* mat, MaterialData& outMaterial, aiTextureType type);

    /// @brief Process nodes for metadata
    void processNodes(const aiNode* node);

    /// @brief Recursive method for walking aiMesh to construct output
    void parseNodeHierarchy(aiNode* aiNode, SkeletonJoint* currentNode);

    /// @brief Add animation data to skeleton if necessary
    void postProcessSkeleton();
    void postProcessSkeleton(SkeletonJoint* joint);

    /// @brief Convert an assimp matrix to a custom matrix
    Matrix4x4g toMatrix(const aiMatrix4x4& mat);

    std::shared_ptr<Model> model();

    /// @}   

    //-----------------------------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{    

    /// @brief The resource handle that the model is being loaded into
    ResourceHandle* m_handle;

    /// @brief Resource cache
    ResourceCache* m_resourceCache;

    /// @brief Assimp importer
    Assimp::Importer m_importer;

    /// @brief Assimp scene
    const aiScene* m_scene;

    /// @brief Vector of node names
    std::vector<QString> m_nodeNames;

    /// @brief Vector of model chunk data
    std::vector<ModelChunkData> m_chunks;

    /// @brief Map of bones, according to node/bone name
    std::unordered_map<QString, Bone*> m_boneMapping;

    /// @brief Map of meshes, by name and index in scene mesh list
    std::unordered_map<uint, QString> m_meshNames;

    /// @brief Count of animated joints
    int m_animatedJointCount = 0;

    /// @brief Vector of animations
    std::vector<std::shared_ptr<Animation>> m_animations;

    bool m_processedSkeleton = false;

    /// @brief Global inverse transformation
    Matrix4x4g m_globalInverseTransform;

    /// @}   
};

        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif
#pragma once

// Internal
#include "core/rendering/buffers/GVertexArrayObject.h"
#include "core/rendering/buffers/GGlBuffer.h"
#include "core/resource/GResourceHandle.h"
#include "fortress/containers/math/GMatrix.h"
#include "core/animation/GAnimation.h"
#include "fortress/containers/GContainerExtensions.h"
#include "heave/collisions/GCollisions.h"

namespace rev {

class Model;
class Skeleton;
class CoreEngine;
class RenderContext;
enum class PrimitiveMode;

/// @class VertexArrayData
/// @brief Class representing a set of vertex data, i.e., a VAO and it's managed VBOs
class VertexArrayData {
public:
    /// @name Constructors/Destructor
    /// @{
    VertexArrayData();
    virtual ~VertexArrayData();

    /// @}

    /// @name Properties 
    /// @{

    /// @brief Return vertex array object for this VAO group
    gl::VertexArrayObject& vao() { return m_vao; }

    /// @}

    /// @name Methods 
    /// @{

    /// @brief Perform draw of geometry
    void drawGeometry(PrimitiveMode glMode, int instanceCount) const;

    /// @brief Initialize the data for this mesh in the GL VAO
    void loadIntoVAO(RenderContext& context, const MeshVertexAttributes& data);

    /// @brief Whether or not the VAO has buffers
    bool hasBuffers() const;

    /// @brief Destroy all buffers in the VAO
    void destroyBuffers();

    /// @brief Release vertex and index buffers
    void release();

    /// @brief Returns the size of the data stored in the mesh vertices
    quint64 sizeInBytes() const;
    unsigned int sizeInMegaBytes() const;

    GlVertexBufferObject& getBuffer(MeshVertexAttributeType type);

    /// @}

    /// @name Members 
    /// @{

    gl::VertexArrayObject m_vao; ///< OpenGL VAO, aggregates buffers, but does not store data within itself
    std::array<GlVertexBufferObject, (size_t)MeshVertexAttributeType::kMAX_ATTRIBUTE_TYPE> m_attributeBuffers; ///< The buffers storing data for OpenGL
    gl::BufferStorageMode m_usagePattern = gl::BufferStorageMode::kStaticDraw;
    Uint64_t m_sizeInBytes{ 0 }; ///< Size of encapsulated VAO + VBOs in bytes
    Uint32_t m_vertexCount{ 0 }; ///< The number of vertices
    Uint32_t m_indexCount{ 0 }; ///< The number of indices to draw

    /// @}

private:
    /// @name Friends

    friend class CubeMap;
    friend class ModelReader;
    friend class Animation;
    friend class NodeAnimation;
    friend class Mesh;
    friend class SkeletonJoint;

    /// @}

    /// @name Private methods 
    /// @{

    /// @brief Generate VAO and its buffers
    /// @detailed Loads attribute data into the currently bound VAO
    /// @see https://www.khronos.org/opengl/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)
    template<typename VertexAttributesType>
    void loadAttributes() {
        // Load buffer attributes, except indices
        constexpr Int32_t indicesIndex = (Int32_t)VertexAttributesType::EnumType::kIndices;
        for (size_t i = 0; i < indicesIndex; i++) {
            GlVertexBufferObject& buffer = m_attributeBuffers[i];
            if (buffer.hasValidId()) {
                m_vao.loadAttributeBuffer<VertexAttributesType>(buffer, i);
            }
        }

        // Need to bind index buffer to associate it with VAO
        constexpr Int32_t indicesBufferIndex = (Int32_t)VertexAttributesType::EnumType::kIndices;
        m_attributeBuffers[indicesBufferIndex].bind();
    }

    /// @brief Generate VBOS (buffers), and load data
    void loadBufferData(RenderContext& context, const MeshVertexAttributes& data);

    /// @brief Create a buffer from the given buffer data type
    template<MeshVertexAttributeType AttributeType>
    void createAndPopulateBuffer(RenderContext& context, const MeshVertexAttributes& attributes, gl::BufferType bufferType) {
        constexpr Int32_t attributeIndex = (Int32_t)AttributeType;
        const auto& attibuteArray = attributes.get<AttributeType>();
        Int32_t attributeCount = attibuteArray.size();
        if (attributeCount) {
            m_attributeBuffers[attributeIndex] = GlVertexBufferObject(bufferType, attributes.getAttributeSizeInBytes<AttributeType>(), m_usagePattern);
            m_attributeBuffers[attributeIndex].setRenderContext(context);
            m_attributeBuffers[attributeIndex].allocateMemory(&attibuteArray[0], attributeCount);
        }
    }


    /// @brief Struct to wrap functor that will populate every buffer type
    template<MeshVertexAttributeType EnumValue>
    struct PopulateBufferStruct {
        void operator()(VertexArrayData* data, RenderContext& context, const MeshVertexAttributes& attributes) const {
            data->createAndPopulateBuffer<EnumValue>(context, attributes, gl::BufferType::kVertexBuffer);
        }
    };

    /// @}
};


} // End namespaces


// Includes

#include "core/rendering/geometry/GVertexData.h"


// Class Definitions


namespace rev {

quint64 VertexAttributes::getSizeInBytes() const
{
    quint64 len = 0;
    len += m_texCoords.size() * sizeof(Vector2);

    len += m_vertices.size() * sizeof(Vector3);
    len += m_normals.size() * sizeof(Vector3);
    len += m_tangents.size() * sizeof(Vector3);

    len += m_colors.size() * sizeof(Vector4);
    len += m_miscReal.size() * sizeof(Vector4);

    len += m_miscInt.size() * sizeof(Vector4i);

    return len;
}

bool VertexAttributes::empty() const {
    return m_vertices.empty();
}

void VertexAttributes::clear()
{
    m_vertices.clear();
    m_normals.clear();
    m_texCoords.clear();
    m_colors.clear();
    m_tangents.clear();
    m_miscInt.clear();
    m_miscReal.clear();
}


// End namespaces
}
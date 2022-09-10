#include "core/resource/GResource.h"
#include "core/resource/GResourceCache.h"

namespace rev {

const GString & Resource::ResourceTypeDirName(GResourceType type)
{
    if ((int)type < 0) {
        Logger::Throw("Error, invalid type");
    }
    return s_resourceDirNames[(size_t)type];
}

Resource::Resource() :
    m_cost(sizeof(Resource)) // default cost
{
}

Resource::~Resource()
{
}

bool Resource::loadBinary(const GString& filepath)
{
    return false;
}

bool Resource::saveBinary(const GString& filepath) const
{
    return false;
}

void Resource::postConstruction(const ResourcePostConstructionData& /*postConstructData*/)
{
}

std::array<GString, (size_t)EResourceType::eUserType + 1> Resource::s_resourceDirNames =
{ {
    "images",
    "textures",
    "materials",
    "meshes",
    "cube_textures",
    "animations",
    "models",
    "shaders",
    "scripts",
    "skeletons",
    "audio"
} };

} // End namespaces
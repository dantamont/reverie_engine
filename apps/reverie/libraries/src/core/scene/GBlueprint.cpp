#include "core/scene/GBlueprint.h"

#include <core/scene/GSceneObject.h>

namespace rev{


Blueprint::Blueprint()
{
}

Blueprint::Blueprint(const SceneObject & sceneObject):
    NameableInterface(sceneObject.getName()),
    m_soJson(sceneObject)
{
}

Blueprint::~Blueprint()
{
}

Blueprint & Blueprint::operator=(const Blueprint & other)
{
    m_name = other.m_name;
    m_soJson = other.m_soJson;
    return *this;
}

void to_json(json& orJson, const Blueprint& korObject)
{
    orJson["so"] = korObject.m_soJson;
    orJson["name"] = korObject.m_name.c_str();
    orJson["uuid"] = korObject.getUuid(); // For widgets, not used on reload from file
}

void from_json(const json& korJson, Blueprint& orObject)
{
    orObject.m_name = korJson["name"].get_ref<const std::string&>().c_str();
    orObject.m_soJson = korJson["so"];
}


// End namespaces        
}
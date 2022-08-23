
// Includes

#include "fortress/types/GLoadable.h"

// Standard Includes

// External

// Project
#include <fortress/json/GJson.h>

// Namespace Definitions
namespace rev {


void to_json(json& orJson, const LoadableInterface& korObject)
{
    orJson = { {JsonKeys::s_filepath, korObject.m_path} };
}

void from_json(const json& korJson, LoadableInterface& orObject)
{
    orObject.m_path = korJson.at(JsonKeys::s_filepath).get<GString>();
}

void to_json(json& orJson, const DistributedLoadableInterface& korObject)
{
    // Create JSON from base class
    orJson = static_cast<const LoadableInterface&>(korObject);
    if (korObject.m_additionalPaths.size()) {
        orJson["additionalPaths"] = json::array();
        for (const GString& path : korObject.m_additionalPaths) {
            orJson["additionalPaths"].push_back(path.c_str());
        }
    }
}

void from_json(const json& korJson, DistributedLoadableInterface& orObject)
{
    FromJson<LoadableInterface>(korJson, orObject);
    if (korJson.contains("additionalPaths")) {
        const json& paths = korJson["additionalPaths"];
        for (const auto& pathJson : paths) {
            orObject.m_additionalPaths.push_back(pathJson.get_ref<const std::string&>().c_str());
        }
    }
}


} // end namespacing

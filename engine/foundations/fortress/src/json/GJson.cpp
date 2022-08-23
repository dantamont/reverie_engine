#include "fortress/json/GJson.h"
#include "fortress/system/path/GPath.h"

namespace rev {

void GJson::FromFile(const char* filePath, json& outJson)
{
#ifdef DEBUG_MODE
    assert(GPath::Exists(filePath) && "Error, file not found");
#endif

    std::ifstream ifs(filePath);
    outJson = json::parse(ifs);
}

} // End namespace rev
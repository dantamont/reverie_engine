#include "core/canvas/GIcon.h"
#include "geppetto/qt/fonts/GFontManager.h"

namespace rev {


Icon::Icon(CanvasComponent* canvas, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t worldMatrixIndex) :
    Label(canvas, worldMatrixVec, worldMatrixIndex, kIcon, 40, "free-solid-900", u8"\uf041", ResourceBehaviorFlag::kRuntimeGenerated),
    m_iconName("map-marker")
{
}


Icon::~Icon()
{
}

void Icon::setFontAwesomeIcon(const QString & iconName)
{
    m_iconName = iconName;
    setText(FontManager::FaUnicodeCharacter(iconName.toStdString()).string().c_str());
}

void to_json(json& orJson, const Icon& korObject)
{
    ToJson<Label>(orJson, korObject);
    orJson["iconName"] = korObject.m_iconName.toStdString().c_str();
}

void from_json(const json& korJson, Icon& orObject)
{
    FromJson<Label>(korJson, orObject);
    orObject.m_iconName = korJson.value("iconName", "angry").c_str();
}


} // End namespaces

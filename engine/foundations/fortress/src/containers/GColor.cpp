#include "fortress/containers/GColor.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

const Color& Color::White() {
    static Color s_white{ 1.0F, 1.0F, 1.0F, 1.0F };
    return s_white;
}


Color::Color()
{
}

Color::Color(const std::vector<Real_t>& vec)
{
    m_r = vec[0];
    m_g = vec[1];
    m_b = vec[2];
    m_a = vec.size() > 3 ? vec[3] : 1.0;
}

Color::Color(const std::vector<int>& vec) :
    Color(vec[0], vec[1], vec[2], vec.size() > 3 ? vec[3] : (int)255)
{
}

Color::Color(const Vector3 & vec):
    m_r(vec[0]),
    m_g(vec[1]),
    m_b(vec[2]),
    m_a(1.0F)
{
}

Color::Color(const Vector4 & vec) :
    m_r(vec[0]),
    m_g(vec[1]),
    m_b(vec[2]),
    m_a(vec[3])
{
}

Color::Color(int r, int g, int b, int a):
    m_r(ToFloatColor(r)),
    m_g(ToFloatColor(g)),
    m_b(ToFloatColor(b)),
    m_a(ToFloatColor(a))
{
}

Color::Color(Real_t r, Real_t g, Real_t b, Real_t a) :
    m_r(r),
    m_g(g),
    m_b(b),
    m_a(a)
{
}

Color::~Color()
{
}

void to_json(nlohmann::json& orJson, const Color& korObject)
{
    orJson = { korObject.m_r, korObject.m_g, korObject.m_b, korObject.m_a};
}

void from_json(const nlohmann::json& korJson, Color& orObject)
{
    orObject.m_r = korJson.at(0);
    orObject.m_g = korJson.at(1);
    orObject.m_b = korJson.at(2);
    if (korJson.size() >= 4) {
        orObject.m_a = korJson.at(3);
    }
    else {
        orObject.m_a = 1.0F;
    }
}

} // End namespaces

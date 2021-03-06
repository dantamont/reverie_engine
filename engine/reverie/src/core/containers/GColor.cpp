#include "GColor.h"

namespace rev {
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color():  QColor(255, 255, 255, 255)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(const std::vector<real_g>& vec): QColor()
{
    *this = fromRgbF(vec[0], vec[1], vec[2], vec.size() > 3 ? vec[3] : 1.0f);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(const std::vector<int>& vec) :
    QColor(vec[0], vec[1], vec[2], vec.size() > 3 ? vec[3]: 255)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(const Vector3 & vec): QColor()
{
    *this = fromRgbF(vec[0], vec[1], vec[2], vec.size() > 3 ? vec[3] : 1.0f);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(const Vector4 & vec) :  QColor()
{
    *this = fromRgbF(vec[0], vec[1], vec[2], vec.size() > 3 ? vec[3] : 1.0f);
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(int r, int g, int b, int a):
    QColor(r, g, b, a)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::Color(const QColor & color):
    QColor(color)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Color::~Color()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
std::vector<int> Color::toVector4i() const
{
    return std::vector<int>({QColor::red(), QColor::green(), QColor::blue(), QColor::alpha()});
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector4 Color::toVector4g() const
{
    return Vector4(QColor::redF(), greenF(), blueF(), alphaF());
}

/////////////////////////////////////////////////////////////////////////////////////////////
Vector3 Color::toVector3g() const
{
    return Vector3(QColor::redF(), greenF(), blueF());
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
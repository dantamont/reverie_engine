#include "fortress/containers/math/GSize.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

Vector2i GSize::toVector2()
{
    return Vector2i(m_width, m_height);
}


} /// End rev namespace

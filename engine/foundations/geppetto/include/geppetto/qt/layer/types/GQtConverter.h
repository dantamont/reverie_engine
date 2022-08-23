#pragma once

//// Includes
#include <vector>

class QUuid;
class QColor;
class QPixmap;
class QImage;
class QString;

namespace rev {

template<class D, size_t R, size_t C> class Matrix;
typedef Matrix<float, 4, 4> Matrix4x4;
class Uuid;
class Color;
class Image;
class GString;

/// @class QConverter
/// @brief Class for converting a variety of containers to their Qt variants, as well as operating on Qt containers with custom types
/// @see https://github.com/nlohmann/json/issues/2216 for emulating QJsonValue
class QConverter
{
public:

    /// @brief Convert GString to QString
    static QString ToQt(const GString& str);

    /// @brief Convert QString to GString
    static GString FromQt(const QString& str);

    /// @brief  Convert Uuid to Quuid
    /// @return 
    static QUuid ToQt(const Uuid& uuid);

    /// @brief Convert Quuid to Uuid
    static Uuid FromQt(const QUuid& quuid);

    /// @brief Convert Color to QColor
    static QColor ToQt(const Color& color);

    /// @brief Convert QColor to Color
    static Color FromQt(const QColor& color);

    /// @brief Convert an Image to a QImage
    /// @note The QImage does not copy the image buffer data, so the original image must outlast the created QImage
    /// @note QImage's loading routines are not thread-safe
    static QImage ToQt(const Image& image);

    /// @brief Set pixmap to a given color
    static QPixmap SetPixmapColor(const QPixmap& pixmap, const Color& color);
};



} // End namespaces

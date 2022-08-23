#include "geppetto/qt/layer/types/GQtConverter.h"
#include <QUuid>
#include <QString>
#include <QColor>
#include <QPixmap>
#include <QImage>
#include "fortress/types/GString.h"
#include "fortress/containers/GColor.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/encoding/uuid/GUuid.h"
#include "fortress/image/GImage.h"

namespace rev {

QString QConverter::ToQt(const GString& str)
{
    return str.c_str();
}

GString QConverter::FromQt(const QString& str)
{
    return GString(str.toStdString());
}

QUuid QConverter::ToQt(const Uuid& uuid)
{
    const std::array<Uint8_t, 8>& data4 = uuid.data4();
    return QUuid(uuid.data1(), uuid.data2(), uuid.data3(), 
        data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
}

Uuid QConverter::FromQt(const QUuid& quuid)
{
    return Uuid(quuid.toString().toStdString());
}

QColor QConverter::ToQt(const Color& color)
{
    return QColor::fromRgbF(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

Color QConverter::FromQt(const QColor& color)
{
    return Color((Real_t)color.redF(), (Real_t)color.greenF(), (Real_t)color.blueF(), (Real_t)color.alphaF());
}

QImage QConverter::ToQt(const Image& image)
{
    return QImage(image.buffer(), image.width(), image.height(), image.bytesPerLine(), (QImage::Format)image.format());
}

QPixmap QConverter::SetPixmapColor(const QPixmap& pixmap, const Color& color)
{
    // Convert the pixmap to QImage
    QImage tmp = pixmap.toImage();

    // Loop all the pixels
    QColor colorTmp = QConverter::ToQt(color);
    for (Int32_t y = 0; y < tmp.height(); y++)
    {
        for (Int32_t x = 0; x < tmp.width(); x++)
        {
            // Read the alpha value each pixel, keeping the RGB values of your color
            colorTmp.setAlpha(tmp.pixelColor(x, y).alpha());

            // Apply the pixel color
            tmp.setPixelColor(x, y, colorTmp);
        }
    }

    // Get the coloured pixmap
    return QPixmap::fromImage(tmp);
}

} // End namespaces

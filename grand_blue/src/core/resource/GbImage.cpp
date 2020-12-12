#include "GbImage.h"
#include "GbResourceCache.h"

#include <QDir>
#include <QDirIterator>
#include "GbImage.h"
#include "../containers/GbColor.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb/stb_image.h"

namespace Gb {
/////////////////////////////////////////////////////////////////////////////////////////////
QPixmap Image::SetPixmapColor(const QPixmap& pixmap, const Color & color)
{
    // Convert the pixmap to QImage
    QImage tmp = pixmap.toImage();

    // Loop all the pixels
    QColor colorTmp = color;
    for (int y = 0; y < tmp.height(); y++)
    {
        for (int x = 0; x < tmp.width(); x++)
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

/////////////////////////////////////////////////////////////////////////////////////////////
void Image::Convert32bitARGBtoRGBA(Image& image)
{
    if (image.m_image.isNull()) return;
    QImage& q = image.m_image;
    uint32_t count = 0, max = (uint32_t)(q.height()*q.width());
    uint32_t* p = (uint32_t*)(q.bits());
    uint32_t n;
    while (count < max)
    {
        n = p[count];   //n = ARGB
        p[count] = 0x00000000 |
            ((n << 8) & 0x0000ff00) |
            ((n << 8) & 0x00ff0000) |
            ((n << 8) & 0xff000000) |
            ((n >> 24) & 0x000000ff);
        // p[count] = RGBA
        count++;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Image::Convert32bitRGBAtoARGB(Image& image)
{
    if (image.m_image.isNull()) return;
    QImage& q = image.m_image;
    uint32_t count = 0, max = (uint32_t)(q.height()*q.width());
    uint32_t* p = (uint32_t*)(q.bits());
    uint32_t n;
    while (count < max)
    {
        n = p[count];   //n = RGBA
        p[count] = 0x00000000 |
            ((n >> 8) & 0x000000ff) |
            ((n >> 8) & 0x0000ff00) |
            ((n >> 8) & 0x00ff0000) |
            ((n << 24) & 0xff000000);
        // p[count] = ARGB
        count++;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image()
{
    m_cost = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const uchar * data, int width, int height, QImage::Format format)
{
    m_image = QImage(data, width, height, format);
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const QString & filepath, QImage::Format format)
{
    bool exists = QFile(filepath).exists();
    if (!exists) {
#ifdef DEBUG_MODE
        logError("Error, image does not exist");
        throw("Error, image does not exist");
#endif
    }

    // QImage loading was not thread-safe, so just going to use stbi_load for EVERYTHING
    //if (filepath.endsWith(".tga")) {
    bool success;
    loadImage(filepath.toStdString().c_str(), success);
    if (!success) {
#ifdef DEBUG_MODE
        logWarning("TGA image failed to load");
#endif
    }
    //}
    //else {
    //    m_image = QImage(filepath);
    //}

    if (m_image.isNull()) {
        logCritical("Warning, image failed to load");
    }

    if (format != QImage::Format_Invalid) {
        // Reformat image
        m_image = std::move(m_image.convertToFormat(format));
    }
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const Image & image):
    m_image(image.m_image)
{
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const QImage & image):
    m_image(image)
{
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::~Image()
{
    m_image = QImage();
    //if (m_imageBuffer) {
        // Free image buffer manually
        //stbi_image_free(m_imageBuffer);
    //}
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image& Image::operator=(const Image & image)
{
    m_image = image.m_image;
    m_cost = image.m_cost;

    return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Image::onRemoval(ResourceCache* cache)
{
    Q_UNUSED(cache)
    m_image = QImage();
}
/////////////////////////////////////////////////////////////////////////////////////////////
size_t Image::sizeInMegaBytes() const
{
    return ceil(m_image.sizeInBytes() / (1000 * 1000.0));
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Image::save(const QString & filename, const char * format, int quality)
{
    return m_image.save(filename, format, quality);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Image::loadImage(const GString& filePath, bool &success)
{
    // TODO: Cache buffer directly to file for read, saves processing
    // Load in RGBA with alpha channel
    int w, h, comp;
    if (m_imageBuffer) {
        throw("Should not have image buffer prior to load");
    }
    m_imageBuffer = stbi_load(filePath.c_str(), &w, &h, &comp, STBI_rgb_alpha);
    m_image = QImage(m_imageBuffer, w, h, QImage::Format_RGBA8888, 
        CleanUpHandler, m_imageBuffer);

    if (!m_imageBuffer) {
        GString failMessage(stbi_failure_reason());
        success = false;
        throw(failMessage);
    }
    else {
        success = true;
    }

    //QString name = QString(filePath).split("/").back();
    //QString path = "C:/Users/dante/Documents/Projects/grand-blue-engine/out/img/" + name;
    //img.save(path);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Image::CleanUpHandler(void * imageBuffer)
{
    unsigned char* buffer = static_cast<unsigned char*>(imageBuffer);
    if (buffer) {
        stbi_image_free(imageBuffer);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
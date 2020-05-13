#include "GbImage.h"
#include "GbResourceCache.h"

#include <QDir>
#include <QDirIterator>
#include "GbImage.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image():
    Resource(kImage)
{
    m_cost = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const uchar * data, int width, int height, QImage::Format format):
    Resource(kImage)
{
    m_image = QImage(data, width, height, format);
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const QString & filepath, QImage::Format format) :
    Resource(kImage)
{
    bool exists = QFile(filepath).exists();
    if (!exists) {
#ifdef DEBUG_MODE
        logError("Error, image does not exist");
        throw("Error, image does not exist");
#endif
    }

    if (filepath.endsWith(".tga")) {
        bool success;
        m_image = loadTga(filepath.toStdString().c_str(), success);
        if (!success) {
#ifdef DEBUG_MODE
            logWarning("TGA image failed to load");
#endif
        }
    }
    else {
        m_image = QImage(filepath);
    }

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
    Resource(kImage),
    m_image(image.m_image)
{
    m_cost = sizeInMegaBytes();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image::Image(const QImage & image):
    Resource(kImage),
    m_image(image)
{
    m_cost = sizeInMegaBytes();
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
QImage Image::loadTga(const char* filePath, bool &success) const
{
    QImage img;
    //if (!img.load(filePath))
    {

        // open the file
        std::fstream fsPicture(filePath, std::ios::in | std::ios::binary);

        if (!fsPicture.is_open())
        {
            img = QImage(1, 1, QImage::Format_RGB32);
            img.fill(Qt::red);
            success = false;
            return img;
        }

        // some variables
        std::vector<std::uint8_t> vui8Pixels;
        std::uint32_t ui32BpP;
        std::uint32_t ui32Width;
        std::uint32_t ui32Height;

        // read in the header
        std::uint8_t ui8x18Header[19] = { 0 };
        fsPicture.read(reinterpret_cast<char*>(&ui8x18Header), sizeof(ui8x18Header) - 1);

        //get variables
        bool bCompressed;
        std::uint32_t ui32IDLength;
        std::uint32_t ui32PicType;
        std::uint32_t ui32PaletteLength;
        std::uint32_t ui32Size;

        // extract all information from header
        ui32IDLength = ui8x18Header[0];
        ui32PicType = ui8x18Header[2];
        ui32PaletteLength = ui8x18Header[6] * 0x100 + ui8x18Header[5];
        ui32Width = ui8x18Header[13] * 0x100 + ui8x18Header[12];
        ui32Height = ui8x18Header[15] * 0x100 + ui8x18Header[14];
        ui32BpP = ui8x18Header[16];

        // calculate some more information
        ui32Size = ui32Width * ui32Height * ui32BpP / 8;
        bCompressed = ui32PicType == 9 || ui32PicType == 10;
        vui8Pixels.resize(ui32Size);

        // jump to the data block
        fsPicture.seekg(ui32IDLength + ui32PaletteLength, std::ios_base::cur);

        if (ui32PicType == 2 && (ui32BpP == 24 || ui32BpP == 32))
        {
            fsPicture.read(reinterpret_cast<char*>(vui8Pixels.data()), ui32Size);
        }
        // else if compressed 24 or 32 bit
        else if (ui32PicType == 10 && (ui32BpP == 24 || ui32BpP == 32))	// compressed
        {
            std::uint8_t tempChunkHeader;
            std::uint8_t tempData[5];
            unsigned int tempByteIndex = 0;

            do {
                fsPicture.read(reinterpret_cast<char*>(&tempChunkHeader), sizeof(tempChunkHeader));

                if (tempChunkHeader >> 7)	// repeat count
                {
                    // just use the first 7 bits
                    tempChunkHeader = (uint8_t(tempChunkHeader << 1) >> 1);

                    fsPicture.read(reinterpret_cast<char*>(&tempData), ui32BpP / 8);

                    for (int i = 0; i <= tempChunkHeader; i++)
                    {
                        vui8Pixels.at(tempByteIndex++) = tempData[0];
                        vui8Pixels.at(tempByteIndex++) = tempData[1];
                        vui8Pixels.at(tempByteIndex++) = tempData[2];
                        if (ui32BpP == 32) vui8Pixels.at(tempByteIndex++) = tempData[3];
                    }
                }
                else						// data count
                {
                    // just use the first 7 bits
                    tempChunkHeader = (uint8_t(tempChunkHeader << 1) >> 1);

                    for (int i = 0; i <= tempChunkHeader; i++)
                    {
                        fsPicture.read(reinterpret_cast<char*>(&tempData), ui32BpP / 8);

                        vui8Pixels.at(tempByteIndex++) = tempData[0];
                        vui8Pixels.at(tempByteIndex++) = tempData[1];
                        vui8Pixels.at(tempByteIndex++) = tempData[2];
                        if (ui32BpP == 32) vui8Pixels.at(tempByteIndex++) = tempData[3];
                    }
                }
            } while (tempByteIndex < ui32Size);
        }
        // not useable format
        else
        {
            fsPicture.close();
            img = QImage(1, 1, QImage::Format_RGB32);
            img.fill(Qt::red);
            success = false;
            return img;
        }

        fsPicture.close();

        //img = QImage(ui32Width, ui32Height, QImage::Format_RGB888);

        int pixelSize = ui32BpP == 32 ? 4 : 3;
        // TODO: write direct into img
        // See: https://stackoverflow.com/questions/6084012/fast-image-manipulation
        unsigned char* buffer_;
        buffer_ = new unsigned char[4 * ui32Width * ui32Height];
        int valr, valg, valb, idx;
        for (unsigned int x = 0; x < ui32Width; x++)
        {
            for (unsigned int y = 0; y < ui32Height; y++)
            {
                idx = pixelSize * (y * ui32Width  + x);
                valr = vui8Pixels.at(idx + 2);
                valg = vui8Pixels.at(idx + 1);
                valb = vui8Pixels.at(idx);

                // TGA is BGR encoded
                buffer_[4 * (y * ui32Width + x)] = valb;
                buffer_[4 * (y * ui32Width + x) + 1] = valg;
                buffer_[4 * (y * ui32Width + x) + 2] = valr;

                //QColor value(valr, valg, valb);
                //img.setPixelColor(x, y, value);
            }
        }
        img = QImage(buffer_, ui32Width, ui32Height, QImage::Format_RGB32);
        img = img.mirrored();

    }
    success = true;
    return img;
}
/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces
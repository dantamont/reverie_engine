#include "fortress/image/GImage.h"

#include <stdexcept>
#include "fortress/GGlobal.h"
#include "fortress/containers/GColor.h"
#include "fortress/system/memory/GMemoryMonitor.h"
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GPath.h"
#include "fortress/string/GString.h"

// Include STB image
#define STB_IMAGE_IMPLEMENTATION
#include "fortress/image/extern/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "fortress/image/extern/stb/stb_image_write.h"

namespace rev {

Uint32_t Image::GetPixelBitCount(Image::ColorFormat fmt)
{
    assert(fmt != ColorFormat::kInvalid && "Error, unsupported pixel format");
    assert(fmt < ColorFormat::kEND && "Error, format out of bounds");
    return (Uint32_t)s_pixelByteCounts[(size_t)fmt];
}

Image::Image()
{
}

Image::Image(const Uint8_t* data, Int32_t width, Int32_t height, Int32_t bytesPerLine, ColorFormat format) :
    m_colorFormat(format),
    m_pixelSizeBytes(bytesPerLine / width),
    m_imageSize(width, height),
    m_imageBuffer(const_cast<Uint8_t*>(data)) 
{
    /// Register buffer without taking ownership of data
    RegisterBuffer(m_imageBuffer, 0);
}

Image::Image(const Uint8_t* data, Int32_t width, Int32_t height, ColorFormat format):
    m_colorFormat(format),
    m_pixelSizeBytes(4),
    m_imageSize(width, height),
    m_imageBuffer(const_cast<Uint8_t*>(data))
{
    /// Register buffer without taking ownership of data
    RegisterBuffer(m_imageBuffer, 0);
}

Image::Image(const GString& filepath, bool flipVertically, Image::ColorFormat format)
{
    // Check if file exists
    bool exists = GPath::Exists(filepath.c_str());
    if (!exists) {
#ifdef DEBUG_MODE
        std::cerr << ("Error, image does not exist");
        throw std::logic_error("Error, image does not exist");
#endif
    }

    // Load image
    bool success;
    loadImage(filepath.c_str(), success, format == ColorFormat::kInvalid ? StandardColorFormat : format, flipVertically);
    if (!success) {
#ifdef DEBUG_MODE
        std::cout << ("Image failed to load");
#endif
    }

    // Check if image is valid
    if (isNull()) {
        throw std::logic_error("Warning, image failed to load");
    }
}

Image::Image(const Image& image)
{
    *this = image;
}

Image::Image(const Image& other, bool deepCopy)
{
    if (deepCopy) {
        /// Copy buffer
        clear();
        if (other.m_imageBuffer) {
            other.copyBuffer(*this);
        }
        else {
            m_imageBuffer = nullptr;
        }

        /// Copy metadata
        other.copyMetadata(*this);
    }
    else {
        // Copy all metadata, but just reference buffer
        *this = other;
    }
}

Image::~Image()
{
    // Decrement reference count for image buffer
    clear();
}

Image& Image::operator=(const Image& other)
{
    /// Copy buffer
    clear();
    setBuffer(other.m_imageBuffer);

    /// Copy metadata
    other.copyMetadata(*this);

    return *this;
}

Image Image::convertToFormat(Image::ColorFormat format)
{
    if (m_colorFormat == format) {
        return Image(*this);
    }

    // Convert to RGBA8888 to convert to output type
    Image outImage = convertToRgba8888();

    // Convert to output type
    outImage = outImage.convertFromRgba8888(format);

    return outImage;
}

void Image::clear()
{
    if (nullptr != m_imageBuffer) {

        std::unique_lock lock(s_bufferMutex);

        // Decrement reference count
        ImageBufferInfo& bufferInfo = s_bufferCounts[m_imageBuffer];
        bufferInfo.m_referenceCount--;

#ifdef DEBUG_MODE
        assert(bufferInfo.m_referenceCount >= 0 && "Reference count cannot dip below zero");
#endif
        
        if (0 == bufferInfo.m_referenceCount) {
            bool freeNotDelete = bufferInfo.m_flags.testFlag(ImageBufferFlag::kFreeNotDelete);
            bool ownBuffer = bufferInfo.m_flags.testFlag(ImageBufferFlag::kOwnBuffer);

            // Delete buffer in static map
            s_bufferCounts.erase(m_imageBuffer);

            if (ownBuffer) {
                // Delete buffer if all images referencing it have gone out of scope
                if (freeNotDelete) {
                    // Free buffer if allocated using malloc
                    /// @note This is because STB loads in using mallocs. It's gross
                    free(m_imageBuffer); ///< Equivalent to stbi_image_free
                }
                else {
                    // Otherwise, delete it with array deletion
                    delete[] m_imageBuffer;
                }
            }
        }
    }
    m_imageBuffer = nullptr;
}

size_t Image::sizeInMegaBytes() const
{
    return ceil(sizeInBytes() / MemoryMonitor::s_numberOfBytesInMb);
}

size_t Image::sizeInBytes() const
{
    return m_pixelSizeBytes * m_imageSize.height() * m_imageSize.width();
}

Int32_t Image::bytesPerLine() const
{
    return m_pixelSizeBytes * m_imageSize.width();
}

Int32_t Image::pixelSizeBytes() const
{
    return m_pixelSizeBytes;
}

bool Image::save(const GString& filename, const char* format, Int32_t quality)
{
    // Write with a supported format if extension is recognized
    GFile imageFile(filename);
    GString ext = imageFile.extension();
    bool written;
    Int32_t numChannels = m_pixelSizeBytes;
    if (ext == "png") {
        written = stbi_write_png(filename.c_str(), m_imageSize.width(), m_imageSize.height(), numChannels, m_imageBuffer, bytesPerLine());
    }
    else if (ext == "jpg" || ext == "jpeg") {
        Int32_t imageQuality = quality >= 0? quality: 100;
        written = stbi_write_jpg(filename.c_str(), m_imageSize.width(), m_imageSize.height(), numChannels, m_imageBuffer, imageQuality);
    }
    else if (ext == "bmp") {
        written = stbi_write_bmp(filename.c_str(), m_imageSize.width(), m_imageSize.height(), numChannels, m_imageBuffer);
    }
    else if (ext == "tga") {
        written = stbi_write_tga(filename.c_str(), m_imageSize.width(), m_imageSize.height(), numChannels, m_imageBuffer);
    }
    else {
#ifdef DEBUG_MODE
        throw std::logic_error("Unrecognized extension, writing as PNG");
#else
        std::cout << "Unrecognized extension, writing as PNG";
#endif
        written = stbi_write_png(filename.c_str(), m_imageSize.width(), m_imageSize.height(), numChannels, m_imageBuffer, bytesPerLine());
    }


    if (written) {
#ifdef DEBUG_MODE
        std::string fileNameString(filename);
        throw std::logic_error("Error, failed to write image to file: " + fileNameString);
#else
        std::cout << ("Warning, image not written to file: " + filename);
#endif
    }

    return written;
}

void Image::loadImage(const GString& filePath, bool& success, ColorFormat format, bool flipVertically)
{
    // TODO: Cache buffer directly to file for read, saves processing
    // Load in RGBA with alpha channel
    Int32_t w;
    Int32_t h;
    if (m_imageBuffer) {
        throw std::logic_error("Should not have image buffer prior to load");
    }

    /// Get the correct STBI component count (number of bytes per pixel) from color format
    /// @note Limitations include no 1-bit BMP(Mono), no 16-bit-per-channel PNG, no 12-bit-per-channel JPEG
    /// @see https://www.cs.unh.edu/~cs770/lwjgl-javadoc/lwjgl-stb/org/lwjgl/stb/STBImage.html
    int originalNumChannels;
    int numChannels = 0;
    switch (format) {
    case ColorFormat::kGrayscale8:
        // One byte per pixel
        numChannels = STBI_grey;
        break;
    case ColorFormat::kGrayscaleAlpha88:
        // Two bytes per pixel
        numChannels = STBI_grey_alpha;
        break;
    case ColorFormat::kRGB888:
        // Three bytes per pixel
        numChannels = STBI_rgb;
        break;
    case ColorFormat::kRGBA8888:
        /// Four bytes per pixel
        numChannels = STBI_rgb_alpha;
        break;
    default:
#ifdef DEBUG_MODE
        throw std::logic_error("Invalid image format");
#endif
        std::cout << ("Invalid image format");
    }

    /// @todo Tweak STBI implementation to make this not suck. Clang doesn't support TLS (thread-local storage)
    stbi_set_flip_vertically_on_load_thread(flipVertically);
    m_imageBuffer = stbi_load(filePath.c_str(), &w, &h, &originalNumChannels, numChannels);

    /// Register buffer so it is cleared properly once out of scope
    ImageBufferInfo& info = RegisterBuffer(m_imageBuffer, ImageBufferFlag::kOwnBuffer);
    info.m_flags.setFlag(ImageBufferFlag::kFreeNotDelete);

    /// Set image metadata
    m_imageSize.setWidth(w);
    m_imageSize.setHeight(h);
    m_colorFormat = Image::StandardColorFormat;
    m_pixelSizeBytes = 0 == numChannels ? originalNumChannels : numChannels;

    if (!m_imageBuffer) {
        GString failMessage(stbi_failure_reason());
        success = false;
        throw std::logic_error(failMessage.c_str());
    }
    else {
        success = true;
    }

    //QString name = QString(filePath).split("/").back();
    //QString path = "C:/Users/dante/Documents/Projects/grand-blue-engine/out/img/" + name;
    //img.save(path);
}

Image Image::convertToRgba8888() const
{
    // Create empty image
    Image outImage(nullptr, m_imageSize.width(), m_imageSize.height(), ColorFormat::kRGBA8888);
    
    // Populate buffer
    switch (m_colorFormat) {
    case ColorFormat::kRGBA8888:
        copyBuffer(outImage);
        break;
    case ColorFormat::kARGB32:
        copyBuffer(outImage);
        ConvertBufferArgbToRgba8888(outImage);
        break;
    case ColorFormat::kRGB888:
        ConvertBufferRgb888ToRgba8888(outImage, m_imageBuffer);
        break;
    case ColorFormat::kRGB32:
        ConvertBufferRgb32ToRgba8888(outImage, m_imageBuffer);
        break;
    case ColorFormat::kGrayscale8:
        ConvertBufferGrayscale8ToRgba8888(outImage, m_imageBuffer);
        break;
    default:
    {
        std::string colorFormatStr(GString::FromNumber((Int32_t)m_colorFormat));
        throw std::logic_error("Image format unrecognized" + colorFormatStr);
    }
    }

#ifdef DEBUG_MODE
    assert(nullptr != outImage.m_imageBuffer && "Output image should have a buffer");
#endif
    
    return outImage;
}


Image Image::convertFromRgba8888(ColorFormat format)
{
    // Verify the color format of the image
    if (m_colorFormat != ColorFormat::kRGBA8888) {
        throw("Error, incorrect color format");
    }

    // Create empty image with the new format
    Image outImage(nullptr, m_imageSize.width(), m_imageSize.height(), format);

    // Update the buffer to reflect the new format
    switch (format) {
    case Image::ColorFormat::kRGBA8888:
        copyBuffer(outImage);
        break;
    case Image::ColorFormat::kARGB32:
        copyBuffer(outImage);
        ConvertBufferRgbaToArgb8888(outImage);
        break;
    case ColorFormat::kRGB888:
        ConvertBufferRgba8888ToRgb888(outImage, m_imageBuffer);
        break;
    case ColorFormat::kRGB32:
        ConvertBufferRgba8888ToRgb32(outImage, m_imageBuffer);
        break;
    case ColorFormat::kGrayscale8:
        ConvertBufferRgba8888ToGrayscale8(outImage, m_imageBuffer);
        break;
    default:
        std::string colorFormatStr(GString::FromNumber((Int32_t)format));
        throw std::logic_error("Image format unrecognized " + colorFormatStr);
    }

    return outImage;
}


void Image::ConvertBufferRgb888ToRgba8888(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer, and register in memory map
    static constexpr Uint32_t oldPixelSizeBytes = sizeof(Rgb888Pixel); /// 3
    static constexpr Uint32_t newPixelSizeBytes = sizeof(RgbaPixel); /// 4
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const Rgb888Pixel* oldBuffer = reinterpret_cast<const Rgb888Pixel*>(buffer);
    RgbaPixel* newBuffer = reinterpret_cast<RgbaPixel*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    Rgb888Pixel rgb;
    while (count < max)
    {
        rgb = oldBuffer[count];
        newBuffer[count] = ((rgb << 8) | 0x000000ff); // p[count] is RGBA
        count++;
    }
}

void Image::ConvertBufferRgba8888ToRgb888(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer
    static constexpr Uint32_t oldPixelSizeBytes = sizeof(RgbaPixel); /// 4
    static constexpr Uint32_t newPixelSizeBytes = sizeof(Rgb888Pixel); /// 3
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const RgbaPixel* oldBuffer = reinterpret_cast<const RgbaPixel*>(buffer);
    Rgb888Pixel* newBuffer = reinterpret_cast<Rgb888Pixel*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    RgbaPixel rgba;
    while (count < max)
    {
        rgba = oldBuffer[count];
        newBuffer[count] = ((rgba >> 8) & 0x00ffffff); /// rgb
        count++;
    }
}

void Image::ConvertBufferRgb32ToRgba8888(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer
    static constexpr Uint32_t oldPixelSizeBytes = 4;
    static constexpr Uint32_t newPixelSizeBytes = 4;
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const Uint32_t* oldBuffer = reinterpret_cast<const Uint32_t*>(buffer);
    RgbaPixel* newBuffer = reinterpret_cast<RgbaPixel*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    Uint32_t xrgb;
    while (count < max)
    {
        xrgb = oldBuffer[count];
        newBuffer[count] = ((xrgb << 8) | 0xff); /// rgba
        count++;
    }
}

void Image::ConvertBufferRgba8888ToRgb32(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer
    static constexpr Uint32_t oldPixelSizeBytes = 4;
    static constexpr Uint32_t newPixelSizeBytes = 4;
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const RgbaPixel* oldBuffer = reinterpret_cast<const RgbaPixel*>(buffer);
    Uint32_t* newBuffer = reinterpret_cast<Uint32_t*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    RgbaPixel rgba;
    while (count < max)
    {
        rgba = oldBuffer[count];
        newBuffer[count] = ((rgba >> 8) | 0xff000000); /// rgba
        count++;
    }
}

void Image::ConvertBufferRgba8888ToGrayscale8(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer
    static constexpr Uint32_t oldPixelSizeBytes = sizeof(RgbaPixel);
    static constexpr Uint32_t newPixelSizeBytes = 1;
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const RgbaPixel* oldBuffer = reinterpret_cast<const RgbaPixel*>(buffer);
    GrayscalePixel* newBuffer = reinterpret_cast<GrayscalePixel*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    RgbaPixel rgba;
    while (count < max)
    {
        rgba = oldBuffer[count];
        newBuffer[count] = GrayscalePixel(rgba); /// grayscale
        count++;
    }
}

void Image::ConvertBufferGrayscale8ToRgba8888(Image& image, const Uint8_t* buffer)
{
    if (image.isNull()) return;
    assert(nullptr == image.m_imageBuffer && "Error, image already has a buffer");

    /// Initialize a new buffer
    static constexpr Uint32_t oldPixelSizeBytes = 1;
    static constexpr Uint32_t newPixelSizeBytes = sizeof(RgbaPixel);
    image.m_imageBuffer = new Uint8_t[newPixelSizeBytes * image.width() * image.height()];
    RegisterBuffer(image.m_imageBuffer, ImageBufferFlag::kOwnBuffer);

    /// Cast buffers so that strides match
    const GrayscalePixel* oldBuffer = reinterpret_cast<const GrayscalePixel*>(buffer);
    RgbaPixel* newBuffer = reinterpret_cast<RgbaPixel*>(image.m_imageBuffer);

    /// Iterate over old buffer to populate new one
    Uint32_t count = 0;
    Uint32_t max = image.height() * image.width();
    GrayscalePixel g;
    while (count < max)
    {
        g = oldBuffer[count];
        newBuffer[count] = RgbaPixel(g); /// grayscale
        count++;
    }
}


void Image::ConvertBufferArgbToRgba8888(Image& image)
{
    if (image.isNull()) return;

    Uint32_t count = 0;
    Uint32_t max = (Uint32_t)(image.height() * image.width());
    Uint32_t* p = reinterpret_cast<Uint32_t*>(image.buffer());
    Uint32_t n;
    while (count < max)
    {
        n = p[count];   //n = ARGB
        p[count] = 0x00000000 |
            ((n << 8) & 0x0000ff00) |
            ((n << 8) & 0x00ff0000) |
            ((n << 8) & 0xff000000) |
            ((n >> 24) & 0x000000ff); // p[count] is RGBA
        count++;
    }
}

void Image::ConvertBufferRgbaToArgb8888(Image& image)
{
    if (image.isNull()) return;
    Uint32_t count = 0;
    Uint32_t max = (Uint32_t)(image.height() * image.width());
    Uint32_t* p = reinterpret_cast<Uint32_t*>(image.buffer());
    Uint32_t n;
    while (count < max)
    {
        n = p[count];   //n = RGBA
        p[count] = 0x00000000 |
            ((n >> 8) & 0x000000ff) |
            ((n >> 8) & 0x0000ff00) |
            ((n >> 8) & 0x00ff0000) |
            ((n << 24) & 0xff000000); // p[count] is ARGB
        count++;
    }
}


void Image::copyBuffer(Image& other) const
{
    // Copy buffer
    other.clear();
    other.m_imageBuffer = new Uint8_t[(Uint32_t)bytesPerLine() * height()];
    memcpy(other.m_imageBuffer, m_imageBuffer, sizeInBytes());

    // Add new buffer (the copy) to static map
    RegisterBuffer(other.m_imageBuffer, ImageBufferFlag::kOwnBuffer);
}

void Image::copyMetadata(Image& other) const
{
    other.m_colorFormat = m_colorFormat;
    other.m_pixelSizeBytes = m_pixelSizeBytes;
    other.m_imageSize = m_imageSize;
}

Image::ImageBufferInfo& Image::RegisterBuffer(Uint8_t* buffer, ImageBufferFlags flags)
{
    std::unique_lock lock(s_bufferMutex);

    ImageBufferInfo* info;
    if (s_bufferCounts.contains(buffer)) {
        info = &s_bufferCounts[buffer];
    }
    else {
        s_bufferCounts.emplace(buffer, ImageBufferInfo());
        info = &s_bufferCounts[buffer];
    }

    // Update buffer info flags
    if (flags != 0) {
        // Assign new flags
        info->m_flags = flags;
    }

    // Increment buffer count
    info->m_referenceCount++;

    return *info;
}

void Image::setBuffer(Uint8_t* buffer, ImageBufferFlags flags)
{
    m_imageBuffer = buffer;
    if (m_imageBuffer) {
        RegisterBuffer(m_imageBuffer, flags);
    }
}

std::vector<Uint8_t> Image::s_pixelByteCounts = {
    0,  // QImage::Format_Invalid   0
    1,  // QImage::Format_Mono	    1	The image is stored using 1 - bit per pixel.Bytes are packed with the most significant bit(MSB) first.
    1,  // QImage::Format_MonoLSB	2	The image is stored using 1 - bit per pixel.Bytes are packed with the less significant bit(LSB) first.
    8,  // QImage::Format_Indexed8	3	The image is stored using 8 - bit indexes into a colormap.
    32, // QImage::Format_RGB32   	4	The image is stored using a 32 - bit RGB format(0xffRRGGBB).
    32, // QImage::Format_ARGB32	5	The image is stored using a 32 - bit ARGB format(0xAARRGGBB).
    32, // QImage::Format_ARGB32_Premultiplied	6	The image is stored using a premultiplied 32 - bit ARGB format(0xAARRGGBB), i.e.the red, green,and blue channels are multiplied by the alpha component divided by 255. (If RR, GG, or BB has a higher value than the alpha channel, the results are undefined.) Certain operations(such as image composition using alpha blending) are faster using premultiplied ARGB32 than with plain ARGB32.
    16, // QImage::Format_RGB16	    7	The image is stored using a 16 - bit RGB format(5 - 6 - 5).
    24, // QImage::Format_ARGB8565_Premultiplied	8	The image is stored using a premultiplied 24 - bit ARGB format(8 - 5 - 6 - 5).
    24, // QImage::Format_RGB666	9	The image is stored using a 24 - bit RGB format(6 - 6 - 6).The unused most significant bits is always zero.
    24, // QImage::Format_ARGB6666_Premultiplied	10	The image is stored using a premultiplied 24 - bit ARGB format(6 - 6 - 6 - 6).
    16, // QImage::Format_RGB555	11	The image is stored using a 16 - bit RGB format(5 - 5 - 5).The unused most significant bit is always zero.
    24, // QImage::Format_ARGB8555_Premultiplied	12	The image is stored using a premultiplied 24 - bit ARGB format(8 - 5 - 5 - 5).
    24, // QImage::Format_RGB888	13	The image is stored using a 24 - bit RGB format(8 - 8 - 8).
    16, // QImage::Format_RGB444	14	The image is stored using a 16 - bit RGB format(4 - 4 - 4).The unused bits are always zero.
    16, // QImage::Format_ARGB4444_Premultiplied	15	The image is stored using a premultiplied 16 - bit ARGB format(4 - 4 - 4 - 4).
    32, // QImage::Format_RGBX8888	16	The image is stored using a 32 - bit byte - ordered RGB(x) format(8 - 8 - 8 - 8).This is the same as the Format_RGBA8888 except alpha must always be 255. (added in Qt 5.2)
    32, // QImage::Format_RGBA8888	17	The image is stored using a 32 - bit byte - ordered RGBA format(8 - 8 - 8 - 8).Unlike ARGB32 this is a byte - ordered format, which means the 32bit encoding differs between big endian and little endian architectures, being respectively(0xRRGGBBAA) and (0xAABBGGRR).The order of the colors is the same on any architecture if read as bytes 0xRR,0xGG,0xBB,0xAA. (added in Qt 5.2)
    32, // QImage::Format_RGBA8888_Premultiplied	18	The image is stored using a premultiplied 32 - bit byte - ordered RGBA format(8 - 8 - 8 - 8). (added in Qt 5.2)
    32, // QImage::Format_BGR30	19	The image is stored using a 32 - bit BGR format(x - 10 - 10 - 10). (added in Qt 5.4)
    32, // QImage::Format_A2BGR30_Premultiplied	20	The image is stored using a 32 - bit premultiplied ABGR format(2 - 10 - 10 - 10). (added in Qt 5.4)
    32, // QImage::Format_RGB30	21	The image is stored using a 32 - bit RGB format(x - 10 - 10 - 10). (added in Qt 5.4)
    32, // QImage::Format_A2RGB30_Premultiplied	22	The image is stored using a 32 - bit premultiplied ARGB format(2 - 10 - 10 - 10). (added in Qt 5.4)
    8,  // QImage::Format_Alpha8	23	The image is stored using an 8 - bit alpha only format. (added in Qt 5.5)
    8,  // QImage::Format_Grayscale8	24	The image is stored using an 8 - bit grayscale format. (added in Qt 5.5)
    64, // QImage::Format_RGBX64	25	The image is stored using a 64 - bit halfword - ordered RGB(x) format(16 - 16 - 16 - 16).This is the same as the Format_RGBA64 except alpha must always be 65535. (added in Qt 5.12)
    64, // QImage::Format_RGBA64	26	The image is stored using a 64 - bit halfword - ordered RGBA format(16 - 16 - 16 - 16). (added in Qt 5.12)
    64, // QImage::Format_RGBA64_Premultiplied	27	The image is stored using a premultiplied 64 - bit halfword - ordered RGBA format(16 - 16 - 16 - 16). (added in Qt 5.12)
    16, // QImage::Format_Grayscale16	28	The image is stored using an 16 - bit grayscale format. (added in Qt 5.13)
    24  // QImage::Format_BGR888	29	The image is stored using a 24 - bit BGR format. (added in Qt 5.14)
};


void to_json(nlohmann::json& orJson, const Image& korObject)
{
}

void from_json(const nlohmann::json& korJson, Image& orObject)
{
}

tsl::robin_map<Uint8_t*, Image::ImageBufferInfo> Image::s_bufferCounts{};

std::shared_mutex Image::s_bufferMutex{};


} // End namespaces
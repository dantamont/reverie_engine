#pragma once

#include <shared_mutex>

// Internal
#include "fortress/containers/extern/tsl/robin_map.h"
#include "fortress/containers/math/GSize.h"
#include "fortress/json/GJson.h"
#include "fortress/layer/framework/GFlags.h"

namespace rev {

class Color;
class GString;

/// @class Image
/// @brief Class representing a resource
class Image {
public:

    /// @brief Struct representing an RGB pixel
    /// @todo See if endianness of system breaks this
    struct Rgb888Pixel {

        Rgb888Pixel() = default;

        Rgb888Pixel(Uint32_t id) :
            m_r((id >> 16) & 0xff),
            m_g((id >> 8) & 0xff),
            m_b((id >> 0) & 0xff)
        {
        }

        explicit operator Uint32_t() {
            return (Uint32_t(m_r) << 16) | (Uint32_t(m_g) << 8) | Uint32_t(m_b);
        }

        Uint32_t operator<<(Uint32_t shift) {
            return Uint32_t(*this) << shift;
        }

        Uint8_t m_r{ 0 }; ///< Red component
        Uint8_t m_g{ 0 }; ///< Green component
        Uint8_t m_b{ 0 }; ///< Blue component
    };
    static_assert(sizeof(Rgb888Pixel) == 3, "Size of RgbPixel must be three bytes");

    /// @brief Struct representing an RGBA pixel
    /// @todo See if endianness of system breaks this
    struct RgbaPixel {

        RgbaPixel() = default;
        RgbaPixel(Uint32_t rgba):
            m_rgba(rgba)
        {
        }

        Uint8_t r() const {
            return static_cast<Uint8_t>(m_rgba >> 24);
        }

        Uint8_t g() const {
            return static_cast<Uint8_t>(m_rgba >> 16);
        }

        Uint8_t b() const {
            return static_cast<Uint8_t>(m_rgba >> 8);
        }

        Uint8_t a() const {
            return static_cast<Uint8_t>(m_rgba);
        }

        explicit operator Uint32_t() {
            return m_rgba;
        }

        Uint32_t operator<<(Uint32_t shift) {
            return m_rgba << shift;
        }

        Uint32_t operator>>(Uint32_t shift) {
            return m_rgba >> shift;
        }

        Uint32_t m_rgba{ 0 }; ///< RGBA components, stored as 0xRRGGBBAA
    };
    static_assert(sizeof(RgbaPixel) == 4, "Size of RgbaPixel must be three bytes");

    /// @brief Grayscale Pixel
    struct GrayscalePixel {

        /// @brief Obtain Grayscale from RGB
        /// @see https://www.kdnuggets.com/2019/12/convert-rgb-image-grayscale.html
        /// @see https://www.codeproject.com/Questions/327689/how-to-convert-coloured-image-to-grayscale-in-C-or
        static inline Uint8_t FromRgb(Uint32_t r, Uint32_t g, Uint32_t b) {
            return ((r * 307) + (g * 604) + (b * 113)) >> 10;
        }

        GrayscalePixel() = default;
        GrayscalePixel(const RgbaPixel& rgba) :
            m_grayscale(FromRgb(rgba.r(), rgba.g(), rgba.b()))
        {
        }

        operator RgbaPixel() const {
            return RgbaPixel((m_grayscale << 24) | (m_grayscale << 16) | (m_grayscale << 8) | 0xff);
        }

        Uint8_t m_grayscale{ 0 }; ///< Grayscale component
    };
    static_assert(sizeof(GrayscalePixel) == 1, "Size of GrayscalePixel must be three bytes");

    /// @brief All possible color formats for an image
    enum class ColorFormat {
        kInvalid,
        kMono,
        kMonoLSB,
        kIndexed8,
        kRGB32,
        kARGB32,
        kARGB32_Premultiplied,
        kRGB16,
        kARGB8565_Premultiplied,
        kRGB666,
        kARGB6666_Premultiplied,
        kRGB555,
        kARGB8555_Premultiplied,
        kRGB888,
        kRGB444,
        kARGB4444_Premultiplied,
        kRGBX8888,
        kRGBA8888,
        kRGBA8888_Premultiplied,
        kBGR30,
        kA2BGR30_Premultiplied,
        kRGB30,
        kA2RGB30_Premultiplied,
        kAlpha8,
        kGrayscale8,
        kRGBX64,
        kRGBA64,
        kRGBA64_Premultiplied,
        kGrayscale16,
        kBGR888,
        kGrayscaleAlpha88, ///< Added for STBI compatibility
        kEND
    };

    static constexpr ColorFormat StandardColorFormat = ColorFormat::kRGBA8888;

    /// @name Static
    /// @{

    /// @brief Get the pixel byte count of the specified format
    static Uint32_t GetPixelBitCount(Image::ColorFormat fmt);

    /// @}

	/// @name Constructors/Destructor
	/// @{
    Image();

    /// @brief Image will not take ownership of const data
    Image(const Uint8_t* data, Int32_t width, Int32_t height, Int32_t bytesPerLine, ColorFormat format);

    /// @brief Image will not take ownership of const data
    Image(const Uint8_t *data, Int32_t width, Int32_t height, ColorFormat format);

    Image(const GString& filepath, bool flipVertically, ColorFormat format = ColorFormat::kInvalid);

    /// @brief Performs a shallow copy of the image
    Image(const Image& image);

    /// @brief Create image from another image
    /// @detail If deepCopy is false, this behaves exactly like the copy constructor, sharing a buffer
    Image(const Image& other, bool deepCopy);
    ~Image();
	/// @}

    /// @name Properties
    /// @{

    /// @brief Return the internal color format of the image
    Image::ColorFormat format() const { return m_colorFormat; }

    /// @}

    /// @name Public Methods
	/// @{

    /// @brief Create a (deep) copy of the image in the given format
    Image convertToFormat(Image::ColorFormat format);

    /// @brief Clear the image 
    void clear();

    /// @brief Whether or not the image is valid
    bool isNull() const { return (nullptr == m_imageBuffer) && m_imageSize.isNull(); }

    /// @brief Return bits of the image
    Uint8_t* buffer() { return m_imageBuffer; }
    const Uint8_t* buffer() const { return m_imageBuffer; }

    /// @brief Return size of the image
    GSize size() const { return m_imageSize; }

    /// @brief Get the width of the image
    Uint32_t width() const { return m_imageSize.width(); }

    /// @brief Get the height of the image
    Uint32_t height() const { return m_imageSize.height(); }

    /// @brief Size of the image in megabytes
    size_t sizeInMegaBytes() const;
    size_t sizeInBytes() const;
    Int32_t bytesPerLine() const;

    /// @brief Get the size (in bytes) of each pixel in the image
    Int32_t pixelSizeBytes() const;

    /// @brief Save the image to a file
    /// @param[in] quality The image quality (only used for JPG), set from 0-100. Defaults to 100 if unset
    /// @note See: https://solarianprogrammer.com/2019/06/10/c-programming-reading-writing-images-stb_image-libraries/
    bool save(const GString& filename, const char* format = 0, Int32_t quality = -1);

	/// @}


    /// @name Friend Functions
    /// @{

    /// @todo Implement
    /// @see https://github.com/nlohmann/json/issues/1553#issuecomment-487569496
    /// @see https://github.com/nlohmann/json/issues/1553
    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const Image& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, Image& orObject);


    /// @}

protected:

    /// @brief Image memory-management flags
    enum class ImageBufferFlag {
        kOwnBuffer = 1 << 0, ///< Whether or not to take ownership of the buffer. If true, will free buffer when reference count reaches zero
        kFreeNotDelete = 1 << 1 ///< If true, calls free rather than delete on buffer destruction
    };
    MAKE_FLAGS(ImageBufferFlag, ImageBufferFlags)

        /// @brief Metadata describing an image buffer
        struct ImageBufferInfo {
        Int32_t m_referenceCount{ 0 }; ///< The number of references to the buffer
        ImageBufferFlags m_flags{ 0 }; ///< The flags describing how the buffer memory is managed
    };


    /// @name Friends
    /// @{

    friend class Texture;
    friend class CubeTexture;
    friend class ResourceCache;

    /// @}

    /// @name Operators
    /// @{

    /// @brief Made private to avoid accidental expensive copy operations
    Image& operator=(const Image& image);

    /// @}

    /// @name Private Methods
    /// @{


    /// @brief Load .tga image
    /// See: https://forum.qt.io/topic/101971/qimage-and-tga-support-in-c
    /// https://forum.qt.io/topic/74712/qimage-from-tga-with-alpha/11
    void loadImage(const GString& filePath, bool &success, ColorFormat format, bool flipVertically);

    /// @brief Convert image to RGBA8888 type
    /// @return A copy of the image, in RGBA8888 format
    Image convertToRgba8888() const;

    /// @brief Convert from Rgba8888 to the given image type
    /// @details Converts the given image to the specified type
    Image convertFromRgba8888(ColorFormat format);

    /// @brief Convert buffer to given format
    /// @param[in] image the image for which to convert the buffer
    static void ConvertBufferArgbToRgba8888(Image& image);
    static void ConvertBufferRgbaToArgb8888(Image& image);

    /// @brief Convert buffer to given format
    /// @param[in] image the image for which to convert the buffer
    /// @param[in] buffer The buffer to use, which must not be from the image itself
    static void ConvertBufferRgb888ToRgba8888(Image& image, const Uint8_t* buffer);
    static void ConvertBufferRgba8888ToRgb888(Image& image, const Uint8_t* buffer);
    static void ConvertBufferRgb32ToRgba8888(Image& image, const Uint8_t* buffer);
    static void ConvertBufferRgba8888ToRgb32(Image& image, const Uint8_t* buffer);
    static void ConvertBufferRgba8888ToGrayscale8(Image& image, const Uint8_t* buffer);
    static void ConvertBufferGrayscale8ToRgba8888(Image& image, const Uint8_t* buffer);

    /// @brief Copy the buffer in this image to the other image
    void copyBuffer(Image& other) const;

    /// @brief Copy the metadata in this image to the other image
    /// @details metadata is everything but the image buffer itself
    void copyMetadata(Image& other) const;

    /// @brief Register a buffer as used by an additional image
    static ImageBufferInfo& RegisterBuffer(Uint8_t* buffer, ImageBufferFlags flags = 0);

    /// @brief Set the image buffer
    void setBuffer(Uint8_t* buffer, ImageBufferFlags flags = 0);

    /// @}

    /// @name Private Members
    /// @{

    Image::ColorFormat m_colorFormat{ Image::ColorFormat::kInvalid }; ///< The format of the image
    Uint8_t m_pixelSizeBytes{ 0 }; ///< The size of a pixel of the image (in bytes)
    GSize m_imageSize{0, 0}; ///< The size of the image
    Uint8_t* m_imageBuffer{ nullptr }; ///< The image buffer manually memory-managed since loaded through stbi

    /// @brief Vector of Image formats and the pixel byte count
    /// @details Index is Image::ColorFormat
    /// @note For compatibility, image formats match QImage::Format (which is a pain)
    static std::vector<Uint8_t> s_pixelByteCounts;

    /// @details Used to determine when buffer should be deallocated
    static tsl::robin_map<Uint8_t*, ImageBufferInfo> s_bufferCounts; ///< All of the unique buffers owned by images, and their counts

    /// @brief Mutex for buffer counts map
    static std::shared_mutex s_bufferMutex;

    /// @}

};


} /// End rev namespaces

#include <iostream>
#include "fortress/image/GTexturePacker.h"
#include "fortress/thread/GParallelLoop.h"

namespace rev {

void to_json(json& orJson, const SpriteSheetInfo& korObject)
{
    if (korObject.m_packedTextures.size()) {
        json spriteObject = json::array();
        for (const PackedTextureInfo& info : korObject.m_packedTextures) {
            json texObject;
            texObject["origin"] = info.m_origin;
            texObject["size"] = info.m_size;
            spriteObject.push_back(texObject);
        }
        orJson = spriteObject;
    }
}

void from_json(const json& korJson, SpriteSheetInfo& orObject)
{
    for (const auto& spriteVal : korJson) {
        const json& spriteObj = spriteVal;
        orObject.m_packedTextures.emplace_back(PackedTextureInfo{
            Vector2i(spriteObj["origin"]), Vector2i(spriteObj["size"]) });
    }
}

TexturePacker::TexturePacker(Image::ColorFormat format):
    TexturePacker(Vector<int, 2>(512, 512), 1, format)
{
}

TexturePacker::TexturePacker(const Vector<int, 2>& initialSize, Uint32_t pixelByteCount, Image::ColorFormat format):
    m_paddingPixels(1),
    m_bytesPerLine(0),
    m_numLines(0),
    m_format(format)
{
    resizeBuffer(initialSize.x() * pixelByteCount, initialSize.y());

    m_rootNode = std::make_unique<TextureNode>();
    m_rootNode->m_origin = Vector<int, 2>(0, 0);
    m_rootNode->m_size = Vector<int, 2>(INT_MAX, INT_MAX);
    m_rootNode->m_empty = true;
    m_rootNode->m_left = nullptr;
    m_rootNode->m_right = nullptr;
}

TexturePacker::~TexturePacker()
{
}

PackedTextureInfo TexturePacker::packTexture(const Image & image)
{
    if (m_format == Image::ColorFormat::kInvalid) {
        m_format = image.format();
    }

    assert(m_format != Image::ColorFormat::kInvalid && "Error, invalid image format");
    assert(m_format == image.format() && "Error, inconsistent format for packed textures");

    // Need to get size of buffer line, not width
    return packTexture(image.buffer(), Vector<int, 2>(image.bytesPerLine(), image.size().height()));
}

PackedTextureInfo TexturePacker::packTexture(const unsigned char * textureBuffer, int bufferWidth, int bufferHeight)
{
    return packTexture(textureBuffer, Vector<int, 2>(bufferWidth, bufferHeight));
}

PackedTextureInfo TexturePacker::packTexture(const unsigned char * textureBuffer,
    const Vector<int, 2>& bufferSize)
{
    // Pack the texture
    Uint32_t bytePadding = getBytePadding();
    Vector<int, 2> paddedSize = bufferSize + bytePadding;
    TextureNode* node = pack(m_rootNode.get(), paddedSize);
    if (node == NULL) {
        // Node is full, need to resize buffer
        this->resizeBuffer(m_bytesPerLine + paddedSize.x(), m_numLines + paddedSize.y());
        node = pack(m_rootNode.get(), paddedSize);
        assert(node != NULL);
    }

    // Confirm that the buffer size and largest node size are consistent
    int bufferWidth = bufferSize.x();
    int bufferHeight = bufferSize.y();
    assert(bufferWidth == node->m_size.x() - bytePadding);
    assert(bufferHeight == node->m_size.y() - bytePadding);

    // Actually populate the buffer
    size_t bufferLen = m_buffer.size();
    int lineLength = m_bytesPerLine;
    for (int ly = 0; ly < bufferHeight; ly++) {
        for (int lx = 0; lx < bufferWidth; lx++) {
            int y = node->m_origin.y() + ly + bytePadding;
            int x = node->m_origin.x() + lx + bytePadding;
            Uint32_t bufferIndex = y * lineLength + x;
#ifdef DEBUG_MODE
            assert(bufferIndex < bufferLen && "Error, overflow when packing buffer");
#endif
            m_buffer[bufferIndex] = textureBuffer[ly * bufferWidth + lx];
        }
    }

    // Return origin and size of the packed texture
    return { node->m_origin + bytePadding, bufferSize };
}

const unsigned char * TexturePacker::getBuffer() const
{
    return m_buffer.data();
}

Image TexturePacker::getImage() const
{
    assert(m_format != Image::ColorFormat::kInvalid, "Error, invalid image format");

    Uint32_t pixelByteCount = Image::GetPixelBitCount(m_format) / 8;
    assert (0 == m_bytesPerLine % pixelByteCount && "Error, unaligned pixel width");

    Uint32_t imageWidth = m_bytesPerLine / pixelByteCount;

    Image image = Image(m_buffer.data(), imageWidth, m_numLines, m_bytesPerLine, m_format);
    if (image.isNull()) {
       std::cout << ("getImage:: Null image obtained from texture packer");
#ifdef DEBUG_MODE
       throw std::runtime_error("Error, null image returned from texture packer");
#endif
    }
    return image;
}


Image TexturePacker::getImage(Image::ColorFormat format) const
{
    return getImage().convertToFormat(format);
}

Vector2i TexturePacker::getTextureSize() const
{
    assert(m_format != Image::ColorFormat::kInvalid, "Error, invalid image format");

    Uint32_t pixelByteCount = Image::GetPixelBitCount(m_format) / 8;
    return Vector2i(m_bytesPerLine / pixelByteCount, m_numLines);
}

TextureNode * TexturePacker::pack(TextureNode * node, const Vector<int, 2>& size)
{
    if (!node->m_empty) {
        // The node is filled, not gonna fit anything else here
        // Assert that the node is a leaf
        assert(!node->m_left && !node->m_right);
        return nullptr;
    }
    else if (node->m_left && node->m_right) {
        // Non-leaf, try inserting to the m_left and then to the m_right
        if (TextureNode* retval = pack(node->m_left.get(), size)) {
            return retval;
        }
        else {
            return pack(node->m_right.get(), size);
        }
    }
    else {
        // This is an unfilled leaf - let's see if we can fill it
        Vector<int, 2> realSize(node->m_size.x(), node->m_size.y());

        // If we're along a boundary, calculate the actual size
        if (node->m_origin.x() + node->m_size.x() == INT_MAX) {
            realSize[0] = m_bytesPerLine - node->m_origin.x();
        }
        if (node->m_origin.y() + node->m_size.y() == INT_MAX) {
            realSize[1] = m_numLines - node->m_origin.y();
        }

        if (node->m_size.x() == size.x() && node->m_size.y() == size.y()) {
            // Perfect size - just pack into this node, flag as full
            node->m_empty = false;
            return node;
        }
        else if (realSize.x() < size.x() || realSize.y() < size.y()) {
            // Not big enough
            return nullptr;
        }
        else {
            // Large enough - split until we get a perfect fit
            TextureNode* m_left;
            TextureNode* m_right;

            // Determine how much space we'll have m_left if we split each way
            int remainX = realSize.x() - size.x();
            int remainY = realSize.y() - size.y();

            // Split the way that will leave the most room
            bool verticalSplit = remainX < remainY;
            if (remainX == 0 && remainY == 0) {
                // Edge case - we are are going to hit the border of
                // the texture atlas perfectly, split at the border instead
                if (node->m_size.x() > node->m_size.y()) {
                    verticalSplit = false;
                }
                else {
                    verticalSplit = true;
                }
            }

            if (verticalSplit) {
                // Split vertically (m_left is top)
                m_left = new TextureNode(node->m_origin, Vector<int, 2>(node->m_size.x(), size.y()));
                m_right = new TextureNode(Vector<int, 2>(node->m_origin.x(), node->m_origin.y() + size.y()),
                    Vector<int, 2>(node->m_size.x(), node->m_size.y() - size.y()));
            }
            else {
                // Split horizontally
                m_left = new TextureNode(node->m_origin, Vector<int, 2>(size.x(), node->m_size.y()));
                m_right = new TextureNode(Vector<int, 2>(node->m_origin.x() + size.x(), node->m_origin.y()), Vector<int, 2>(node->m_size.x() - size.x(), node->m_size.y()));
            }

            node->m_left = std::unique_ptr<TextureNode>(m_left);
            node->m_right = std::unique_ptr<TextureNode>(m_right);
            return pack(node->m_left.get(), size);
        }
    }
}

void TexturePacker::resizeBuffer(Uint32_t bytesPerLine, Uint32_t numLines)
{
    Uint32_t newLineLength = (Uint32_t)bytesPerLine;
    Uint32_t newHeight = (Uint32_t)numLines;
    size_t oldLineLength = m_bytesPerLine;
    size_t oldHeight = m_numLines;

    // Create a new buffer
    std::vector<unsigned char> newBuffer;
    newBuffer.resize(newLineLength * newHeight);

    // Populate with data from current buffer
    Uint32_t bufferSize = (Uint32_t)newBuffer.size();
    for (Uint32_t y = 0; y < oldHeight; y++) {
        for (Uint32_t x = 0; x < oldLineLength; x++) {
            Uint32_t bufferIndex = y * newLineLength + x;
#ifdef DEBUG_MODE
            assert(bufferIndex < bufferSize && "Error, overflow when packing buffer");
#endif
            newBuffer[bufferIndex] = m_buffer[y * oldLineLength + x];
        }
    }

    // Update texture size to new size
    m_bytesPerLine = bytesPerLine;
    m_numLines = numLines;

    // Swap buffer out with new one
    m_buffer = std::move(newBuffer);
}

size_t TexturePacker::getBufferSize() const
{
    // Does not include overhead for size of empty vector, sizeof(std::vector<Type>)
    return sizeof(unsigned char) * m_buffer.size();
}

Uint32_t TexturePacker::getBytePadding() const
{
    return m_paddingPixels * Image::GetPixelBitCount(m_format) / 8;
}



} // End namespaces

#include "GbTexturePacker.h"
#include "GbParallelization.h"
#include "../GbCoreEngine.h"

#define USE_THREADING false

namespace Gb {


/////////////////////////////////////////////////////////////////////////////////////////////
TexturePacker::TexturePacker():
    TexturePacker(Vector<int, 2>(512, 512))
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
TexturePacker::TexturePacker(const Vector<int, 2>& initialSize):
    m_padding(1)
{
    resizeBuffer(initialSize);

    m_rootNode = std::make_unique<TextureNode>();
    m_rootNode->m_origin = Vector<int, 2>(0, 0);
    m_rootNode->m_size = Vector<int, 2>(INT_MAX, INT_MAX);
    m_rootNode->m_empty = true;
    m_rootNode->m_left = nullptr;
    m_rootNode->m_right = nullptr;
}
/////////////////////////////////////////////////////////////////////////////////////////////
TexturePacker::~TexturePacker()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector<int, 2> TexturePacker::packTexture(const Image & image)
{
    if (m_format == QImage::Format_Invalid) {
        m_format = image.format();
    }
    if (m_format == QImage::Format_Invalid) {
        throw("Error, invalid image format");
    }
    if (m_format != image.format()) {
        throw("Error, inconsistent format for packed textures");
    }

    QSize s = image.size();
    return packTexture(image.buffer(), Vector<int, 2>(s.width(), s.height()));
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector<int, 2> TexturePacker::packTexture(const unsigned char * textureBuffer, int bufferWidth, int bufferHeight)
{
    return packTexture(textureBuffer, Vector<int, 2>(bufferWidth, bufferHeight));
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector<int, 2> TexturePacker::packTexture(const unsigned char * textureBuffer,
    const Vector<int, 2>& bufferSize)
{
    // Pack the texture
    Vector<int, 2> paddedSize = bufferSize + m_padding;
    TextureNode* node = pack(m_rootNode.get(), paddedSize);
    if (node == NULL) {
        // Node is full, need to resize buffer
        this->resizeBuffer(m_textureSize * 2);
        node = pack(m_rootNode.get(), paddedSize);
        assert(node != NULL);
    }

    // Confirm that the buffer size and largest node size are consistent
    int bufferWidth = bufferSize.x();
    int bufferHeight = bufferSize.y();
    assert(bufferWidth == node->m_size.x() - m_padding);
    assert(bufferHeight == node->m_size.y() - m_padding);

    // Actually populate the buffer
    ParallelLoopGenerator loop(nullptr, USE_THREADING);

    // FIXME: Figure out why there is an access violation once in a blue moon from the multi-threading
    // Update 4/10/2020: May have fixed.  std::vector is not thread-safe for simultaneous writes
    int textureWidth = m_textureSize.x();
    //QMutex lock;
    loop.parallelFor(bufferHeight, [&](int start, int end) {
        for (int ly = start; ly < end; ly++) {
            for (int lx = 0; lx < bufferWidth; lx++) {
                int y = node->m_origin.y() + ly + m_padding;
                int x = node->m_origin.x() + lx + m_padding;
                //QMutexLocker locker(&lock);
                m_buffer[y * textureWidth + x] = textureBuffer[ly * bufferWidth + lx];
            }
        }
    });

    return node->m_origin + m_padding;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Vector<int, 2> TexturePacker::getTextureSize() const
{
    return m_textureSize;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const unsigned char * TexturePacker::getBuffer() const
{
    return m_buffer.data();
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image TexturePacker::getImage(QImage::Format format) const
{
    Image image = Image(m_buffer.data(), m_textureSize[0], m_textureSize[1], format);
    if (image.isNull()) {
        logWarning("getImage:: Null image obtained from texture packer");
#ifdef DEBUG_MODE
        throw("Error, null image returned from texture packer");
#endif
    }
    return image;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Image TexturePacker::getImage() const
{
    if (m_format == QImage::Format_Invalid) {
        throw("Error, no valid image format specified");
    }
    return getImage(m_format);
}
/////////////////////////////////////////////////////////////////////////////////////////////
TexturePacker::TextureNode * TexturePacker::pack(TexturePacker::TextureNode * node,
    const Vector<int, 2>& size)
{
    if (!node->m_empty) {
        // The node is filled, not gonna fit anything else here
        assert(!node->m_left && !node->m_right);
        return NULL;
    }
    else if (node->m_left && node->m_right) {
        // Non-leaf, try inserting to the m_left and then to the m_right
        TextureNode* retval = pack(node->m_left.get(), size);
        if (retval != NULL) {
            return retval;
        }
        return pack(node->m_right.get(), size);
    }
    else {
        // This is an unfilled leaf - let's see if we can fill it
        Vector<int, 2> realSize(node->m_size.x(), node->m_size.y());

        // If we're along a boundary, calculate the actual size
        if (node->m_origin.x() + node->m_size.x() == INT_MAX) {
            realSize[0] = m_textureSize.x() - node->m_origin.x();
        }
        if (node->m_origin.y() + node->m_size.y() == INT_MAX) {
            realSize[1] = m_textureSize.y() - node->m_origin.y();
        }

        if (node->m_size.x() == size.x() && node->m_size.y() == size.y()) {
            // Perfect size - just pack into this node
            node->m_empty = false;
            return node;
        }
        else if (realSize.x() < size.x() || realSize.y() < size.y()) {
            // Not big enough
            return NULL;
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
/////////////////////////////////////////////////////////////////////////////////////////////
void TexturePacker::resizeBuffer(const Vector<int, 2>& newSize)
{
    int newWidth = newSize.x();
    int newHeight = newSize.y();
    int oldWidth = m_textureSize.x();
    int oldHeight = m_textureSize.y();

    // Create a new buffer
    std::vector<unsigned char> newBuffer;
    newBuffer.resize(newWidth * newHeight);

    // Populate with data from current buffer
    ParallelLoopGenerator loop(nullptr, USE_THREADING);
    loop.parallelFor((int)oldHeight, [&](int start, int end) {
        for (int y = start; y < end; y++) {
            for (int x = 0; x < oldWidth; x++) {
                newBuffer[y * newWidth + x] = m_buffer[y * oldWidth + x];
            }
        }
    });

    // Update texture size to new size
    m_textureSize = newSize;

    // Swap buffer out with new one
    m_buffer = std::move(newBuffer);
}
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long TexturePacker::getBufferSize() const
{
    // Does not include overhead for size of empty vector, sizeof(std::vector<Type>)
    return sizeof(unsigned char) * m_buffer.size();
}


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

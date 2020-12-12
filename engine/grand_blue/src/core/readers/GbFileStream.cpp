#include "GbFileStream.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include "../containers/GbContainerExtensions.h"

namespace Gb {

/////////////////////////////////////////////////////////////////////////////////////////////
FileStream::FileStream(const GString & filePath):
    m_filePath(filePath)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
FileStream::~FileStream()
{
    if (m_fileStream) {
        Object().logWarning("Warning, file stream was not closed properly before destruction");
        close();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
llong FileStream::lendian_write(const void * ptr, size_t size, const llong& count, FILE * stream)
{
    static int x = 1;
    static bool isLittleEndian = *((char*)&x) == 1;
    if (isLittleEndian)
    {
        // Little endian machine, use fwrite directly
        // Args: 
        // ptr - pointer to the array of elements to be written
        // size - size in bytes of each element to be written
        // count -  each one with a size of size bytes
        // stream - pointer to a FILE object that specifies an output stream
        return fwrite(ptr, size, count, stream);
    }
    else
    {
        // Big endian machine, pre-process first
        return endian_swap_fwrite(ptr, size, count, stream);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
llong FileStream::endian_swap_fwrite(const void * ptr, size_t size, const llong& nmemb, FILE * stream)
{
    // Duplicate source buffer into a destination buffer, with byte order swapped for endianness
    unsigned char *buffer_src = (unsigned char*)ptr;
    unsigned char *buffer_dst = new unsigned char[size*nmemb];

    // Iterate over all members
    for (llong i = 0; i < nmemb; i++)
    {
        // Reverse byte order for current member
        for (size_t ix = 0; ix < size; ix++) {
            buffer_dst[size * i + (size - 1 - ix)] = buffer_src[size * i + ix];
        }
    }

    // Perform fwrite
    // TODO: Perform write as many times as needed for nmemb > size_t::max_size
    llong result = fwrite(buffer_dst, size, nmemb, stream);

    // Delete intermediate buffer
    delete buffer_dst;
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::array<GString, (size_t)FileAccessMode::kMAX_ACCESS_MODE> FileStream::s_accessModes =
{ "wb", 
  "rb"//,
  //"wb+",
  //"rb+"
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

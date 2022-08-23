#include "fortress/streams/GFileStream.h"
#include "fortress/containers/GContainerExtensions.h"

namespace rev {


FileStream::FileStream(const GString & filePath):
    m_filePath(filePath)
{
}

FileStream::~FileStream()
{
    if (m_fileStream) {
        std::cout << "Warning, file stream was not closed properly before destruction";
        close();
    }
}


// End namespaces
}

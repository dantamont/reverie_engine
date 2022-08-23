#include <gtest/gtest.h>
#include "fortress/streams/GFileStream.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(FileStreamTest, TestReadWrite) {
    // Setup
    GString path(_FORTRESS_TEST_DIR + std::string("/data/tmp/file_read_write_test"));
    FileStream stream(path);
    stream.open(FileAccessMode::kWrite | FileAccessMode::kBinary);

    // Write to file
    const char* myValue = "This is my stuff to write to a file";
    stream.write(myValue, 16);
    stream.close();

    // Read from file
    char myReadValue[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    Uint64_t outCount;
    stream.open(FileAccessMode::kRead | FileAccessMode::kBinary);
    stream.read(myReadValue, outCount);
    EXPECT_EQ(outCount, 16);
    for (Uint64_t i = 0; i < 16; i++) {
        EXPECT_EQ(myReadValue[i], myValue[i]);
    }

    // Close file stream
    stream.close();
}

} /// End rev namespace
#include <gtest/gtest.h>
#include "fortress/system/path/GFile.h"
#include "fortress/system/path/GDir.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(FileTest, getExtension) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GFile myFile(dataPath);
    GString extension = myFile.extension();
    EXPECT_EQ(extension, "txt");
}

TEST(FileTest, replaceExtension) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GFile myFile(dataPath);
    GString newPath = myFile.replaceExtension("goof");
    GString expectedPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.goof"));
    EXPECT_EQ(newPath, expectedPath);
}

TEST(FileTest, getFileName) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/Lorem_ipsum_1000.txt"));
    GFile myFile(dataPath);

    GString name = myFile.getFileName(true, true);
    EXPECT_EQ(name, "Lorem_ipsum_1000.txt");

    name = myFile.getFileName(false, true);
    EXPECT_EQ(name, "Lorem_ipsum_1000");
    EXPECT_EQ(name.length(), GString("Lorem_ipsum_1000").length());

    name = myFile.getFileName(true, false);
    EXPECT_EQ(name, "lorem_ipsum_1000.txt");
}

TEST(FileTest, getFileSizeBytes) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GFile myFile(dataPath);

    // Git may pull in file with windows-style line-endings, adding
    // an extra two bytes, so check for either case
    constexpr Uint32_t expectedUnixSize = 1000;
    EXPECT_EQ(myFile.getFileSizeBytes() == expectedUnixSize ||
        myFile.getFileSizeBytes() == (expectedUnixSize + 2), true);
}

TEST(FileTest, isFile) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GString badPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.tx"));
    GFile myFile(dataPath);
    GFile badFile(badPath);
    GFile myDir(_FORTRESS_TEST_DIR);
    EXPECT_EQ(myFile.isFile(), true);
    EXPECT_EQ(badFile.isFile(), false);
    EXPECT_EQ(myDir.isFile(), false);
}

TEST(FileTest, exists) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GString badPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.tx"));
    GFile myFile(dataPath);
    GFile badFile(badPath);
    EXPECT_EQ(myFile.exists(), true);
    EXPECT_EQ(badFile.exists(), false);
}

TEST(FileTest, getDirectory) {
    GString myFilePath(_FORTRESS_TEST_DIR + GString("/data/tmp/create/test.txt"));
    GFile myFile(myFilePath);
    GString expectedDir(_FORTRESS_TEST_DIR + GString("/data/tmp/create"));
    EXPECT_EQ(expectedDir, myFile.getDirectory());
}

TEST(FileTest, getAbsoluteDirectory) {
    GDir::SetCurrentWorkingDir(_FORTRESS_TEST_DIR);
    GString myFilePath(GString("data/tmp/create/test.txt"));
    GFile myFile(myFilePath);
    GString expectedDir(_FORTRESS_TEST_DIR + GString("/data/tmp/create"));
    EXPECT_EQ(expectedDir, myFile.getAbsoluteDirectory());
}

TEST(FileTest, create) {
    GString myFilePath(_FORTRESS_TEST_DIR + GString("/data/tmp/create/test.txt"));
    GFile myFile(myFilePath);
    EXPECT_EQ(myFile.exists(), false);

    myFile.create();
    EXPECT_EQ(myFile.exists(), false);

    myFile.create(true);
    EXPECT_EQ(myFile.exists(), true);
    myFile.remove();

    GString dirPath = myFile.getDirectory();
    GDir myDir(dirPath);
    myDir.remove();
}

TEST(FileTest, remove) {
    GString myFilePath(_FORTRESS_TEST_DIR + GString("/data/tmp/remove/test.txt"));
    GFile myFile(myFilePath);
    EXPECT_EQ(myFile.exists(), false);

    myFile.create(true);
    EXPECT_EQ(myFile.exists(), true);
    myFile.remove();
    EXPECT_EQ(myFile.exists(), false);

    GString dirPath = myFile.getDirectory();
    GDir myDir(dirPath);
    myDir.remove();
}

TEST(FileTest, read) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lorem_ipsum_1000.txt"));
    GFile myFile(dataPath);
    EXPECT_EQ(myFile.isFile(), true);

    GString contents = myFile.read();

    // Git may pull in file with windows-style line-endings, adding
    // an extra two bytes, so check for either case
    constexpr Uint32_t expectedUnixSize = 1000;
    EXPECT_EQ(myFile.getFileSizeBytes() == expectedUnixSize ||
        myFile.getFileSizeBytes() == (expectedUnixSize + 2), true);
}

TEST(FileTest, readLines) {
    GString dataPath(_FORTRESS_TEST_DIR + GString("/data/lines_test.txt"));
    GFile myFile(dataPath);
    EXPECT_EQ(myFile.isFile(), true);

    std::vector<GString> contents = myFile.readLines();
    EXPECT_EQ(contents.size(), 5); // File is 5 lines
    EXPECT_EQ(contents[0], "line0");
    EXPECT_EQ(contents[1], "line1");
    EXPECT_EQ(contents[2], "line2");
    EXPECT_EQ(contents[3], "line3");
    EXPECT_EQ(contents[4], "another_line");
}

} /// End rev namespace
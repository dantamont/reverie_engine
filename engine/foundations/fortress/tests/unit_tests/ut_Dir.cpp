#include <gtest/gtest.h>
#include "fortress/system/path/GDir.h"
#include "fortress/system/path/GFile.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(DirTest, CurrentWorkingDir) {
    GString currentDirStr = GDir::CurrentWorkingDir();
    GDir currentDir(currentDirStr);
    EXPECT_EQ(currentDir.exists(), true);
    EXPECT_EQ(currentDir.isDirectory(), true);
}

TEST(DirTest, SetCurrentWorkingDir) {
    GString firstDirStr = GDir::CurrentWorkingDir();
    GString newDirStr = _FORTRESS_TEST_DIR;

    GDir::SetCurrentWorkingDir(newDirStr);
    GString currentDirStr = GDir::CurrentWorkingDir();
    GDir currentDir(currentDirStr);

    EXPECT_EQ(currentDir.exists(), true);
    EXPECT_EQ(currentDir.isDirectory(), true);
    EXPECT_EQ(currentDir.path(), newDirStr);
}

TEST(DirTest, isDirectory) {
    GString dirStr = _FORTRESS_TEST_DIR;
    GDir dir(dirStr);
    GString badDirStr = GString(_FORTRESS_TEST_DIR) + "/this_is_fake/no_good";
    GDir badDir(badDirStr);
    EXPECT_EQ(dir.isDirectory(), true);
    EXPECT_EQ(badDir.isDirectory(), false);
}

TEST(DirTest, exists) {
    GString dirStr = _FORTRESS_TEST_DIR;
    GDir dir(dirStr);
    GString badDirStr = GString(_FORTRESS_TEST_DIR) + "/this_is_fake/no_good";
    GDir badDir(badDirStr);
    EXPECT_EQ(dir.exists(), true);
    EXPECT_EQ(badDir.exists(), false);
}

TEST(DirTest, absolutePath) {
    GString dirStr = _FORTRESS_TEST_DIR;
    GDir dir(dirStr);
    GString absPath(dir.absolutePath("data"));
    GString expectedPath(_FORTRESS_TEST_DIR + GString("/data"));
    EXPECT_EQ(expectedPath, absPath);
}

TEST(DirTest, relativePath) {
    GString dirStr = _FORTRESS_TEST_DIR;
    GDir dir(dirStr);
    GString relPath(dir.relativePath(_FORTRESS_TEST_DIR + GString("/data")));
    GString expectedPath("data");
    EXPECT_EQ(expectedPath, relPath);
}

TEST(DirTest, create) {
    GString dirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create"));
    GDir dir(dirStr);

    EXPECT_EQ(dir.exists(), false);
    dir.create();
    EXPECT_EQ(dir.exists(), true);
    dir.remove();

    dir.removeAll();
}

TEST(DirTest, remove) {
    GString dirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create"));
    GDir dir(dirStr);

    EXPECT_EQ(dir.exists(), false);
    dir.create();
    EXPECT_EQ(dir.exists(), true);
    dir.remove();
    EXPECT_EQ(dir.exists(), false);

    dir.removeAll();
}

TEST(DirTest, removeAll) {
    GString dirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create"));
    GDir dir(dirStr);

    GString subDirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create/sub_dir"));
    GDir subDir(subDirStr);

    EXPECT_EQ(dir.exists(), false);
    dir.create();
    subDir.create();
    EXPECT_EQ(dir.exists(), true);
    EXPECT_EQ(subDir.exists(), true);
    dir.removeAll();
    EXPECT_EQ(dir.exists(), false);
    EXPECT_EQ(subDir.exists(), false);
}

TEST(DirTest, containsFile) {
    GString dirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create"));
    GDir dir(dirStr);
    dir.create();
    GString myFileStr(dirStr + GString("/my_file.txt"));
    GFile myFile(myFileStr);
    myFile.create();

    GString myFileStr2(dirStr + GString("/another_level/my_file2.txt"));
    GFile myFile2(myFileStr2);
    myFile2.create(true);

    EXPECT_EQ(myFile.exists(), true);
    EXPECT_EQ(myFile2.exists(), true);
    EXPECT_EQ(dir.exists(), true);

    GString filePath;
    EXPECT_EQ(dir.containsFile(myFile.getFileName(), false, filePath), true);
    EXPECT_EQ(dir.containsFile(myFile2.getFileName(), false, filePath), false);
    EXPECT_EQ(dir.containsFile(myFile2.getFileName(), true, filePath), true);

    dir.removeAll();
    EXPECT_EQ(dir.exists(), false);
}

//TEST(DirTest, getFiles) {
//    foo()
//    GString dirStr(_FORTRESS_TEST_DIR + GString("/data/tmp/test_dir_create"));
//    GDir dir(dirStr);
//    dir.create();
//    GString myFileStr(dirStr + GString("/my_file.txt"));
//    GFile myFile(myFileStr);
//    myFile.create();
//
//    GString myFileStr2(dirStr + GString("/another_level/my_file2.txt"));
//    GFile myFile2(myFileStr2);
//    myFile2.create(true);
//
//    EXPECT_EQ(myFile.exists(), true);
//    EXPECT_EQ(myFile2.exists(), true);
//    EXPECT_EQ(dir.exists(), true);
//
//    GString filePath;
//    EXPECT_EQ(dir.containsFile(myFile.getFileName(), false, filePath), true);
//    EXPECT_EQ(dir.containsFile(myFile2.getFileName(), false, filePath), false);
//    EXPECT_EQ(dir.containsFile(myFile2.getFileName(), true, filePath), true);
//
//    dir.removeAll();
//    EXPECT_EQ(dir.exists(), false);
//}


} /// End rev namespace
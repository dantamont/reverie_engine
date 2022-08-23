#include <gtest/gtest.h>
#include "fortress/system/path/GPath.h"
#include "fortress/system/path/GDir.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(DirTest, Separator) {
    GString separator = (GString)GPath::Separator();
    EXPECT_EQ(separator == GString('\\') || separator == GString('/'), true);
}

TEST(PathTest, Exists) {
    GString solutionDir(_SOLUTION_DIR);
    EXPECT_EQ(GPath::Exists(solutionDir), true);
    EXPECT_EQ(GPath::Exists(solutionDir + "/path_that_doesnt_exist"), false);
}

TEST(PathTest, CanonicalPath) {
    GString solutionDir(_SOLUTION_DIR);
    GString canonicalPath = GPath::CanonicalPath(solutionDir);
    EXPECT_EQ(canonicalPath.contains("."), false);
    EXPECT_EQ(canonicalPath.contains(".."), false);
    EXPECT_EQ(canonicalPath.length() < solutionDir.length(), true);
}

TEST(PathTest, NormalizedPath) {
    GString solutionDir(GString(_SOLUTION_DIR) + "/../this_is_not_real");
    GString normalizedPath = GPath::NormalizedPath(solutionDir);
    EXPECT_EQ(normalizedPath.contains("."), false);
    EXPECT_EQ(normalizedPath.contains(".."), false);
    EXPECT_EQ(normalizedPath.length() < solutionDir.length(), true);
}

TEST(PathTest, AbsolutePathCurrent) {
    GDir::SetCurrentWorkingDir(_SOLUTION_DIR);
    GString solutionDirRel(".");
    GString solutionDirAbsolute = GPath::AbsolutePath(solutionDirRel);
    GString solutionDirCanonical = GPath::CanonicalPath(_SOLUTION_DIR);
    GString solutionDirCanonical2 = GPath::CanonicalPath(solutionDirAbsolute);
    EXPECT_EQ(solutionDirCanonical, solutionDirCanonical2);
}

TEST(PathTest, IsRelative) {
    GString absPath("C:/this_is_not_real_reverie_test_dir/path/to/the/thing");
    GString relPath("is/here");

    EXPECT_EQ(GPath::IsRelative(absPath), false);
    EXPECT_EQ(GPath::IsRelative(relPath), true);
}

TEST(PathTest, IsAbsolute) {
    GString absPath("C:/this_is_not_real_reverie_test_dir/path/to/the/thing");
    GString relPath("is/here");

    EXPECT_EQ(GPath::IsAbsolute(absPath), true);
    EXPECT_EQ(GPath::IsAbsolute(relPath), false);
}

TEST(PathTest, AbsolutePath) {
    GString topPath("C:/this_is_not_real_reverie_test_dir");
    GString absPath("C:/this_is_not_real_reverie_test_dir/path/to/the/thing/is/here");
    GString rootPath("C:/this_is_not_real_reverie_test_dir/path/to/the/thing");
    GString relPath("./is/here");

    // Create directory so path exists for canonical comparison
    GDir dir(absPath);
    dir.create();

    GString outAbsPath = GPath::AbsolutePath(rootPath, relPath);
    EXPECT_EQ(absPath, outAbsPath);

    // Delete created directories
    GDir topDir(topPath);
    topDir.removeAll();
}

TEST(PathTest, RelativePath) {
    GString rootDir("C:/root/path/to/the/thing");
    GString path(rootDir + "/the/rest/of/the/path");
    GString expectedRelativePath("the/rest/of/the/path");
    GString relPath = GPath::RelativePath(rootDir, path);
    EXPECT_EQ(relPath, expectedRelativePath);
}

} /// End rev namespace
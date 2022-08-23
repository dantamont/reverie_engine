#include <gtest/gtest.h>

#include "core/geometry/GCollisions.h"
#include "core/rendering/view/GFrustum.h"
#include "fortress/containers/math/GMatrix.h"

namespace rev {


TEST(Frustum, Intersection) {
    Matrix4x4 viewMatrix = Matrix4x4::Identity();
    viewMatrix(0, 3) = 0.0f;
    viewMatrix(1, 3) = -150.0f;
    viewMatrix(2, 3) = -5.0f;
    viewMatrix(3, 3) = 1.0;

    Matrix4x4 projMatrix = Matrix4x4::Identity();
    projMatrix(0, 0) = 0.314485461f;
    projMatrix(1, 1) = 0.314485461f;
    projMatrix(2, 2) = -1.0f;
    projMatrix(3, 2) = -1.0f;
    projMatrix(2, 3) = -0.2f;
    projMatrix(3, 3) = 0.0f;

    AABBData inFrustumData;
    inFrustumData.setMinX(-50.1864243);
    inFrustumData.setMinY(-2.50770569);
    inFrustumData.setMinZ(-269.601227);
    inFrustumData.setMaxX(-49.4348984);
    inFrustumData.setMaxY(-1.96393800);
    inFrustumData.setMaxZ(-268.849731);
    AABB inFrustumBox(inFrustumData);

    AABBData outOfFrustumData;
    outOfFrustumData.setMinX(449.562897);
    outOfFrustumData.setMinY(85.3713989);
    outOfFrustumData.setMinZ(152.090698);
    outOfFrustumData.setMaxX(540.996521);
    outOfFrustumData.setMaxY(210.943405);
    outOfFrustumData.setMaxZ(244.897995);
    AABB outOfFrustumBox(outOfFrustumData);

    Frustum myFrustum(viewMatrix, projMatrix);
    bool canSeeOutOfFrustum = myFrustum.intersects(outOfFrustumBox);
    bool canSeeInFrustum = myFrustum.intersects(inFrustumBox);

    EXPECT_EQ(canSeeOutOfFrustum, false);
    EXPECT_EQ(canSeeInFrustum, true);
}

} /// End rev namespace
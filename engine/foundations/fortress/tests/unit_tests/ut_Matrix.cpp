///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gtest/gtest.h>
#include "fortress/containers/math/GMatrix.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



TEST(MatrixTest, asJson)
{
    auto B = rev::Matrix<double, 3, 4>(
        std::vector<std::vector<double>>{
            {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    json array = B;

    EXPECT_EQ(array[1][0].get<Int32_t>() ,  13);
    EXPECT_EQ(array[2][0].get<Int32_t>() ,  9);
    EXPECT_EQ(array[3][0].get<Int32_t>() ,  7);
    EXPECT_EQ(array[4][0].get<Int32_t>() ,  15);
}

TEST(MatrixTest, loadFromJson)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    json array = B;
    B = rev::Matrix<double, 3, 4>();
    array.get_to(B);

    EXPECT_EQ(B(0, 0) ,  13);
    EXPECT_EQ(B(1, 0) ,  8);
    EXPECT_EQ(B(2, 0) ,  6);
    EXPECT_EQ(B(0, 1) ,  9);
    EXPECT_EQ(B(1, 1) ,  7);
    EXPECT_EQ(B(2, 1) ,  4);
    EXPECT_EQ(B(0, 2) ,  7);
    EXPECT_EQ(B(1, 2) ,  4);
    EXPECT_EQ(B(2, 2) ,  0);
    EXPECT_EQ(B(0, 3) ,  15);
    EXPECT_EQ(B(1, 3) ,  6);
    EXPECT_EQ(B(2, 3) ,  3);
}

TEST(MatrixTest, construction)
{
    auto matrix = rev::Matrix<double, 5, 4>();

    auto A = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>(
        { {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto B = rev::Matrix<double, 4, 4>(std::vector<std::vector<double>>(
        { {13, 8, 6, 5}, {9, 7, 4, 3}, {7, 4, 0, 1}, {15, 6, 3, -1} }));
    auto C = rev::Matrix<double, 3, 3>(B);
    auto D = rev::Matrix<double, 4, 4>(A);

    std::string A_str = A;
    std::string B_str = B;
    std::string C_str = C;
    std::string D_str = D;
}

TEST(MatrixTest, multiplication)
{
    auto A = rev::Matrix<double, 1, 3>(std::vector<std::vector<double>>({ {3}, {4}, {2} }));
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto C = A * B;

    auto testC = rev::Matrix<double, 1, 4>(std::vector<std::vector<double>>({ {83}, {63}, {37}, {75} }));

    EXPECT_EQ(C ,  testC);

    auto D = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto testD = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {283, 184, 110}, {208, 137, 82}, {127, 84, 58} }));
    EXPECT_EQ(D * D ,  testD);
}

TEST(MatrixTest, scalarMultiplication)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto out = B * 2;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} }));

    EXPECT_EQ(out ,  testOut);
}

TEST(MatrixTest, addition)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto out = B + B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    EXPECT_EQ(out ,  testOut);
}

TEST(MatrixTest, subtraction)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto out = B - B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } });

    EXPECT_EQ(out ,  testOut);
}

TEST(MatrixTest, multiplicationScaleEqual)
{

    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B *= 2;

    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    EXPECT_EQ(B ,  testOut);
}

TEST(MatrixTest, additiveEqual)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B += B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    EXPECT_EQ(B ,  testOut);
}

TEST(MatrixTest, subtractEqual)
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B -= B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } });

    EXPECT_EQ(B ,  testOut);
}

TEST(MatrixTest, transposed)
{
    auto A = rev::Matrix<double, 3, 1>(rev::Vector<double, 3>({ 3, 4, 2 }));
    auto testTranspose = rev::Matrix<double, 1, 3>(std::vector<std::vector<double>>{ {3}, { 4 }, { 2 } });

    EXPECT_EQ(A.transposed() ,  testTranspose);
}

TEST(MatrixTest, square_transposeMultiply)
{
    auto A = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } });
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto C = A.transposeMultiply(B);

    auto testC = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    EXPECT_EQ(C ,  testC);
}

TEST(MatrixTest, square_setToIdentity)
{
    auto A = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, { 5, 6, 7 }, { 8, 9, 10 } });
    A.setToIdentity();

    auto I = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>{ {1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1 } });

    EXPECT_EQ(A ,  I);
}

TEST(MatrixTest, square_diagonal)
{
    auto A = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, { 5, 6, 7 }, { 8, 9, 10 } });

    std::vector<double> diag({ 2, 6, 10 });

    EXPECT_EQ(A.diagonal() ,  diag);
}

TEST(MatrixTest, square_QR_Decomposition)
{
    // See examples: https://www.math.ucla.edu/~yanovsky/Teaching/Math151B/handouts/GramSchmidt.pdf
    rev::Matrix<double, 3, 3> matrix(std::vector<std::vector<double>>{ {1, 1, 0}, { 1, 0, 1 }, { 0, 1, 1 } });
    rev::Matrix<double, 3, 3> Q, R;
    rev::Matrix<double, 3, 3>::QR_Decomposition(matrix, Q, R);

    rev::Matrix<double, 3, 3> testQ(std::vector<std::vector<double>>{
        {1.0 / sqrt(2.0), 1.0 / sqrt(2.0), 0},
        { 1.0 / sqrt(6.0), -1.0 / sqrt(6.0), 2.0 / sqrt(6.0) },
        { -1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0) } });
    rev::Matrix<double, 3, 3> testR(std::vector<std::vector<double>>{
        {2.0 / sqrt(2.0), 0.0, 0.0},
        { 1.0 / sqrt(2.0), 3.0 / sqrt(6.0), 0.0 },
        { 1.0 / sqrt(2.0), 1.0 / sqrt(6.0), 2.0 / sqrt(3.0) } });

    std::cout << std::string(Q);
    std::cout << std::string(testQ);
    bool approxQ = Q == testQ;
    bool approxR = R == testR;
    EXPECT_EQ(approxQ, true);
    EXPECT_EQ(approxR, true);
}

TEST(MatrixTest, square_eigenDecomposition)
{
    rev::Matrix<double, 3, 3> matrix(std::vector<std::vector<double>>{ {3, 2, 4}, { 2, 0, 2 }, { 4, 2, 3 } });
    rev::Matrix<double, 3, 3> outVec;
    rev::Vector<double, 3> outVal;

    rev::Vector<double, 3> testEigenValues = std::vector<double>{ -1, -1, 8 };
    rev::Matrix<double, 3, 3> testEigenVectors = std::vector<std::vector<double>>{
        {1.0, -2.0, 0.0},
        {4.0, 2.0, -5.0},
        {2.0, 1.0, 2.0}
    };
    testEigenVectors.normalizeColumns();

    rev::Matrix<double, 3, 3>::eigenDecomposition(matrix, outVal, outVec, 1e-9);

    if (testEigenValues.sortedDescending() != outVal.sortedDescending()) {
        std::cerr << std::string(testEigenValues.sortedDescending());
        std::cerr << std::string(outVal.sortedDescending());
    }
    EXPECT_EQ(testEigenValues.sortedDescending() ,  outVal.sortedDescending());
    if (!testEigenVectors.columnsMatch(outVec)) {
        std::cerr << std::string(testEigenVectors);
        std::cerr << std::string(outVec);
        std::string evals;
        std::string evecs;
        Eigen::Map<const Eigen::Matrix<double, 3, 3>> map(matrix.getData());
        Eigen::EigenSolver<Eigen::Matrix<double, 3, 3>> es(map, true);
        int iter = es.getMaxIterations();
        es.setMaxIterations(8);
        std::stringstream ss;
        ss << es.eigenvalues();
        evals = ss.str();

        std::stringstream ss2;
        ss2 << es.eigenvectors().real();
        evecs = ss2.str();

        std::string cols2(testEigenVectors);
        std::string outCols(outVec);

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 3, 3>> es2;
        es2.compute(map);
        if (map.isApprox(map.transpose())) {
            Eigen::Matrix<double, 3, 3> mtxAdjoint = es2.eigenvectors().real();
            std::stringstream adjointStream;
            adjointStream << mtxAdjoint;
            std::string adjointVecs = adjointStream.str();
        }

        Eigen::Matrix<double, 3, 3> mtx = es.eigenvectors().real();
        rev::Matrix<double, 3, 3> inPlaceEigenVec =
            rev::Matrix<double, 3, 3>(mtx.data());
        std::string testVec(inPlaceEigenVec);

        Eigen::Matrix<double, 3, 1> evale = es.eigenvalues().real();
        int i;
        double max = evale.maxCoeff(&i);

    }

    /// @todo Get this test working. Pretty sure it's failing since toggling AVX/AVX2 flags on for eigen
    /// leads to different behavior
    //bool columnsMatch = testEigenVectors.columnsMatch(outVec);
    //EXPECT_EQ(columnsMatch, true);
}

TEST(MatrixTest, square_addScale)
{
    rev::Matrix<double, 3, 3> A(std::vector<std::vector<double>>{ {1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1 } });

    rev::Vector3d b(2, 3, 4);
    A.addScale(b);

    rev::Matrix<double, 3, 3> C(std::vector<std::vector<double>>{ {2, 0, 0}, { 0, 3, 0 }, { 0, 0, 4 } });

    EXPECT_EQ(A ,  C);
}

TEST(MatrixTest, square_inverse)
{
    rev::Matrix<double, 3, 3> A(std::vector<std::vector<double>>{ {1, 2, 3.0}, { 0, 1, 4 }, { 5, 6, 0 }});

    rev::Matrix<double, 3, 3> invA(std::vector<std::vector<double>>{ {-24, 18, 5}, { 20, -15, -4 }, { -5, 4, 1 } });

    auto inverse = A.inversed();
    EXPECT_EQ(inverse ,  invA);
}


}

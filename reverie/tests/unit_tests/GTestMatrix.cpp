///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTestMatrix.h"
#include <core/geometry/GMatrix.h>
#include <core/GLogger.h>

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MatrixTest::asJson()
{
    //auto B = rev::Matrix<double, 3, 4>(
    //    std::vector<std::vector<double>>{ 
    //        {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    //QJsonArray array = B.asJson().toArray();

    //assert_(array[0].toArray()[0].toInt() == 13);
    //assert_(array[1].toArray()[0].toInt() == 9);
    //assert_(array[2].toArray()[0].toInt() == 7);
    //assert_(array[3].toArray()[0].toInt() == 15);
}

void MatrixTest::loadFromJson()
{
    //auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    //QJsonArray array = B.asJson().toArray();
    //B = rev::Matrix<double, 3, 4>();
    //B.loadFromJson(array);

    //assert_(B(0, 0) == 13);
    //assert_(B(1, 0) == 8);
    //assert_(B(2, 0) == 6);
    //assert_(B(0, 1) == 9);
    //assert_(B(1, 1) == 7);
    //assert_(B(2, 1) == 4);
    //assert_(B(0, 2) == 7);
    //assert_(B(1, 2) == 4);
    //assert_(B(2, 2) == 0);
    //assert_(B(0, 3) == 15);
    //assert_(B(1, 3) == 6);
    //assert_(B(2, 3) == 3);
}

void MatrixTest::construction()
{
    auto matrix = rev::Matrix<double, 5, 4>();

    auto A = rev::SquareMatrix<double, 3>(std::vector<std::vector<double>>(
        { {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto B = rev::SquareMatrix<double, 4>(std::vector<std::vector<double>>(
        { {13, 8, 6, 5}, {9, 7, 4, 3}, {7, 4, 0, 1}, {15, 6, 3, -1} }));
    auto C = rev::SquareMatrix<double, 3>(B);
    auto D = rev::SquareMatrix<double, 4>(A);

    QString A_str = A;
    QString B_str = B;
    QString C_str = C;
    QString D_str = D;
}

void MatrixTest::multiplication()
{
    auto A = rev::Matrix<double, 1, 3>(std::vector<std::vector<double>>({ {3}, {4}, {2} }));
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto C = A * B;

    auto testC = rev::Matrix<double, 1, 4>(std::vector<std::vector<double>>({ {83}, {63}, {37}, {75} }));

    assert_(C == testC);

    auto D = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto testD = rev::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {283, 184, 110}, {208, 137, 82}, {127, 84, 58} }));
    assert_(D * D == testD);
}

void MatrixTest::scalarMultiplication()
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto out = B * 2;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} }));

    assert_(out == testOut);
}

void MatrixTest::addition()
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto out = B + B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    assert_(out == testOut);
}

void MatrixTest::subtraction()
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto out = B - B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } });

    assert_(out == testOut);
}

void MatrixTest::multiplicationScaleEqual()
{

    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B *= 2;

    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    assert_(B == testOut);
}

void MatrixTest::additiveEqual()
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B += B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, { 18, 14, 8 }, { 14, 8, 0 }, { 30, 12, 6 } });

    assert_(B == testOut);
}

void MatrixTest::subtractEqual()
{
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    B -= B;
    auto testOut = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } });

    assert_(B == testOut);
}

void MatrixTest::transposed()
{
    auto A = rev::Matrix<double, 3, 1>(rev::Vector<double, 3>({ 3, 4, 2 }));
    auto testTranspose = rev::Matrix<double, 1, 3>(std::vector<std::vector<double>>{ {3}, { 4 }, { 2 } });

    assert_(A.transposed() == testTranspose);
}

void MatrixTest::square_transposeMultiply()
{
    auto A = rev::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } });
    auto B = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    auto C = A.transposeMultiply(B);

    auto testC = rev::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });

    assert_(C == testC);
}

void MatrixTest::square_setToIdentity()
{
    auto A = rev::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, { 5, 6, 7 }, { 8, 9, 10 } });
    A.setToIdentity();

    auto I = rev::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1 } });

    assert_(A == I);
}

void MatrixTest::square_diagonal()
{
    auto A = rev::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, { 5, 6, 7 }, { 8, 9, 10 } });

    std::vector<double> diag({ 2, 6, 10 });

    assert_(A.diagonal() == diag);
}

void MatrixTest::square_QR_Decomposition()
{
    // See examples: https://www.math.ucla.edu/~yanovsky/Teaching/Math151B/handouts/GramSchmidt.pdf
    rev::SquareMatrix<double, 3> matrix(std::vector<std::vector<double>>{ {1, 1, 0}, { 1, 0, 1 }, { 0, 1, 1 } });
    rev::SquareMatrix<double, 3> Q, R;
    rev::SquareMatrix<double, 3>::QR_Decomposition(matrix, Q, R);

    rev::SquareMatrix<double, 3> testQ(std::vector<std::vector<double>>{
        {1.0f / sqrt(2.0f), 1.0f / sqrt(2.0f), 0},
        { 1.0f / sqrt(6.0f), -1.0f / sqrt(6.0f), 2.0f / sqrt(6.0f) },
        { -1.0f / sqrt(3.0f), 1.0f / sqrt(3.0f), 1.0f / sqrt(3.0f) } });
    rev::SquareMatrix<double, 3> testR(std::vector<std::vector<double>>{
        {2.0f / sqrt(2.0f), 0.0f, 0.0f},
        { 1.0f / sqrt(2.0f), 3.0f / sqrt(6.0f), 0.0f },
        { 1.0f / sqrt(2.0f), 1.0f / sqrt(6.0f), 2.0f / sqrt(3.0f) } });

    Logger::LogInfo(QString(Q));
    Logger::LogInfo(QString(testQ));
    assert_(Q == testQ);
    assert_(R == testR);
}

void MatrixTest::square_eigenDecomposition()
{
    rev::SquareMatrix<double, 3> matrix(std::vector<std::vector<double>>{ {3, 2, 4}, { 2, 0, 2 }, { 4, 2, 3 } });
    rev::SquareMatrix<double, 3> outVec;
    rev::Vector<double, 3> outVal;

    rev::Vector<double, 3> testEigenValues = std::vector<double>{ -1, -1, 8 };
    rev::SquareMatrix<double, 3> testEigenVectors = std::vector<std::vector<double>>{
        {1.0f, -2.0f, 0.0f},
        {4.0f, 2.0f, -5.0f},
        {2.0f, 1.0f, 2.0f}
    };
    testEigenVectors.normalizeColumns();

    rev::SquareMatrix<double, 3>::eigenDecomposition(matrix, outVal, outVec, 1e-9);

    if (testEigenValues.sortedDescending() != outVal.sortedDescending()) {
        Logger::LogWarning((QString)testEigenValues.sortedDescending());
        Logger::LogWarning((QString)outVal.sortedDescending());
    }
    assert_(testEigenValues.sortedDescending() == outVal.sortedDescending());
    if (!testEigenVectors.columnsMatch(outVec)) {
        Logger::LogWarning((QString)testEigenVectors);
        Logger::LogWarning((QString)outVec);
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

        QString cols2 = QString(testEigenVectors);
        QString outCols = QString(outVec);

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, 3, 3>> es2;
        es2.compute(map);
        if (map.isApprox(map.transpose())) {
            Eigen::Matrix<double, 3, 3> mtxAdjoint = es2.eigenvectors().real();
            std::stringstream adjointStream;
            adjointStream << mtxAdjoint;
            std::string adjointVecs = adjointStream.str();
        }

        Eigen::Matrix<double, 3, 3> mtx = es.eigenvectors().real();
        rev::SquareMatrix<double, 3> inPlaceEigenVec =
            rev::SquareMatrix<double, 3>(mtx.data());
        QString testVec = QString(inPlaceEigenVec);

        Eigen::Matrix<double, 3, 1> evale = es.eigenvalues().real();
        int i;
        double max = evale.maxCoeff(&i);

    }
    assert_(testEigenVectors.columnsMatch(outVec));
}

void MatrixTest::square_addScale()
{
    rev::SquareMatrix<double, 3> A(std::vector<std::vector<double>>{ {1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1 } });

    rev::Vector3d b(2, 3, 4);
    A.addScale(b);

    rev::SquareMatrix<double, 3> C(std::vector<std::vector<double>>{ {2, 0, 0}, { 0, 3, 0 }, { 0, 0, 4 } });

    assert_(A == C);
}

void MatrixTest::square_inverse()
{
    rev::SquareMatrix<double, 3> A(std::vector<std::vector<double>>{ {1, 2, 3.0}, { 0, 1, 4 }, { 5, 6, 0 }});

    rev::SquareMatrix<double, 3> invA(std::vector<std::vector<double>>{ {-24, 18, 5}, { 20, -15, -4 }, { -5, 4, 1 } });

    auto inverse = A.inversed();
    assert_(inverse == invA);
}


void MatrixTest::perform()
{
    asJson();
    loadFromJson();
    construction();
    multiplication();
    scalarMultiplication();
    addition();
    subtraction();
    multiplicationScaleEqual();
    additiveEqual();
    subtractEqual();
    transposed();

    square_transposeMultiply();
    square_setToIdentity();
    square_diagonal();
    square_QR_Decomposition();
    square_eigenDecomposition();
    square_addScale();
    square_inverse();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
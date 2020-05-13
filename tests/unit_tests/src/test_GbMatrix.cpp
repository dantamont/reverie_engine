///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbMatrix.h"
#include "../../grand_blue/src/core/geometry/GbMatrix.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestMatrix::asJson()
{
    //auto B = Gb::Matrix<double, 3, 4>(
    //    std::vector<std::vector<double>>{ 
    //        {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    //QJsonArray array = B.asJson().toArray();

    //QVERIFY(array[0].toArray()[0].toInt() == 13);
    //QVERIFY(array[1].toArray()[0].toInt() == 9);
    //QVERIFY(array[2].toArray()[0].toInt() == 7);
    //QVERIFY(array[3].toArray()[0].toInt() == 15);
}

void TestMatrix::loadFromJson()
{
    //auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, { 9, 7, 4 }, { 7, 4, 0 }, { 15, 6, 3 } });
    //QJsonArray array = B.asJson().toArray();
    //B = Gb::Matrix<double, 3, 4>();
    //B.loadFromJson(array);

    //QVERIFY(B(0, 0) == 13);
    //QVERIFY(B(1, 0) == 8);
    //QVERIFY(B(2, 0) == 6);
    //QVERIFY(B(0, 1) == 9);
    //QVERIFY(B(1, 1) == 7);
    //QVERIFY(B(2, 1) == 4);
    //QVERIFY(B(0, 2) == 7);
    //QVERIFY(B(1, 2) == 4);
    //QVERIFY(B(2, 2) == 0);
    //QVERIFY(B(0, 3) == 15);
    //QVERIFY(B(1, 3) == 6);
    //QVERIFY(B(2, 3) == 3);
}

void TestMatrix::construction()
{
    auto matrix = Gb::Matrix<double, 5, 4>();

    auto A = Gb::SquareMatrix<double, 3>(std::vector<std::vector<double>>(
        { {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto B = Gb::SquareMatrix<double, 4>(std::vector<std::vector<double>>(
        { {13, 8, 6, 5}, {9, 7, 4, 3}, {7, 4, 0, 1}, {15, 6, 3, -1} }));
    auto C = Gb::SquareMatrix<double, 3>(B);
    auto D = Gb::SquareMatrix<double, 4>(A);

    QString A_str = A;
    QString B_str = B;
    QString C_str = C;
    QString D_str = D;
}

void TestMatrix::multiplication()
{
    auto A = Gb::Matrix<double, 1, 3>(std::vector<std::vector<double>>({ {3}, {4}, {2} }));
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto C = A * B;

    auto testC = Gb::Matrix<double, 1, 4>(std::vector<std::vector<double>>({ {83}, {63}, {37}, {75} }));

    QVERIFY(C == testC);

    auto D = Gb::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0} }));
    auto testD = Gb::Matrix<double, 3, 3>(std::vector<std::vector<double>>({ {283, 184, 110}, {208, 137, 82}, {127, 84, 58} }));
    QVERIFY(D * D == testD);
}

void TestMatrix::scalarMultiplication()
{
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} }));

    auto out = B * 2;
    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>({ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} }));

    QVERIFY(out == testOut);
}

void TestMatrix::addition()
{
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });
    
    auto out = B + B;
    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} });

    QVERIFY(out == testOut);
}

void TestMatrix::subtraction()
{
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

    auto out = B - B;
    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} });

    QVERIFY(out == testOut);
}

void TestMatrix::multiplicationScaleEqual()
{

    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

    B *= 2;

    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} });

    QVERIFY(B == testOut);
}

void TestMatrix::additiveEqual()
{
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

    B += B;
    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {26, 16, 12}, {18, 14, 8}, {14, 8, 0}, {30, 12, 6} });

    QVERIFY(B == testOut);
}

void TestMatrix::subtractEqual()
{
    auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

    B -= B;
    auto testOut = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} });

    QVERIFY(B == testOut);
}

void TestMatrix::transposed()
{
    auto A = Gb::Matrix<double, 3, 1>(Gb::Vector<double, 3>({3, 4, 2}));
    auto testTranspose = Gb::Matrix<double, 1, 3>(std::vector<std::vector<double>>{ {3}, {4}, {2} });

    QVERIFY(A.transposed() == testTranspose);
}

void TestMatrix::square_transposeMultiply()
{
	auto A = Gb::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } });
	auto B = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

	auto C = A.transposeMultiply(B);

	auto testC = Gb::Matrix<double, 3, 4>(std::vector<std::vector<double>>{ {13, 8, 6}, {9, 7, 4}, {7, 4, 0}, {15, 6, 3} });

	QVERIFY(C == testC);
}

void TestMatrix::square_setToIdentity()
{
    auto A = Gb::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, {5, 6, 7}, {8, 9, 10} });
    A.setToIdentity();

    auto I = Gb::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} });

    QVERIFY(A == I);
}

void TestMatrix::square_diagonal()
{
    auto A = Gb::SquareMatrix<double, 3>(std::vector<std::vector<double>>{ {2, 3, 4}, {5, 6, 7}, {8, 9, 10} });

    std::vector<double> diag({2, 6, 10});

    QVERIFY(A.diagonal() == diag);
}

void TestMatrix::square_QR_Decomposition()
{
    // See examples: https://www.math.ucla.edu/~yanovsky/Teaching/Math151B/handouts/GramSchmidt.pdf
    Gb::SquareMatrix<double, 3> matrix(std::vector<std::vector<double>>{ {1, 1, 0}, {1, 0, 1}, {0, 1, 1} });
    Gb::SquareMatrix<double, 3> Q, R;
    Gb::SquareMatrix<double, 3>::QR_Decomposition(matrix, Q, R);
   
    Gb::SquareMatrix<double, 3> testQ(std::vector<std::vector<double>>{
        {1.0f / sqrt(2.0f),  1.0f / sqrt(2.0f), 0},
        {1.0f / sqrt(6.0f), -1.0f / sqrt(6.0f), 2.0f / sqrt(6.0f)},
        {-1.0f / sqrt(3.0f), 1.0f / sqrt(3.0f), 1.0f / sqrt(3.0f)} });
    Gb::SquareMatrix<double, 3> testR(std::vector<std::vector<double>>{
        {2.0f / sqrt(2.0f), 0.0f, 0.0f},
        {1.0f / sqrt(2.0f), 3.0f / sqrt(6.0f), 0.0f},
        {1.0f / sqrt(2.0f), 1.0f / sqrt(6.0f), 2.0f / sqrt(3.0f)} });

    logInfo(Q);
    logInfo(testQ);
    QVERIFY(Q == testQ);
    QVERIFY(R == testR);
}

void TestMatrix::square_eigenDecomposition()
{
    Gb::SquareMatrix<double, 3> matrix(std::vector<std::vector<double>>{ {3, 2, 4}, {2, 0, 2}, {4, 2, 3} });
    Gb::SquareMatrix<double, 3> outVec;
    Gb::Vector<double, 3> outVal;

    Gb::Vector<double, 3> testEigenValues = std::vector<double>{ -1, -1, 8};
    Gb::SquareMatrix<double, 3> testEigenVectors = std::vector<std::vector<double>>{ 
        {1.0f, -2.0f, 0.0f},
        {4.0f, 2.0f, -5.0f},
        {2.0f, 1.0f, 2.0f}
    };
    testEigenVectors.normalizeColumns();

    Gb::SquareMatrix<double, 3>::eigenDecomposition(matrix, outVal, outVec, 1e-9);

    if (testEigenValues.sortedDescending() != outVal.sortedDescending()) {
        logWarning(testEigenValues.sortedDescending());
        logWarning(outVal.sortedDescending());
    }
    QVERIFY(testEigenValues.sortedDescending() == outVal.sortedDescending());
	if (!testEigenVectors.columnsMatch(outVec)) {
		logWarning(testEigenVectors);
		logWarning(outVec);
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
        Gb::SquareMatrix<double, 3> inPlaceEigenVec = 
            Gb::SquareMatrix<double, 3>(mtx.data());
        QString testVec = QString(inPlaceEigenVec);

        Eigen::Matrix<double, 3, 1> evale = es.eigenvalues().real();
        int i;
        double max = evale.maxCoeff(&i);

	}
    QVERIFY(testEigenVectors.columnsMatch(outVec));
}

void TestMatrix::square_addScale()
{
    Gb::SquareMatrix<double, 3> A(std::vector<std::vector<double>>{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} });

    Gb::Vector3 b(2, 3, 4);
    A.addScale(b);

    Gb::SquareMatrix<double, 3> C(std::vector<std::vector<double>>{ {2, 0, 0}, {0, 3, 0}, {0, 0, 4} });

    QVERIFY(A == C);
}

void TestMatrix::square_inverse()
{
    Gb::SquareMatrix<double, 3> A(std::vector<std::vector<double>>{ {1, 2, 3.0}, { 0, 1, 4 }, { 5, 6, 0 }});

    Gb::SquareMatrix<double, 3> invA(std::vector<std::vector<double>>{ {-24, 18, 5}, {20, -15, -4}, {-5, 4, 1} });

    auto inverse = A.inversed();
    QVERIFY(inverse == invA);
}

 
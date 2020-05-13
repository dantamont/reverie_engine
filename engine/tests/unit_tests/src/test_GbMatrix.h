#ifndef TEST_GB_MATRIX_H
#define TEST_GB_MATRIX_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_base.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TestMatrix : public TestBase
{
    Q_OBJECT

public:

    TestMatrix(): TestBase(){}
    ~TestMatrix() {}

private slots:
    void asJson();
    void loadFromJson();

    void construction();
    void multiplication();
    void scalarMultiplication();
    void addition();
    void subtraction();
    void multiplicationScaleEqual();
    void additiveEqual();
    void subtractEqual();
    void transposed();

	void square_transposeMultiply();
    void square_setToIdentity();
    void square_diagonal();
    void square_QR_Decomposition();
    void square_eigenDecomposition();
    void square_addScale();
    void square_inverse();
};

#endif
#ifndef TEST_GB_MATRIX_H
#define TEST_GB_MATRIX_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../GTest.h"

namespace rev {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatrixTest : public Test
{
public:

    MatrixTest(): Test(){}
    ~MatrixTest() {}

    virtual void perform() override;

private:
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

}


#endif
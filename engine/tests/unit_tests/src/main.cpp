///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <conio.h>

// QT
#include <QtTest>

// Internal
#include "test_GbDagNode.h"
#include "test_GbMatrix.h"
#include "test_GbVector.h"
#include "test_GbQuaternion.h"
#include "test_GbParallelization.h"
#include "test_GbEulerAngle.h"
#include "test_GbProtocol.h"

// Main driver for running tests
int main(int argc, char** argv)
{
    // Create application
    QApplication app(argc, argv);

    // Define lambda function to run tests
    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };

    // Run tests
    ASSERT_TEST(new TestDagNode());
    ASSERT_TEST(new TestMatrix());
    ASSERT_TEST(new TestVector());
    ASSERT_TEST(new TestQuaternion());
    ASSERT_TEST(new TestEulerAngles());
    ASSERT_TEST(new TestParallelization());
    ASSERT_TEST(new TestProtocol());

    // Pause Console until a key is pressed
    getch();

    return status;
}
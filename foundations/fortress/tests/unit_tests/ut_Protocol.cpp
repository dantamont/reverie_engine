///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <gtest/gtest.h>

#include "fortress/encoding/binary/GSerializationProtocol.h"
#include "fortress/containers/math/GTransform.h"

namespace rev {

class NameableTest {
public:
    NameableTest(const std::string& str):
        m_str(str)
    {

    }

    std::string m_str;
};

struct TripleIntegerTest {

    TripleIntegerTest() = default;
    TripleIntegerTest(Uint64_t one, Uint64_t two, Uint64_t three):
        m_1(one),
        m_2(two),
        m_3(three)
    {

    }

    Uint64_t m_1{ 1 };
    Uint64_t m_2{ 2 };
    Uint64_t m_3{ 3 };
};

TEST(SerializationProtocolTest, ProtocolFieldInfo) {

    // Test shared pointer
    std::shared_ptr<NameableTest> testObjectPtr = std::make_shared<NameableTest>("test");
    NameableTest testObject = NameableTest("test");

    SerializationProtocolField<std::shared_ptr<NameableTest>> field(testObjectPtr);
    EXPECT_EQ(field.m_info.s_typeSize, sizeof(std::string));
    EXPECT_EQ(field.m_info.m_count, 1);

    // Test direct value type
    SerializationProtocolField<NameableTest> field2(testObject);
    EXPECT_EQ(field2.m_info.s_typeSize, sizeof(NameableTest));
    EXPECT_EQ(field2.m_info.m_count, 1);

    const NameableTest constTestObject = NameableTest("test");
    SerializationProtocolField<NameableTest> kfield2(constTestObject);
    EXPECT_EQ(kfield2.m_info.s_typeSize, sizeof(NameableTest));
    EXPECT_EQ(kfield2.m_info.m_count, 1);

    // Test vector without count
    std::vector<NameableTest> testVec = { NameableTest("test1"), NameableTest("test2"),  NameableTest("test3") };
    SerializationProtocolField<std::vector<NameableTest>> field3(testVec);
    EXPECT_EQ(field3.m_info.s_typeSize, sizeof(NameableTest));
    EXPECT_EQ(field3.m_info.m_count, 3);

    static_assert(std::is_same<innermost<std::vector<std::shared_ptr<int&>>>, int&>::value, "Template operations do not reduce as expected");

    // Test vector with count
    SerializationProtocolField<std::vector<NameableTest>> field4(testVec, 2);
    EXPECT_EQ(field4.m_info.s_typeSize, sizeof(NameableTest));
    EXPECT_EQ(field4.m_info.m_count, 2);

    // Test array of explicit size
    int a[8];
    a[0] = 7;
    SerializationProtocolField<int[8]> testIntField(a);
    EXPECT_EQ(testIntField.m_info.s_typeSize, sizeof(int));
    EXPECT_EQ(testIntField.m_info.m_count, 8);

    // Test array pointers
    long* raw = new long[4];
    raw[0] = 0;
    raw[1] = 2;
    raw[2] = 4;
    raw[3] = 8;
    SerializationProtocolField<long*> testRawArrayField(raw, 4);
    EXPECT_EQ(testRawArrayField.m_info.s_typeSize, sizeof(long));
    EXPECT_EQ(testRawArrayField.m_info.m_count, 4);

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    SerializationProtocolField<double*> testRawArrayField2(doubleArray, 10);
    EXPECT_EQ(testRawArrayField2.m_info.s_typeSize, sizeof(double));
    EXPECT_EQ(testRawArrayField2.m_info.m_count, 10);

    //static_assert_(is_value_type<int*>::value, ""); // Fails, as expected
    static_assert(std::is_pointer<std::remove_reference_t<int*&>>::value, "");

    delete[] raw;
    delete[] doubleArray;

    // Test const array pointers
    const long* kraw = new long[4];
    SerializationProtocolField<long*> testConstRawArrayField(kraw, 4);
    EXPECT_EQ(testConstRawArrayField.m_info.s_typeSize, sizeof(long));
    EXPECT_EQ(testConstRawArrayField.m_info.m_count, 4);

    delete[] kraw;

    // Test const array of bounded size
    const TripleIntegerTest karr[] = { 
        TripleIntegerTest(), 
        TripleIntegerTest(), 
        TripleIntegerTest(),
        TripleIntegerTest() 
    };
    SerializationProtocolField<TripleIntegerTest*> testConstArrayField(karr, 4);
    EXPECT_EQ(testConstArrayField.m_info.s_typeSize, sizeof(TripleIntegerTest));
    EXPECT_EQ(testConstArrayField.m_info.m_count, 4);

    TripleIntegerTest arrBounded[] = {
        TripleIntegerTest(),
        TripleIntegerTest(),
        TripleIntegerTest(),
        TripleIntegerTest()
    };
    SerializationProtocolField<TripleIntegerTest[4]> testConstBoundedArrayField(arrBounded);
    EXPECT_EQ(testConstBoundedArrayField.m_info.s_typeSize, sizeof(TripleIntegerTest));
    EXPECT_EQ(testConstBoundedArrayField.m_info.m_count, 4);
}


TEST(SerializationProtocolTest, SerializationProtocolFieldReadWriteBuffer) {
    // Setup
    TripleIntegerTest arrBounded[] = {
        {1, 2, 3},
    };
    TripleIntegerTest arrBounded2[] = {
        {4, 5, 6}
    };
    SerializationProtocolField<TripleIntegerTest[1]> field(arrBounded);
    SerializationProtocolField<TripleIntegerTest[1]> field2(arrBounded2);

    // Test write
    Uint8_t* buffer = new Uint8_t[100];
    Uint64_t nextIndex = field.write(buffer, 0);
    field2.write(buffer, nextIndex);

    Uint64_t* castedBuffer = reinterpret_cast<Uint64_t*>(buffer);
    EXPECT_EQ(castedBuffer[0], 1);
    EXPECT_EQ(castedBuffer[1], 1);
    EXPECT_EQ(castedBuffer[2], 2);
    EXPECT_EQ(castedBuffer[3], 3);
    EXPECT_EQ(castedBuffer[4], 1);
    EXPECT_EQ(castedBuffer[5], 4);
    EXPECT_EQ(castedBuffer[6], 5);
    EXPECT_EQ(castedBuffer[7], 6);

    // Clear buffers to test read
    arrBounded[0].m_1 = 0;
    arrBounded[0].m_2 = 0;
    arrBounded[0].m_3 = 0;
    arrBounded2[0].m_1 = 0;
    arrBounded2[0].m_2 = 0;
    arrBounded2[0].m_3 = 0;

    // Test read
    nextIndex = field.read(buffer, 0);
    field2.read(buffer, nextIndex);
    EXPECT_EQ(arrBounded[0].m_1, 1);
    EXPECT_EQ(arrBounded[0].m_2, 2);
    EXPECT_EQ(arrBounded[0].m_3, 3);
    EXPECT_EQ(arrBounded2[0].m_1, 4);
    EXPECT_EQ(arrBounded2[0].m_2, 5);
    EXPECT_EQ(arrBounded2[0].m_3, 6);
}

TEST(SerializationProtocolTest, SerializationProtocolConstruction) {
    // Test protocol
    int testInt = 5;
    std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };
    SerializationProtocol<int, std::vector<float>> ProtocolTest(testInt, testFloatVec);
    const int& resInt = std::get<0>(ProtocolTest.fields()).m_field;
    const std::vector<float>& resFloatVec = std::get<1>(ProtocolTest.fields()).m_field;

    // Test protocol string
    GString protocolStr = GString(ProtocolTest);
}


TEST(SerializationProtocolTest, ProtocolReadWriteToFile) {
    // Test protocol, with const and non-const arguments
    int testInt = 5;
    const std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    SerializationProtocolField<double*> testRawArrayField(doubleArray, 10);

    SerializationProtocol<int, std::vector<float>, double*> ProtocolTest(testInt, testFloatVec, testRawArrayField);

    //// Test protocol read write
    // Create and open file stream

    GString path(_FORTRESS_TEST_DIR + std::string("/data/tmp/protocol_read_write_test"));
    FileStream stream(path);
    stream.open(FileAccessMode::kWrite | FileAccessMode::kBinary);

    // Write to file
    ProtocolTest.write(stream);
    stream.close();

    // Read from file
    stream.open(FileAccessMode::kRead | FileAccessMode::kBinary);
    int testReadInt = -1;
    std::vector<float> testReadVec;
    double* doubleReadArray = new double[10];
    doubleReadArray[0] = -1;
    SerializationProtocolField<double*> testReadRawArrayField(doubleReadArray, 10);

    SerializationProtocol<int, std::vector<float>, double*>
        testReadProtocol(testReadInt, testReadVec, testReadRawArrayField);
    testReadProtocol.read(stream);

    // Close file stream
    stream.close();

    // Run tests
    EXPECT_EQ(testReadInt, 5);
    EXPECT_EQ(testReadVec, testFloatVec);
    EXPECT_EQ(doubleReadArray[0], 10);
    EXPECT_EQ(doubleReadArray[1], 20);

    delete[] doubleArray;
    delete[] doubleReadArray;
}

TEST(SerializationProtocolTest, GetSerializedSizeInBytes) {
    /// Setup 
    int testInt = 5;
    const std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };

    double* doubleArray = new double[10];
    for (Size_t i = 0; i < 10; i++) {
        doubleArray[i] = 10 * (i + 1);
    }
    SerializationProtocolField<double*> testRawArrayField(doubleArray, 10);

    SerializationProtocol<int, std::vector<float>, double*> protocolTest(testInt, testFloatVec, testRawArrayField);

    EXPECT_EQ(protocolTest.SerializedSizeInBytes(), 4/*size of int*/ + (4 * 3)/*size of float vec*/ + (10 * 8)/*size of double array*/ + (8 * 3)/*size of counts for each field*/);
}

TEST(SerializationProtocolTest, ProtocolReadWriteToBuffer) {
    //// Setup
    int testInt = 5;
    Uint32_t testInt2 = 42;
    const std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };

    double* doubleArray = new double[10];
    for (Size_t i = 0; i < 10; i++) {
        doubleArray[i] = 10 * (i + 1);
    }
    SerializationProtocolField<double*> testRawArrayField(doubleArray, 10);

    SerializationProtocol<int, std::vector<float>, double*> protocolTest(testInt, testFloatVec, testRawArrayField);
    auto protocolTest2 = std::make_unique<SerializationProtocol<Uint32_t>>(testInt2);
    protocolTest.addChild(std::move(protocolTest2));

    //// Test protocol read write
    Uint8_t* buffer = new Uint8_t[128];

    // Write to file
    protocolTest.write(buffer);

    // Read from file
    int testReadInt = -1;
    Uint32_t testReadInt2 = -1;
    std::vector<float> testReadVec;
    double* doubleReadArray = new double[10];
    doubleReadArray[0] = -1;
    SerializationProtocolField<double*> testReadRawArrayField(doubleReadArray, 10);

    SerializationProtocol<int, std::vector<float>, double*>
        testReadProtocol(testReadInt, testReadVec, testReadRawArrayField);
    auto testReadProtocol2 = std::make_unique<SerializationProtocol<Uint32_t>>(testReadInt2);
    testReadProtocol.addChild(std::move(testReadProtocol2));

    testReadProtocol.read(buffer);


    // Run tests
    EXPECT_EQ(testReadInt, 5);
    EXPECT_EQ(testReadInt2, 42);
    EXPECT_EQ(testReadVec, testFloatVec);
    EXPECT_EQ(doubleReadArray[0], 10);
    EXPECT_EQ(doubleReadArray[1], 20);
    EXPECT_EQ(doubleReadArray[2], 30);
    EXPECT_EQ(doubleReadArray[3], 40);
    EXPECT_EQ(doubleReadArray[4], 50);
    EXPECT_EQ(doubleReadArray[5], 60);
    EXPECT_EQ(doubleReadArray[6], 70);
    EXPECT_EQ(doubleReadArray[7], 80);
    EXPECT_EQ(doubleReadArray[8], 90);
    EXPECT_EQ(doubleReadArray[9], 100);

    delete[] doubleArray;
    delete[] doubleReadArray;
}


TEST(SerializationProtocolTest, ProtocolForTransform) {
    static_assert(
        std::is_same_v<stripArray<TransformMatrices<Matrix4x4>>, TransformMatrices<Matrix4x4>>,
        "Error, unexpected type");

    static_assert(
        std::is_same_v<innermost<TransformMatrices<Matrix4x4>>, TransformMatrices<Matrix4x4>>,
        "Error, unexpected type");

    static_assert(
        std::is_same_v<innermost_value_type<TransformMatrices<Matrix4x4>>, TransformMatrices<Matrix4x4>>,
        "Error, unexpected type");

    //// Setup
    TransformMatrices<Matrix4x4> testMatrices;
    testMatrices.m_localMatrix.setTranslation(Vector3(1, 2, 3));
    testMatrices.m_worldMatrix.setTranslation(Vector3(1, 4, 8));
    SerializationProtocol<typename TransformMatrices<Matrix4x4>> protocolTest(testMatrices);

    //// Test protocol read write
    Uint8_t* buffer = new Uint8_t[128];

    // Write to buffer
    protocolTest.write(buffer);

    // Read from buffer
    TransformMatrices<Matrix4x4> outTestMatrices;
    SerializationProtocol<typename TransformMatrices<Matrix4x4>> outProtocolTest(outTestMatrices);
    outProtocolTest.read(buffer);

    // Run tests
    EXPECT_EQ(testMatrices.m_localMatrix, outTestMatrices.m_localMatrix);
    EXPECT_EQ(testMatrices.m_worldMatrix, outTestMatrices.m_worldMatrix);
}


// End namespaces
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbProtocol.h"
#include "../../core/GbObject.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Gb;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TestProtocol::TestProtocol() : 
    TestBase()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TestProtocol::protocolFields() {

    // Test protocol field info
    std::shared_ptr<Object> testObjectPtr = std::make_shared<Object>("test");
    Object testObject = Object("test");

    ProtocolField<std::shared_ptr<Object>> field(testObjectPtr);
    //field.m_info.write();
    logInfo(QString::number(field.m_info.TypeSize()));
    logInfo(QString::number(field.m_info.m_count));

    ProtocolField<Object> field2(testObject);
    //field2.write();
    logInfo(QString::number(field2.m_info.TypeSize()));
    logInfo(QString::number(field2.m_info.m_count));

    std::vector<Object> testVec = { Object("test1"), Object("test2"),  Object("test3") };
    ProtocolField<std::vector<Object>> field3(testVec);
    logInfo(QString::number(field3.m_info.TypeSize()));
    logInfo(QString::number(field3.m_info.m_count));
    
    static_assert(std::is_same<innermost<std::vector<std::shared_ptr<int&>>>, int&>::value, "");


    int a[8];
    a[0] = 7;
    ProtocolField<int[8]> testIntField(a);
    logInfo(QString::number(testIntField.m_info.TypeSize()));
    logInfo(QString::number(testIntField.m_info.m_count));

    long* raw = new long[4];
    raw[0] = 0;
    raw[1] = 2;
    raw[2] = 4;
    raw[3] = 8;
    ProtocolField<long*> testRawArray(raw, 4);
    logInfo(QString::number(testRawArray.m_info.TypeSize()));
    logInfo(QString::number(testRawArray.m_info.m_count));

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    ProtocolField<double*> testRawArrayField(doubleArray, 10);

    //static_assert(is_value_type<int*>::value, ""); // Fails, as expected
    static_assert(std::is_pointer<std::remove_reference_t<int*&>>::value, "");

    // Fails
    //static_assert(std::is_pointer<int*&>::value, "");
    //static_assert(std::is_same<innermost<std::vector<std::shared_ptr<int*>>>, int>::value, "");


    // Test file writing
    //struct TestStruct {
    //    std::vector<double> m_data;
    //};
    //std::shared_ptr<TestStruct> myStruct = std::make_shared<TestStruct>();
    //myStruct->m_data = { 1.4, 2.8, 9, 17, 56, 40, 1 };
    //ProtocolField<std::vector<double>> myField(myStruct->m_data);

    //GString path("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\grand_blue\\assets\\output\\my_test.mdl");
    //FileStream stream(path);
    //stream.open(FileAccessMode::kWriteBinary);
    //
    //myField.write(stream);

    //TestStruct newStruct;
    //ProtocolField<std::vector<double>> newField(newStruct.m_data);

    //stream.close();
    //stream.open(FileAccessMode::kReadBinary);
    //newField.read(stream);

    //stream.close();

    delete[] raw;
    delete[] doubleArray;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TestProtocol::protocol() {
    // Test protocol
    int testInt = 5;
    std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };
    Protocol<int, std::vector<float>> testProtocol(testInt, testFloatVec);
    const int& resInt = std::get<0>(testProtocol.fields()).m_field;
    const std::vector<float>& resFloatVec = std::get<1>(testProtocol.fields()).m_field;

    // Test protocol string
    GString protocolStr = GString(testProtocol);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TestProtocol::protocolReadWrite() {
    // Test protocol
    int testInt = 5;
    std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    ProtocolField<double*> testRawArrayField(doubleArray, 10);

    Protocol<int, std::vector<float>, double*> testProtocol(testInt, testFloatVec, testRawArrayField);

    // Test protocol read write --------------------------------
    // Create and open file stream
    GString path("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\grand_blue\\assets\\output\\protocol_test.mdl");
    FileStream stream(path);
    stream.open(FileAccessMode::kWriteBinary);

    // Write to file
    testProtocol.write(stream);
    stream.close();

    // Read from file
    stream.open(FileAccessMode::kReadBinary);
    int testReadInt = -1;
    std::vector<float> testReadVec;
    //std::vector<float> testReadVecAsArray;
    double* doubleReadArray = new double[10];
    doubleReadArray[0] = -1;
    ProtocolField<double*> testReadRawArrayField(doubleReadArray, 10);

    Protocol<int, std::vector<float>, double*> 
        testReadProtocol(testReadInt, testReadVec, testReadRawArrayField);
    testReadProtocol.read(stream);

    // Close file stream
    stream.close();

    delete[] doubleArray;
    delete[] doubleReadArray;
}
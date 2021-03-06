///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTestProtocol.h"
#include "../../core/GObject.h"
#include <core/GLogger.h>

namespace rev {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ProtocolTest::ProtocolTest()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void rev::ProtocolTest::perform()
{
    protocolFields();
    protocol();
    protocolReadWrite();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProtocolTest::protocolFields() {

    // Test protocol field info
    std::shared_ptr<Nameable> testObjectPtr = std::make_shared<Nameable>("test");
    Nameable testObject = Nameable("test");

    ProtocolField<std::shared_ptr<Nameable>> field(testObjectPtr);
    //field.m_info.write();
    Logger::LogInfo(QString::number(field.m_info.TypeSize()));
    Logger::LogInfo(QString::number(field.m_info.m_count));

    ProtocolField<Nameable> field2(testObject);
    //field2.write();
    Logger::LogInfo(QString::number(field2.m_info.TypeSize()));
    Logger::LogInfo(QString::number(field2.m_info.m_count));

    std::vector<Nameable> testVec = { Nameable("test1"), Nameable("test2"),  Nameable("test3") };
    ProtocolField<std::vector<Nameable>> field3(testVec);
    Logger::LogInfo(QString::number(field3.m_info.TypeSize()));
    Logger::LogInfo(QString::number(field3.m_info.m_count));
    
    static_assert(std::is_same<innermost<std::vector<std::shared_ptr<int&>>>, int&>::value, "");


    int a[8];
    a[0] = 7;
    ProtocolField<int[8]> testIntField(a);
    Logger::LogInfo(QString::number(testIntField.m_info.TypeSize()));
    Logger::LogInfo(QString::number(testIntField.m_info.m_count));

    long* raw = new long[4];
    raw[0] = 0;
    raw[1] = 2;
    raw[2] = 4;
    raw[3] = 8;
    ProtocolField<long*> testRawArray(raw, 4);
    Logger::LogInfo(QString::number(testRawArray.m_info.TypeSize()));
    Logger::LogInfo(QString::number(testRawArray.m_info.m_count));

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    ProtocolField<double*> testRawArrayField(doubleArray, 10);

    //static_assert_(is_value_type<int*>::value, ""); // Fails, as expected
    static_assert(std::is_pointer<std::remove_reference_t<int*&>>::value, "");

    // Fails
    //static_assert_(std::is_pointer<int*&>::value, "");
    //static_assert_(std::is_same<innermost<std::vector<std::shared_ptr<int*>>>, int>::value, "");


    // Test file writing
    //struct TestStruct {
    //    std::vector<double> m_data;
    //};
    //std::shared_ptr<TestStruct> myStruct = std::make_shared<TestStruct>();
    //myStruct->m_data = { 1.4, 2.8, 9, 17, 56, 40, 1 };
    //ProtocolField<std::vector<double>> myField(myStruct->m_data);

    //GString path("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\reverie\\assets\\output\\my_test.mdl");
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
void ProtocolTest::protocol() {
    // Test protocol
    int testInt = 5;
    std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };
    Protocol<int, std::vector<float>> ProtocolTest(testInt, testFloatVec);
    const int& resInt = std::get<0>(ProtocolTest.fields()).m_field;
    const std::vector<float>& resFloatVec = std::get<1>(ProtocolTest.fields()).m_field;

    // Test protocol string
    GString protocolStr = GString(ProtocolTest);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProtocolTest::protocolReadWrite() {
    // Test protocol
    int testInt = 5;
    std::vector<float> testFloatVec = { 1.2f, 2.4f, 3.6f };

    double* doubleArray = new double[10];
    doubleArray[0] = 10;
    doubleArray[1] = 20;
    ProtocolField<double*> testRawArrayField(doubleArray, 10);

    Protocol<int, std::vector<float>, double*> ProtocolTest(testInt, testFloatVec, testRawArrayField);

    // Test protocol read write --------------------------------
    // Create and open file stream
    GString path("C:\\Users\\dante\\Documents\\Projects\\grand-blue-engine\\reverie\\assets\\output\\protocol_test.mdl");
    FileStream stream(path);
    stream.open(FileAccessMode::kWrite | FileAccessMode::kBinary);

    // Write to file
    ProtocolTest.write(stream);
    stream.close();

    // Read from file
    stream.open(FileAccessMode::kRead | FileAccessMode::kBinary);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}

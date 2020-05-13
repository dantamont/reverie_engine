#ifndef GB_TEST_BASE_H
#define GB_TEST_BASE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <QtTest/QtTest>
#include <iostream>
#include <QObject>
#include <iostream>
#include "../../grand_blue/src/core/GbObject.h"
#include "../../grand_blue/src/core/GbLogger.h"
#include "../../grand_blue/src/view/logging/GbConsoleTool.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TestBase : public QObject, public Gb::Object
{
    Q_OBJECT

public:

    TestBase(){
        // Create logger
        auto& logger = Gb::Logger::getInstance();
        logger.setLevel(Gb::LogLevel::Info);

        Gb::Logger::getConsoleTool("ConsoleTool");
        Gb::Logger::getVSLogger("ConsoleTool");
    }

    ~TestBase() {
        // Print line between tests
        std::cout << "\n";

        // Wait so that console messages may be written
        QTest::qWait(500);
    }

};

#endif
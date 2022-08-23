#pragma once 

#include <gtest/gtest.h>

#include "fortress/types/GString.h"
#include "fortress/types/GSizedTypes.h"
#include "fortress/system/memory/GPointerTypes.h"
#include "fortress/containers/graph/GDagNode.h"

namespace rev{

struct MyStruct: DagNode<MyStruct> {
    MyStruct(Uint32_t myInt, const GString& myString):
        m_testInt(myInt),
        m_testString(myString)
    {
    }

    virtual void onAddChild(const std::shared_ptr<MyStruct>& child) {}
    virtual void onAddParent(const std::shared_ptr<MyStruct>& parent) {}
    virtual void onRemoveChild(const std::shared_ptr<MyStruct>& child) {}
    virtual void onRemoveParent(const std::shared_ptr<MyStruct>& parent) {}

    Uint32_t m_testInt{0};
    GString m_testString;
};

TEST(DagNodeTest, Construction)
{
    // Create node
    static const char* testStr = "TestStr";
    std::shared_ptr<MyStruct> myDag = MyStruct::CreateDagNode<MyStruct>(1, testStr);
    std::vector<std::shared_ptr<MyStruct>>& dagNodes = MyStruct::DagNodes();

    // Test that construction worked
    EXPECT_EQ(myDag->m_testInt, 1);
    EXPECT_EQ(myDag->m_testString, testStr);

    // Delete node
    MyStruct::EraseFromNodeVec(myDag->id());
}

TEST(DagNodeTest, UuidValue)
{
    // Create node
    static const char* testStr = "TestStr";
    std::shared_ptr<MyStruct> myDag = MyStruct::CreateDagNode<MyStruct>(1, testStr);

    // Test that construction worked
    EXPECT_NE(myDag->id(), std::numeric_limits<Uint32_t>::max());

    // Delete node
    MyStruct::EraseFromNodeVec(myDag->id());
}

TEST(DagNodeTest, AddChild)
{
    static const char* testStr = "TestStr";
    std::shared_ptr<MyStruct> myDag = MyStruct::CreateDagNode<MyStruct>(1, testStr);

    for (int i = 0; i < 2; i++) {
        std::shared_ptr<MyStruct> child = MyStruct::CreateDagNode<MyStruct>(i, testStr);

        myDag->addChild(child);
        EXPECT_EQ(myDag->numChildren(), i + 1);
        for (int j = 0; j < 2; j++) {
            std::shared_ptr<MyStruct> grandChild = MyStruct::CreateDagNode<MyStruct>(j, testStr);
            child->addChild(grandChild);
            EXPECT_EQ(child->numChildren(), j + 1);
        }
    }
}

TEST(DagNodeTest, AddParent)
{
    static const char* testStr = "TestStr";
    std::shared_ptr<MyStruct> myDag = MyStruct::CreateDagNode<MyStruct>(1, testStr);

    for (int i = 0; i < 2; i++) {
        std::shared_ptr<MyStruct> parent = MyStruct::CreateDagNode<MyStruct>(i, testStr);
        myDag->addParent(parent);
        EXPECT_EQ(myDag->numParents(), i + 1);
    }
}


} /// End rev namespace

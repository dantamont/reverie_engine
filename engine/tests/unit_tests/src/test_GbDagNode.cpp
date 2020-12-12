///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "test_GbDagNode.h"
#include "../../grand_blue/src/core/containers/GbDagNode.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void TestDagNode::construction()
{
    //auto dagNode = Gb::DagNode::create();
}

void TestDagNode::uuidValue()
{
    //auto dagNode = Gb::DagNode::create();
    //QVERIFY(QString(dagNode->getUuid().asString()) != QUuid().toString());
    //QVERIFY(!dagNode->getUuid().isNull());
}

void TestDagNode::addChild()
{
    //auto dagNode = Gb::DagNode::create();

    //for (int i = 0; i < 2; i++) {
    //    auto child = Gb::DagNode::create();
    //    dagNode->addChild(child);
    //    QVERIFY2(dagNode->numChildren() == i + 1, "Add child failed");
    //    for (int j = 0; j < 2; j++) {
    //        auto childChild = Gb::DagNode::create();
    //        child->addChild(childChild);
    //        QVERIFY2(child->numChildren() == j + 1, "Add child's child failed");
    //    }
    //}

    //logInfo(dagNode->hierarchyDescription().c_str());
}

void TestDagNode::addParent() {
    //auto dagNode = Gb::DagNode::create();

    //for (int i = 0; i < 2; i++) {
    //    auto parent = Gb::DagNode::create();
    //    dagNode->addParent(parent);
    //    QVERIFY2(dagNode->numParents() == i + 1, "Add parent failed");
    //}
}

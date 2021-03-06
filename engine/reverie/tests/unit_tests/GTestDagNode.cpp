///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GTestDagNode.h"
#include <core/containers/GDagNode.h>

namespace rev{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void DagNodeTest::construction()
{
    //auto dagNode = rev::DagNode::create();
}

void DagNodeTest::uuidValue()
{
    //auto dagNode = rev::DagNode::create();
    //QVERIFY(QString(dagNode->getUuid().asString()) != QUuid().toString());
    //QVERIFY(!dagNode->getUuid().isNull());
}

void DagNodeTest::addChild()
{
    //auto dagNode = rev::DagNode::create();

    //for (int i = 0; i < 2; i++) {
    //    auto child = rev::DagNode::create();
    //    dagNode->addChild(child);
    //    QVERIFY2(dagNode->numChildren() == i + 1, "Add child failed");
    //    for (int j = 0; j < 2; j++) {
    //        auto childChild = rev::DagNode::create();
    //        child->addChild(childChild);
    //        QVERIFY2(child->numChildren() == j + 1, "Add child's child failed");
    //    }
    //}

    //logInfo(dagNode->hierarchyDescription().c_str());
}

void DagNodeTest::addParent() {
    //auto dagNode = rev::DagNode::create();

    //for (int i = 0; i < 2; i++) {
    //    auto parent = rev::DagNode::create();
    //    dagNode->addParent(parent);
    //    QVERIFY2(dagNode->numParents() == i + 1, "Add parent failed");
    //}
}

void DagNodeTest::perform()
{
    construction();
    uuidValue();
    addParent();
    addChild();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

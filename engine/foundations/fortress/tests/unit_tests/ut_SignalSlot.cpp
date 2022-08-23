#include <gtest/gtest.h>
#include "fortress/layer/framework/GSignalSlot.h"
#include "fortress/types/GString.h"

namespace rev {


class Button {
public:
    Signal<> on_click;
};

class Message {
public:
    void display() const {
        m_displayed = true;
        m_callCount++;
    }

    mutable bool m_displayed = false;
    mutable Uint32_t m_callCount{ 0 };
};

class Person {
public:
    Person() {}

    Signal<std::string const&> say;
    Signal<int> sayInt;

    void listen(std::string const& message) {
        m_listened = true;
        m_messageHeard = message;
    }

    void overLoadedFoo(const int input) {
        m_input = input;
    }

    void overLoadedFoo(const std::string& input) {
        m_inputStr =  input;
    }

    std::string m_messageHeard;
    bool m_listened{ false };
    int m_input{ 0 };
    std::string m_inputStr;
};

/// Tests
TEST(SignalSlot, Connect) {
    // create new signal
    Signal<std::string, int> signal;

    // attach a slot
    std::string myString;
    const std::string myExpectedString = "The answer: 42";
    signal.connect([&](std::string arg1, int arg2) {
        myString = arg1 + " " + std::to_string(arg2);
        });

    signal.emitForAll("The answer:", 42);

    EXPECT_EQ(myExpectedString, myString);
}

TEST(SignalSlot, Disconnect) {
    Button  button;
    Message message;

    Uint32_t id =button.on_click.connect(&message, &Message::display);
    button.on_click.emitForAll();
    button.on_click.emitForAll();

    EXPECT_EQ(message.m_displayed, true);
    EXPECT_EQ(message.m_callCount, 2);

    button.on_click.disconnect(id);

    button.on_click.emitForAll();

    EXPECT_EQ(message.m_callCount, 2);
}

TEST(SignalSlot, DisconnectAll) {
    Button  button;
    Message message, message2;

    // Connect signal and emitForAll twice
    Uint32_t id = button.on_click.connect(&message, &Message::display);
    Uint32_t id2 = button.on_click.connect(&message2, &Message::display);
    button.on_click.emitForAll();
    button.on_click.emitForAll();

    EXPECT_EQ(message.m_displayed, true);
    EXPECT_EQ(message.m_callCount, 2);
    EXPECT_EQ(message2.m_displayed, true);
    EXPECT_EQ(message2.m_callCount, 2);

    // Disconnect from one slot and ensure that only one disconnected
    button.on_click.disconnect(id);
    button.on_click.emitForAll();

    EXPECT_EQ(message.m_callCount, 2);
    EXPECT_EQ(message2.m_callCount, 3);

    // Disconnect remaining slot and confirm that nothing called
    button.on_click.disconnectAll();
    button.on_click.emitForAll();

    EXPECT_EQ(message.m_callCount, 2);
    EXPECT_EQ(message2.m_callCount, 3);
}

TEST(SignalSlot, ConnectMember) {
    Button  button;
    Message message;

    button.on_click.connect(&message, &Message::display);
    button.on_click.emitForAll();

    EXPECT_EQ(message.m_displayed, true);
}

TEST(SignalSlot, ConnectMemberWitArgs) {
    Person alice;
    Person bob;

    alice.say.connect(&bob, &Person::listen);
    bob.say.connect(&alice, &Person::listen);

    std::string aliceSaid = "Have a nice day!";
    alice.say.emitForAll(aliceSaid);

    EXPECT_EQ(bob.m_messageHeard, aliceSaid);
    EXPECT_EQ(bob.m_listened, true);
}

TEST(SignalSlot, Cast) {
    Person alice;

    alice.say.connect(
        &alice, 
        alice.say.Cast(&Person::overLoadedFoo));
    alice.sayInt.connect(
        &alice,
        alice.sayInt.Cast(&Person::overLoadedFoo));

    std::string aliceSaid = "Have a nice day!";
    alice.say.emitForAll(aliceSaid);
    alice.sayInt.emitForAll(12);
    alice.say.disconnectAll();
    alice.sayInt.disconnectAll();

    EXPECT_EQ(alice.m_inputStr, aliceSaid);
    EXPECT_EQ(alice.m_input, 12);

    alice.m_input = 0;
    alice.m_inputStr = "";
    alice.say.connect(&alice, &Person::overLoadedFoo);
    alice.sayInt.connect(&alice, &Person::overLoadedFoo);

    alice.say.emitForAll(aliceSaid);
    alice.sayInt.emitForAll(12);

    EXPECT_EQ(alice.m_inputStr, aliceSaid);
    EXPECT_EQ(alice.m_input, 12);
}


} /// End rev namespace
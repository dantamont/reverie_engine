#include <gtest/gtest.h>

#include "fortress/types/GString.h"

namespace rev {

TEST(GString, Join) {
    std::vector<GString> myStrings{"str0", "str1", "str2"};
    GString expectedStr("str0\nstr1\nstr2");
    GString outStr = GString::Join(myStrings, GString('\n'));
    EXPECT_EQ(outStr, expectedStr);
}

TEST(GString, HexToInt) {
    Uint64_t myInt = GString::HexToInt("0xa234");
    EXPECT_EQ(myInt, 0xa234);
}

TEST(GString, Strtok_r) {
    char* str1;
    char* savePtr;

    GString myString("this, is, a, delimited, string, to, tokenize");
    std::vector<GString> myStrings{ 
        GString("this"),
        GString("is"),
        GString("a"),
        GString("delimited"),
        GString("string"),
        GString("to"),
        GString("tokenize")
    };

    Uint32_t count = 0;
    char* outStr = GString::Strtok_r(myString.c_str(), ", ", savePtr);
    while (outStr) {
        EXPECT_EQ(GString(outStr), myStrings[count]);
        outStr = GString::Strtok_r(nullptr, ", ", savePtr);
        count++;
    }

}

TEST(GString, Format) {
    GString expected("This is a number: 120");
    GString myStr = GString::Format("%s %s %s %s: %d", "This", "is", "a", "number", 120);
    EXPECT_EQ(myStr, expected);
}

TEST(GString, FromNumber) {
    GString numberString = GString::FromNumber(987654321);
    EXPECT_EQ(numberString, "987654321");
}

TEST(GString, GetLength) {
    Int32_t size = GString::GetLength("%s %s %s %s: %d", "This", "is", "a", "number", 120);
    EXPECT_EQ(size, 21);
}

TEST(GString, Construction) {
    GString empty;
    EXPECT_EQ(empty.c_str(), nullptr);
    EXPECT_EQ(empty.length(), 0);
    EXPECT_EQ(empty.reservedSize(), 0);

    GString chr(',');
    EXPECT_NE(chr, ", ");
    EXPECT_EQ(chr, ",");
    EXPECT_EQ(chr, ',');

    GString filled(10, 'p');
    EXPECT_EQ(filled, "pppppppppp");

    GString fromStd(std::string("Ayee"));
    EXPECT_EQ(fromStd, "Ayee");

    GString fromChars("Ayee Again");
    EXPECT_EQ(fromChars, "Ayee Again");

    GString copied(fromChars);
    EXPECT_EQ(fromChars, copied);

    GString moved(std::move(GString("I am moving")));
    EXPECT_EQ(moved, "I am moving");
}

TEST(GString, Assignment) {
    GString myString;
    myString = "A string";
    EXPECT_EQ(myString, "A string");

    GString myString2 = "Some nonsense";
    myString2 = myString;
    myString2 = myString2;
    EXPECT_EQ(myString, myString2);
}

TEST(GString, EncodedConstruction) {
    /// @note In C++20, u8 strings have a dedicated char8_t type
    Char8_t* utf8Chars = reinterpret_cast<Char8_t*>(u8"\uf5ae\uf5b6\u0061\u0161\U00012000"); //13 bytes
    Char16_t* utf16Chars = u"\uf5ae\uf5b6\u0061\u0161\U00012000";
    Char32_t* utf32Chars = U"\uf5ae\uf5b6\u0061\u0161\U00012000";
    GString utf8(utf8Chars);
    GString utf82(utf8Chars, 13); // Less practical to use
    GString utf16(utf16Chars, 5); 
    GString utf32(utf32Chars, 5);

    EXPECT_EQ(utf8, utf8Chars);
    EXPECT_EQ(utf82, utf8Chars);
    EXPECT_EQ(utf16, utf16Chars);
    EXPECT_EQ(utf32, utf32Chars);
}

TEST(GString, Resize) {
    GString myStr("This is a string");
    myStr.resize(20);
    EXPECT_EQ(myStr.length(), 16);
    EXPECT_EQ(myStr.reservedSize(), 20);

    myStr[16] = ' ';
    myStr[17] = ' ';
    myStr[18] = ' ';
    myStr[19] = 0;
    EXPECT_EQ(myStr, "This is a string   ");
}

TEST(GString, SteamOperators) {
    GString myString("This is a test string");
    std::ostringstream myStream;
    myStream << myString;
    EXPECT_EQ(myString, myStream.str().c_str());
}

TEST(GString, Addition) {
    GString str1("str1");
    GString str2("str2");
    GString str3("str3");

    EXPECT_EQ(str1 + str2, "str1str2");
    EXPECT_EQ(str3 += str2, "str3str2");
    EXPECT_EQ(str3, "str3str2");

    GString addCharStr = str1 + '\\' + str2;
    EXPECT_EQ(addCharStr, "str1\\str2");
    str3 += "/";
    EXPECT_EQ(str3, "str3str2/");
}

TEST(GString, Equivalence) {
    const char* myChars = "These are my chars";
    const char myChar = 'C';
    GString myString(myChars);
    GString myCharG(myChar);
    GString myEmptyStr;

    EXPECT_EQ(myString, myChars);
    EXPECT_EQ(myChars, myString);
    EXPECT_EQ(myString, myString);
    EXPECT_NE(myString, myString + " ");
    EXPECT_EQ(myChar, myCharG);
    EXPECT_EQ(myCharG, myChar);
    EXPECT_NE(myEmptyStr, myString);
    EXPECT_NE(myString, myEmptyStr);
    EXPECT_EQ(myEmptyStr == myString, false);
    EXPECT_EQ(myString == myEmptyStr, false);
}

TEST(GString, SubStr) {
    GString myStr("AAAthis_is_my_test_string");
    EXPECT_EQ(myStr.subStr(3), "this_is_my_test_string");
    EXPECT_EQ(myStr.subStr(5, 5), "is_is");
}

TEST(GString, RegexSplit) {
    std::regex splitRegex("\\s*\\.\\s*");
    GString myStr("This . is .   a .  string .   with   . a  .    regex  .  split");

    std::vector<GString> myStrings = myStr.split(splitRegex);
    EXPECT_EQ(myStrings[0], "This");
    EXPECT_EQ(myStrings[1], "is");
    EXPECT_EQ(myStrings[2], "a");
    EXPECT_EQ(myStrings[3], "string");
    EXPECT_EQ(myStrings[4], "with");
    EXPECT_EQ(myStrings[5], "a");
    EXPECT_EQ(myStrings[6], "regex");
    EXPECT_EQ(myStrings[7], "split");
}

TEST(GString, RangeFor) {
    GString myStr("This is a test");
    Uint32_t i = 0;
    for (char& c : myStr) {
        c = 'A';
        i++;
    }
    EXPECT_EQ(GString("AAAAAAAAAAAAAA"), myStr);
    EXPECT_EQ(i, myStr.length());
}

TEST(GString, ConstRangeFor) {
    GString myStr("This is a test");
    std::string myStdStr("This is a test");
    Uint32_t i = 0;
    for (char& c : myStr) {
        EXPECT_EQ(myStdStr[i], c);
        i++;
    }
    EXPECT_EQ(i, myStr.length());
}

TEST(GString, Trim) {
    GString myStr("this_is_my_test_stringAAA");
    myStr.trim(3);
    EXPECT_EQ(myStr, "this_is_my_test_string");
}


} // End rev namespace

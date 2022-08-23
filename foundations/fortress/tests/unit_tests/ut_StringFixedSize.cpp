#include <gtest/gtest.h>

#include "fortress/types/GStringFixedSize.h"

namespace rev {

TEST(GStringFixedSize, Construction) {
    GStringFixedSize empty;
    EXPECT_EQ(empty.length(), 0);
    EXPECT_EQ(empty.reservedSize(), 255);

    GStringFixedSize chr(',');
    EXPECT_NE(chr, ", ");
    EXPECT_EQ(chr, ",");
    EXPECT_EQ(chr, ',');

    GStringFixedSize filled(10, 'p');
    EXPECT_EQ(filled, "pppppppppp");

    GStringFixedSize fromStd(std::string("Ayee"));
    EXPECT_EQ(fromStd, "Ayee");

    GStringFixedSize fromChars("Ayee Again");
    EXPECT_EQ(fromChars, "Ayee Again");

    GStringFixedSize copied(fromChars);
    EXPECT_EQ(fromChars, copied);

    GStringFixedSize moved(std::move(GStringFixedSize("I am moving")));
    EXPECT_EQ(moved, "I am moving");
}

TEST(GStringFixedSize, Assignment) {
    GStringFixedSize myString;
    myString = "A string";
    EXPECT_EQ(myString, "A string");

    GStringFixedSize myString2 = "Some nonsense";
    myString2 = myString;
    myString2 = myString2;
    EXPECT_EQ(myString, myString2);
}

TEST(GStringFixedSize, EncodedConstruction) {
    /// @note In C++20, u8 strings have a dedicated char8_t type
    Char8_t* utf8Chars = reinterpret_cast<Char8_t*>(u8"\uf5ae\uf5b6\u0061\u0161\U00012000"); //13 bytes
    Char16_t* utf16Chars = u"\uf5ae\uf5b6\u0061\u0161\U00012000";
    Char32_t* utf32Chars = U"\uf5ae\uf5b6\u0061\u0161\U00012000";
    GStringFixedSize utf8(utf8Chars);
    GStringFixedSize utf82(utf8Chars, 13); // Less practical to use
    GStringFixedSize utf16(utf16Chars, 5); 
    GStringFixedSize utf32(utf32Chars, 5);

    EXPECT_EQ(utf8, utf8Chars);
    EXPECT_EQ(utf82, utf8Chars);
    EXPECT_EQ(utf16, utf16Chars);
    EXPECT_EQ(utf32, utf32Chars);
}

TEST(GStringFixedSize, Size) {
    GStringFixedSize<20> myStr("This is a string");
    EXPECT_EQ(myStr.length(), 16);
    EXPECT_EQ(myStr.reservedSize(), 20);

    myStr[16] = ' ';
    myStr[17] = ' ';
    myStr[18] = ' ';
    myStr[19] = 0;
    EXPECT_EQ(myStr, "This is a string   ");
}

TEST(GStringFixedSize, SteamOperators) {
    GStringFixedSize myString("This is a test string");
    std::ostringstream myStream;
    myStream << myString;
    EXPECT_EQ(myString, myStream.str().c_str());
}

TEST(GStringFixedSize, Addition) {
    GStringFixedSize str1("str1");
    GStringFixedSize str2("str2");
    GStringFixedSize str3("str3");

    EXPECT_EQ(str1 + str2, "str1str2");
    EXPECT_EQ(str3 += str2, "str3str2");
    EXPECT_EQ(str3, "str3str2");

    GStringFixedSize addCharStr = str1 + '\\' + str2;
    EXPECT_EQ(addCharStr, "str1\\str2");
    str3 += "/";
    EXPECT_EQ(str3, "str3str2/");
}

TEST(GStringFixedSize, Equivalence) {
    const char* myChars = "These are my chars";
    const char myChar = 'C';
    GStringFixedSize myString(myChars);
    GStringFixedSize myCharG(myChar);
    GStringFixedSize myEmptyStr;

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

TEST(GStringFixedSize, SubStr) {
    // Substrings need room for the whole substring, plus the null terminator
    GStringFixedSize myStr("AAAthis_is_my_test_string");
    EXPECT_EQ(myStr.subStr<23>(3), "this_is_my_test_string");
    EXPECT_EQ(myStr.subStr<6>(5, 5), "is_is");
}

TEST(GStringFixedSize, RegexSplit) {
    std::regex splitRegex("\\s*\\.\\s*");
    GStringFixedSize myStr("This . is .   a .  string .   with   . a  .    regex  .  split");

    std::vector<GStringFixedSize<>> myStrings = myStr.split(splitRegex);
    EXPECT_EQ(myStrings[0], "This");
    EXPECT_EQ(myStrings[1], "is");
    EXPECT_EQ(myStrings[2], "a");
    EXPECT_EQ(myStrings[3], "string");
    EXPECT_EQ(myStrings[4], "with");
    EXPECT_EQ(myStrings[5], "a");
    EXPECT_EQ(myStrings[6], "regex");
    EXPECT_EQ(myStrings[7], "split");
}

TEST(GStringFixedSize, RangeFor) {
    GStringFixedSize myStr("This is a test");
    Uint32_t i = 0;
    for (char& c : myStr) {
        c = 'A';
        i++;
    }
    EXPECT_EQ(GStringFixedSize("AAAAAAAAAAAAAA"), myStr);
    EXPECT_EQ(i, myStr.length());
}

TEST(GStringFixedSize, ConstRangeFor) {
    GStringFixedSize myStr("This is a test");
    std::string myStdStr("This is a test");
    Uint32_t i = 0;
    for (char& c : myStr) {
        EXPECT_EQ(myStdStr[i], c);
        i++;
    }
    EXPECT_EQ(i, myStr.length());
}

TEST(GStringFixedSize, Trim) {
    GStringFixedSize myStr("this_is_my_test_stringAAA");
    myStr.trim(3);
    EXPECT_EQ(myStr, "this_is_my_test_string");
}


} // End rev namespace

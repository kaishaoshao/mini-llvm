#include <cstdint>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "mini-llvm/ir_reader/Lexer.h"
#include "mini-llvm/ir_reader/Token.h"

using namespace mini_llvm::ir;

using enum Token::Kind;

TEST(LexerTest, empty) {
    const char *input = "";

    std::vector<Token> expectedOutput{
        {kEOF, {}, input},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, comments) {
    const char *input = R"(
; Comment
@test = ; Comment
; Comment
; Comment
global i32 42 ; Comment
; Comment
)";

    std::vector<Token> expectedOutput{
        {kAt,     {},     input + 11},
        {kName,   "test", input + 12},
        {kEqual,  {},     input + 17},
        {kGlobal, {},     input + 49},
        {kI32,    {},     input + 56},
        {kNumber, 42,     input + 60},
        {kEOF,    {},     input + 83},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberInt64Max) {
    const char *input = "9223372036854775807";

    std::vector<Token> expectedOutput{
        {kNumber, std::numeric_limits<int64_t>::max(), input + 0},
        {kEOF, {}, input + 19},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberInt64Min) {
    const char *input = "-9223372036854775808";

    std::vector<Token> expectedOutput{
        {kNumber, std::numeric_limits<int64_t>::min(), input + 0},
        {kEOF, {}, input + 20},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberUInt64Max) {
    const char *input = "18446744073709551615";

    std::vector<Token> expectedOutput{
        {kNumber, static_cast<int64_t>(std::numeric_limits<uint64_t>::max()), input + 0},
        {kEOF, {}, input + 20},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberInt64MaxHex) {
    const char *input = "0x7fffffffffffffff";

    std::vector<Token> expectedOutput{
        {kNumber, std::numeric_limits<int64_t>::max(), input + 0},
        {kEOF, {}, input + 18},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberInt64MinHex) {
    const char *input = "0x8000000000000000";

    std::vector<Token> expectedOutput{
        {kNumber, std::numeric_limits<int64_t>::min(), input + 0},
        {kEOF, {}, input + 18},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, numberUInt64MaxHex) {
    const char *input = "0xffffffffffffffff";

    std::vector<Token> expectedOutput{
        {kNumber, static_cast<int64_t>(std::numeric_limits<uint64_t>::max()), input + 0},
        {kEOF, {}, input + 18},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, label) {
    const char *input = "_test.42:";

    std::vector<Token> expectedOutput{
        {kName,  "_test.42", input + 0},
        {kColon, {},         input + 8},
        {kEOF,   {},         input + 9},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, labelKeyword) {
    const char *input = "define:";

    std::vector<Token> expectedOutput{
        {kName, "define", input + 0},
        {kColon, {},      input + 6},
        {kEOF,   {},      input + 7},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, labelNumber) {
    const char *input = "42:";

    std::vector<Token> expectedOutput{
        {kName, "42", input + 0},
        {kColon, {},  input + 2},
        {kEOF,   {},  input + 3},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, localIdentifier) {
    const char *input = "%_test.42";

    std::vector<Token> expectedOutput{
        {kPercent, {},         input + 0},
        {kName,    "_test.42", input + 1},
        {kEOF,     {},         input + 9},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, localIdentifierKeyword) {
    const char *input = "@define";

    std::vector<Token> expectedOutput{
        {kAt,   {},       input + 0},
        {kName, "define", input + 1},
        {kEOF,  {},       input + 7},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, localIdentifierNumber) {
    const char *input = "%42";

    std::vector<Token> expectedOutput{
        {kPercent, {},   input + 0},
        {kName,    "42", input + 1},
        {kEOF,     {},   input + 3},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

TEST(LexerTest, string) {
    const char *input = R"(c"test\\42\0A\00")";

    std::vector<Token> expectedOutput{
        {kString, std::vector<int8_t>{0x74, 0x65, 0x73, 0x74, 0x5c, 0x34, 0x32, 0x0a, 0x00}, input + 0},
        {kEOF, {}, input + 17},
    };

    EXPECT_EQ(lex(input), expectedOutput);
}

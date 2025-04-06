#include <gtest/gtest.h>

#include "mini-llvm/ir_reader/IRReader.h"

using namespace mini_llvm::ir;

TEST(IRReaderTest, test00) {
    const char *input = R"(
define void @test() {
0:
    ret void
}
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test01) {
    const char *input = R"(
define internal void @test() {
0:
    ret void
}
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test02) {
    const char *input = "declare void @test()";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test03) {
    const char *input = "@test = global i32 42";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test04) {
    const char *input = "@test = internal global i32 42";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test05) {
    const char *input = "@test = external global i32";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test06) {
    const char *input = R"(
@test = global [2 x [3 x [4 x i32]]] [
    [3 x [4 x i32]] [
        [4 x i32] [i32 42, i32 43, i32 44, i32 45],
        [4 x i32] [i32 46, i32 47, i32 48, i32 49],
        [4 x i32] [i32 50, i32 51, i32 52, i32 53]
    ],
    [3 x [4 x i32]] [
        [4 x i32] [i32 54, i32 55, i32 56, i32 57],
        [4 x i32] [i32 58, i32 59, i32 60, i32 61],
        [4 x i32] [i32 62, i32 63, i32 64, i32 65]
    ]
]
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test07) {
    const char *input = R"(
define i32 @test1() {
0:
    br label %2

1:
    ret i32 %3

2:
    %3 = load i32, ptr @test2
    br label %1
}

@test2 = global ptr @test3
@test3 = global i32 42
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test08) {
    const char *input = R"(
define void @test() {
0:
    br label %1

1:
    br label %2

2:
    br label %1
}
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test09) {
    const char *input = R"(
define ptr @test1() {
0:
    ret ptr @test2
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test10) {
    const char *input = R"(
define ptr @test() {
0:
    ret ptr %1
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test11) {
    const char *input = R"(
define void @test() {
0:
    br label %1
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test12) {
    const char *input = R"(
@test = global i32 42

define void @test() {
0:
    ret void
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test13) {
    const char *input = R"(
define void @test(i32 %0) {
1:
    %2 = add i32 %0, %0
    %2 = sub i32 %0, %0
    ret void
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test14) {
    const char *input = R"(
define void @test() {
0:
    ret void

1:
    ret void

1:
    ret void
}
)";

    EXPECT_FALSE(parseModule(input));
}

TEST(IRReaderTest, test15) {
    const char *input = R"(
define void @test1() {
0:
    call void @test2(i32 42, i32 43, i32 44)
    ret void
}

declare void @test2(i32, i32, i32)
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test16) {
    const char *input = R"(
define void @test1() {
0:
    %1 = call i32 @test2(i32 42, i32 43, i32 44)
    ret void
}

declare i32 @test2(i32, i32, i32)
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test17) {
    const char *input = R"(
define void @test1() {
0:
    %1 = getelementptr [2 x [3 x i32]], ptr @test2, i32 0, i32 0, i32 0
    %2 = load i32, ptr %1
    ret void
}

@test2 = global [2 x [3 x i32]] [
    [3 x i32] [i32 42, i32 43, i32 44],
    [3 x i32] [i32 45, i32 46, i32 47]
]
)";

    EXPECT_TRUE(parseModule(input));
}

TEST(IRReaderTest, test18) {
    const char *input = R"(
define void @test(i1 %0) {
1:
    br i1 %0, label %2, label %3

2:
    br label %3

3:
    %4 = phi i32 [ 42, %1 ], [ 43, %2 ]
    ret void
}
)";

    EXPECT_TRUE(parseModule(input));
}

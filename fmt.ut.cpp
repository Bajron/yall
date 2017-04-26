#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "fmt.hpp"

struct FmtFormatShould : public ::testing::Test {
  void inHasOut(const char* str, int val) {
    EXPECT_EQ(placeholderCount(str), val);
  }
  void throwsFor(const char* str) {
    EXPECT_THROW(placeholderCount(str), std::logic_error);
  }
};

TEST_F(FmtFormatShould, ReturnZeroForEmptyString) {
  inHasOut("", 0);
}

TEST_F(FmtFormatShould, ReturnZeroForNonEmptyStringWithoutToken) {
  inHasOut("test", 0);
  inHasOut("{}}}{{", 0);
}

TEST_F(FmtFormatShould, ReturnOneForSimpleToken) {
  inHasOut("${1}", 1);
}

TEST_F(FmtFormatShould, ThrowIfTokenHasNoBrace) {
  throwsFor("$1");
}

TEST_F(FmtFormatShould, ThrowIfTokenHasNoClosingBrace) {
  throwsFor("${1");
  throwsFor("${1:");
  throwsFor("${1:xx");
}

TEST_F(FmtFormatShould, ThrowIfPlaceholderIsNotNumeral) {
  throwsFor("${x}");
}

TEST_F(FmtFormatShould, ReturnOneForSimpleTokenWithHint) {
  inHasOut("${1:hint}", 1);
}

TEST_F(FmtFormatShould, ReturnMaximalToken) {
  inHasOut("${1} ${1}", 1);
  inHasOut("${1} ${2}", 2);
  inHasOut("${2:x} ${1:x}", 2);
  inHasOut("${22}", 22);
}

TEST_F(FmtFormatShould, BeAConstExpression) {
  static_assert(placeholderCount("${1:x} ${2}") == 2,
                "placeholderCount is not constexpr or does not work");
}

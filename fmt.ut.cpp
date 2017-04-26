#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "fmt.hpp"

namespace {

struct YallDetailFmtFormatShould : public ::testing::Test {
  void inHasOut(const char* str, int val) {
    EXPECT_EQ(::yall::detail::placeholderCount(str), val);
  }
  void throwsFor(const char* str) {
    EXPECT_THROW(::yall::detail::placeholderCount(str), std::logic_error);
  }
};

TEST_F(YallDetailFmtFormatShould, ReturnZeroForEmptyString) {
  inHasOut("", 0);
}

TEST_F(YallDetailFmtFormatShould, ReturnZeroForNonEmptyStringWithoutToken) {
  inHasOut("test", 0);
  inHasOut("{}}}{{", 0);
}

TEST_F(YallDetailFmtFormatShould, ReturnOneForSimpleToken) {
  inHasOut("${1}", 1);
}

TEST_F(YallDetailFmtFormatShould, ThrowIfTokenHasNoBrace) {
  throwsFor("$1");
}

TEST_F(YallDetailFmtFormatShould, ThrowIfTokenHasNoClosingBrace) {
  throwsFor("${1");
  throwsFor("${1:");
  throwsFor("${1:xx");
}

TEST_F(YallDetailFmtFormatShould, ThrowIfPlaceholderIsNotNumeral) {
  throwsFor("${x}");
}

TEST_F(YallDetailFmtFormatShould, ReturnOneForSimpleTokenWithHint) {
  inHasOut("${1:hint}", 1);
}

TEST_F(YallDetailFmtFormatShould, ReturnMaximalToken) {
  inHasOut("${1} ${1}", 1);
  inHasOut("${1} ${2}", 2);
  inHasOut("${2:x} ${1:x}", 2);
  inHasOut("${22}", 22);
}

TEST_F(YallDetailFmtFormatShould, BeAConstExpression) {
  static_assert(::yall::detail::placeholderCount("${1:x} ${2}") == 2,
                "placeholderCount is not constexpr or does not work");
}

}

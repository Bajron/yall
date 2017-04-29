#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "yall/mocks.hpp"
#include "yall/fmt.hpp"

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


struct YallFmtEvaluatingBackendShould: public ::testing::Test {
  YallFmtEvaluatingBackendShould():
    decoratedMock(std::make_shared<MockLoggerBackend>()),
    uut(decoratedMock) {
  }
  std::shared_ptr<MockLoggerBackend> decoratedMock;
  ::yall::FmtEvaluatingBackend uut;
};

TEST_F(YallFmtEvaluatingBackendShould, ForwardToDecorated) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"test", "test"});
  EXPECT_CALL(*decoratedMock, take(msg)).Times(1);
  uut.take(::yall::LoggerMessage(msg));
}

TEST_F(YallFmtEvaluatingBackendShould, EvaluateFormatting) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"yall::Fmt", "x${1}x"});
  msg.sequence.emplace_back(yall::TypeAndValue{"test", "test"});

  ::yall::LoggerMessage formatted;
  formatted.sequence.emplace_back(yall::TypeAndValue{"yall::Formatted", "xtestx"});

  EXPECT_CALL(*decoratedMock, take(formatted)).Times(1);
  uut.take(::yall::LoggerMessage(msg));
}


}

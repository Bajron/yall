#include <gtest/gtest.h>

#include "yall/backends.hpp"
#include "yall/mocks.hpp"
#include "yall/prefix.hpp"

namespace {

  struct YallPrefixDecoratingBackendShould: public ::testing::Test {
    YallPrefixDecoratingBackendShould():
    decoratedMock(std::make_shared<MockLoggerBackend>()),
    uut(decoratedMock, "testPrefix") {
    }
    std::shared_ptr<MockLoggerBackend> decoratedMock;
    ::yall::PrefixDecoratingBackend uut;
  };

  TEST_F(YallPrefixDecoratingBackendShould, ForwardWithPrefixToDecorated) {
    ::yall::LoggerMessage msg;
    EXPECT_CALL(*decoratedMock, take(::testing::_))
      .Times(1).WillOnce(::testing::SaveArg<0>(&msg));

    ::yall::LoggerMessage input;
    input.sequence.push_back(::yall::TypeAndValue{"test", "test value"});

    uut.take(::yall::LoggerMessage(input));

    EXPECT_EQ(1, msg.sequence.size());
    EXPECT_EQ("test value", msg.sequence[0].value);
    EXPECT_THAT(msg.meta["yall::Prefix"], "testPrefix");
  }

  TEST_F(YallPrefixDecoratingBackendShould, ProvideChildWithExtendedPrefix) {
    ::yall::LoggerMessage msg;
    EXPECT_CALL(*decoratedMock, take(::testing::_))
      .Times(1).WillOnce(::testing::SaveArg<0>(&msg));

    ::yall::LoggerMessage input;
    input.sequence.push_back(::yall::TypeAndValue{"test", "test value"});

    uut.getChild("ch")->take(::yall::LoggerMessage(input));

    EXPECT_EQ(1, msg.sequence.size());
    EXPECT_EQ("test value", msg.sequence[0].value);
    EXPECT_THAT(msg.meta["yall::Prefix"], "testPrefix.ch");
  }

  struct YallPrefixedLoggerShould: public ::testing::Test {
    YallPrefixedLoggerShould():
      decoratedMock(std::make_shared<MockLoggerBackend>()),
      uut(decoratedMock) {
    }
    std::shared_ptr<MockLoggerBackend> decoratedMock;
    ::yall::PrefixedLogger uut;
  };

  TEST_F(YallPrefixedLoggerShould, ForwardWithPrefixToDecorated) {
    ::yall::LoggerMessage msg;
    EXPECT_CALL(*decoratedMock, take(::testing::_))
      .Times(1).WillOnce(::testing::SaveArg<0>(&msg));

    uut.log("test");

    EXPECT_EQ(1, msg.sequence.size());
    EXPECT_EQ("test", msg.sequence[0].value);
    EXPECT_THAT(msg.meta["yall::Prefix"], "root");
  }

  TEST_F(YallPrefixedLoggerShould, AllowToGetChild) {
    ::yall::LoggerMessage msg;
    EXPECT_CALL(*decoratedMock, take(::testing::_))
      .Times(1).WillOnce(::testing::SaveArg<0>(&msg));

    uut.child("child").log("test");

    EXPECT_EQ(1, msg.sequence.size());
    EXPECT_EQ("test", msg.sequence[0].value);
    EXPECT_THAT(msg.meta["yall::Prefix"], "root.child");
  }


}


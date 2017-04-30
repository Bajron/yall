#include <gtest/gtest.h>

#include "yall/backends.hpp"
#include "yall/mocks.hpp"

namespace {

struct YallNullBackendShould: public ::testing::Test {
  YallNullBackendShould():
    uut() {
  }
  ::yall::NullBackend uut;
};

TEST_F(YallNullBackendShould, BeABackend) {
  uut.take(::yall::LoggerMessage{});
}

struct YallMetaFormattingBackendShould: public ::testing::Test {
  YallMetaFormattingBackendShould():
    decoratedMock(std::make_shared<MockLoggerBackend>()),
    uut(decoratedMock) {
  }
  std::shared_ptr<MockLoggerBackend> decoratedMock;
  ::yall::MetaFormattingBackend uut;
};

TEST_F(YallMetaFormattingBackendShould, ForwardFormattedToDecorated) {
  ::yall::LoggerMessage msg;
  EXPECT_CALL(*decoratedMock, take(::testing::_))
    .Times(1).WillOnce(::testing::SaveArg<0>(&msg));;

  ::yall::LoggerMessage input;
  input.meta["yall::TimeStamp"] = "<time>";
  input.meta["yall::ThreadId"] = "<tid>";
  input.meta["yall::Priority"] = "<priority>";
  input.sequence.push_back(::yall::TypeAndValue{"test", "test value"});

  uut.take(::yall::LoggerMessage(input));

  EXPECT_EQ(2, msg.sequence.size());
  EXPECT_EQ("test value", msg.sequence[1].value);
  EXPECT_EQ("yall::Formatted", msg.sequence[0].type);
  EXPECT_THAT(msg.sequence[0].value, ::testing::ContainsRegex("<time>"));
  EXPECT_THAT(msg.sequence[0].value, ::testing::ContainsRegex("<tid>"));
  EXPECT_THAT(msg.sequence[0].value, ::testing::ContainsRegex("<priority>"));
}

struct YallStreamBackendShould: public ::testing::Test {
  YallStreamBackendShould():
    testStream(std::make_shared<std::stringstream>()),
    uut(testStream) {
  }
  std::shared_ptr<std::stringstream> testStream;
  ::yall::StreamBackend uut;
};

TEST_F(YallStreamBackendShould, ReceiveSequenceOnStream) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"type", "value"});
  uut.take(::yall::LoggerMessage(msg));

  std::string output = testStream->str();
  EXPECT_EQ(output, "value\n");
}

TEST_F(YallStreamBackendShould, ReceiveSequenceOnStream2) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"type", "hello"});
  msg.sequence.emplace_back(yall::TypeAndValue{"type", " "});
  msg.sequence.emplace_back(yall::TypeAndValue{"type", "world"});
  msg.sequence.emplace_back(yall::TypeAndValue{"type", "!"});

  uut.take(::yall::LoggerMessage(msg));

  std::string output = testStream->str();
  EXPECT_EQ(output, "hello world!\n");
}


struct YallFanOutBackendShould: public ::testing::Test {
  YallFanOutBackendShould():
    decoratedMock1(std::make_shared<MockLoggerBackend>()),
    decoratedMock2(std::make_shared<MockLoggerBackend>()),
    uut({decoratedMock1, decoratedMock2}) {
  }
  std::shared_ptr<MockLoggerBackend> decoratedMock1;
  std::shared_ptr<MockLoggerBackend> decoratedMock2;

  ::yall::FanOutBackend uut;
};

TEST_F(YallFanOutBackendShould, ForwardToAllDecorated) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"test", "test value"});

  EXPECT_CALL(*decoratedMock1, take(msg)).Times(1);
  EXPECT_CALL(*decoratedMock2, take(msg)).Times(1);

  uut.take(::yall::LoggerMessage(msg));
}

TEST_F(YallFanOutBackendShould, ForwardToAllAdded) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"test", "test value"});

  ::yall::FanOutBackend localUut;
  localUut.add(decoratedMock1);
  localUut.add(decoratedMock2);

  EXPECT_CALL(*decoratedMock1, take(msg)).Times(1);
  EXPECT_CALL(*decoratedMock2, take(msg)).Times(1);

  localUut.take(::yall::LoggerMessage(msg));
}


}

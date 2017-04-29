#include <gtest/gtest.h>

#include "yall/logger.hpp"
#include "yall/mocks.hpp"

namespace {

struct YallLoggerShould: public ::testing::Test {
  YallLoggerShould():
    backendMock(std::make_shared<MockLoggerBackend>()),
    uut(backendMock) {
  }
  std::shared_ptr<MockLoggerBackend> backendMock;
  ::yall::Logger uut;

  ::yall::LoggerMessage msg;

  void SetUp() {
    EXPECT_CALL(*backendMock, take(::testing::_))
      .Times(1).WillOnce(::testing::SaveArg<0>(&msg));
  }

  void verifyCall2() {
    EXPECT_EQ(2, msg.sequence.size());
    EXPECT_EQ(msg.sequence[0].value, "test");
    EXPECT_EQ(msg.sequence[1].value, "1");
  }
};

TEST_F(YallLoggerShould, ForwardToBackend) {
  uut.log("test");

  EXPECT_EQ(1, msg.sequence.size());
  EXPECT_EQ(msg.sequence[0].value, "test");
}

TEST_F(YallLoggerShould, ForwardToBackend2) {
  uut.log("test", 1);
  verifyCall2();
}

TEST_F(YallLoggerShould, ForwardToBackend2WithOperator) {
  uut("test", 1);
  verifyCall2();
}

TEST_F(YallLoggerShould, ForwardToBackend2WithStream) {
  uut("test", 1);
  verifyCall2();
}

TEST_F(YallLoggerShould, ProvideMetaData) {
  uut("test");
  EXPECT_EQ(1, msg.meta.count("yall::TimeStamp"));
  EXPECT_EQ(1, msg.meta.count("yall::ThreadId"));
}

TEST_F(YallLoggerShould, HandleNumbers) {
  uut(short(1), int(1), long(1), (long long)(1),
    (unsigned short)(1), (unsigned int)1, (unsigned long)1, (unsigned long long)1,
    float(1), double(1));
  EXPECT_EQ(10, msg.sequence.size());
}


}

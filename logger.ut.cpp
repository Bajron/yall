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
};

TEST_F(YallLoggerShould, ForwardToBackend) {
  ::yall::LoggerMessage msg;

  EXPECT_CALL(*backendMock, take(::testing::_))
    .Times(1).WillOnce(::testing::SaveArg<0>(&msg));

  uut.log("test");

  EXPECT_EQ(1, msg.sequence.size());
  EXPECT_EQ(msg.sequence[0].value, "test");
}

}

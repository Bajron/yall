#include <gtest/gtest.h>

#include "yall/priority.hpp"
#include "yall/mocks.hpp"

namespace {

struct YallPriorityDecoratingBackendShould: public ::testing::Test {
  YallPriorityDecoratingBackendShould():
    decoratedMock(std::make_shared<MockLoggerBackend>()),
    uut(decoratedMock, yall::Priority::Info) {
  }
  std::shared_ptr<MockLoggerBackend> decoratedMock;
  ::yall::PriorityDecoratingBackend uut;
};

TEST_F(YallPriorityDecoratingBackendShould, ForwardToDecoratedWithNewPriority) {
  ::yall::LoggerMessage msg;
  msg.sequence.emplace_back(yall::TypeAndValue{"test", "test"});

  auto expected = msg;
  expected.meta["yall::Priority"] = "info";

  EXPECT_CALL(*decoratedMock, take(expected)).Times(1);
  uut.take(::yall::LoggerMessage(msg));
}

}

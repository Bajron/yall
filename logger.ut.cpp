#include <gtest/gtest.h>

#include "yall/logger.hpp"
#include "yall/mocks.hpp"
#include "yall/backends.hpp"
#include "yall/fmt.hpp"

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
  void verifyOneXCall() {
    EXPECT_EQ(1, msg.sequence.size());
    EXPECT_EQ("x", msg.sequence[0].value);
  }
  void verifyFmtTest() {
    EXPECT_EQ(2, msg.sequence.size());
    EXPECT_EQ("yall::Fmt", msg.sequence[0].type);
    EXPECT_EQ("${1}", msg.sequence[0].value);
    EXPECT_EQ("test", msg.sequence[1].value);
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

TEST_F(YallLoggerShould, UseTheSameBackendAfterCopy) {
  auto copy = uut;
  copy.log('x');
  verifyOneXCall();
}

TEST_F(YallLoggerShould, UseTheSameBackendAfterAssign) {
  ::yall::Logger copy = ::yall::Logger(std::make_shared<yall::NullBackend>());
  copy = uut;
  copy.log('x');
  verifyOneXCall();
}

TEST_F(YallLoggerShould, UseTheSameBackendAfterSelfAssign) {
  uut = uut;
  uut.log('x');
  verifyOneXCall();
}

TEST_F(YallLoggerShould, AcceptFmtAsFirstArgument) {
  uut.log(MakeFmt("${1}"), "test");
  verifyFmtTest();
}

TEST_F(YallLoggerShould, AcceptFmtAsFirstArgumentLValue) {
  auto fmt = MakeFmt("${1}");
  uut.log(fmt, "test");
  verifyFmtTest();
}

TEST_F(YallLoggerShould, HandleVariousStrings) {
  std::string str = "test";
  const char* cch = "test";
  char* ch = (char*)cch;

  uut.log(str, cch, ch, "test", str.c_str());
  EXPECT_EQ(5, msg.sequence.size());
  for (int i=0; i < 5; ++i) {
    EXPECT_EQ("test", msg.sequence[i].value);
  }
}

}

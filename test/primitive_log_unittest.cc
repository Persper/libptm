//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#include "gtest/gtest.h"

#include "log/primitive_log.h"

#include <stdlib.h>
#include <string.h>
#include <queue>

namespace persper {

class PrimitiveLogTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    log_ = (PrimitiveLog *)malloc(size_);
    memset(log_, 0, size_);
  }

  virtual void TearDown() { free(log_); }

  PrimitiveLog *log_;
  const size_t size_ = 1000;
};

TEST_F(PrimitiveLogTest, Init) {
  ASSERT_EQ(log_->Init(size_), 0);
  ASSERT_EQ((char *)log_->Head(), (char *)log_ + sizeof(PrimitiveLog));
  memset(log_, 1, size_);
  ASSERT_NE(log_->Init(size_), 0);
}

TEST_F(PrimitiveLogTest, AppendTruncate) {
  ASSERT_EQ(log_->Init(size_), 0);
  for (int i = 0; i < 9; ++i) {
    ASSERT_NE(log_->Append(100), nullptr) << "i = " << i;
  }
  ASSERT_EQ(log_->Append(100), nullptr);
  for (int i = 0; i < 9; ++i) {
    ASSERT_NE(log_->Head()->size, 0) << "i = " << i;
    ASSERT_NE(log_->Truncate(), nullptr) << "i = " << i;
  }
  ASSERT_EQ(log_->Head()->size, 0);
  ASSERT_EQ(log_->Truncate(), nullptr);
}

TEST_F(PrimitiveLogTest, Comprehensive) {
  ASSERT_EQ(log_->Init(size_), 0);
  std::queue<size_t> entries;
  for (int i = 0; i < 10000; ++i) {
    size_t s = rand() % (size_ / 2);
    LogEntry *entry = log_->Append(s);
    if (!entry) {
      while (!entries.empty()) {
        ASSERT_EQ(log_->Head()->size, entries.front()) << "i = " << i;
        ASSERT_NE(log_->Truncate(), nullptr) << "i = " << i;
        entries.pop();
      }
      log_->Rewind();
      ASSERT_NE(entry = log_->Append(s), nullptr) << "i = " << i;
      ASSERT_EQ(log_->Extend(entry, 100), s += 100) << "i = " << i;
    }
    entries.push(s);
  }
}

} // namespace persper

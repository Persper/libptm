//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#include "gtest/gtest.h"

#include "log/sync_pers_log.h"

#include <cstdlib>
#include <cstring>
#include <queue>

namespace persper {

struct SamplePcommit : public PersCommit {
  int operator()(void *data, size_t size) {
    return 0;
  }
};

class SyncPersLogTest : public ::testing::Test {
 protected:
  SyncPersLogTest() {
    void *cache = malloc(size_);
    memset(cache, 0, size_);
    log_ = new SyncPersLog<SamplePcommit>(cache, size_);
  }
  virtual ~SyncPersLogTest() {
    free(log_->head());
    delete log_;
  }

  SyncPersLog<SamplePcommit> *log_;
  const size_t size_ = 1000;
};

struct MetaData {
  uint64_t address;
  size_t size;

  bool operator==(const MetaData &other) const {
    return address == other.address && size == other.size;
  }
};

TEST_F(SyncPersLogTest, MetaOnly) {
  std::queue<MetaData> check; // records for check purpose
  for (int i = 0; i < 100000; ++i) {
    MetaData meta = { (uint64_t)i, (size_t)rand() };
    if (log_->Append(&meta, nullptr, 0) == ENOSPC) {
      for (MetaData *cur = (MetaData *)log_->head()->data();
          cur != (MetaData *)log_->end(); ++cur) {
        ASSERT_EQ(check.front(), *cur);
        check.pop();
      }
      ASSERT_EQ(0, log_->Commit());
      ASSERT_TRUE(check.empty());
      ASSERT_EQ(0, log_->Append(&meta, nullptr, 0));
    }
    check.push(meta);
  }
}

TEST_F(SyncPersLogTest, MetaAndData) {
  std::queue<MetaData> check; // records for check purpose
  for (int i = 0; i < 10000; ++i) {
    MetaData meta = { (uint64_t)i, (size_t)rand() % (size_ / 3) + 1 };
    void *data = malloc(meta.size); // data to log beside metadata
    if (log_->Append(&meta, data, meta.size) == ENOSPC) {
      for (MetaData *cur = (MetaData *)log_->head()->data();
          (char *)log_->end() - (char *)cur > sizeof(MetaData); ) {
        ASSERT_EQ(check.front(), *cur);
        cur = (MetaData *)((char *)cur + sizeof(MetaData) + cur->size);
        check.pop();
      }
      ASSERT_EQ(0, log_->Commit());
      ASSERT_TRUE(check.empty());
      ASSERT_EQ(0, log_->Append(&meta, data, meta.size));
    }
    check.push(meta);
    free(data);
  }
}

} // namespace persper

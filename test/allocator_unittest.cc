//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#include "gtest/gtest.h"

#include "allocator.h"

#include <vector>

namespace persper {

TEST(StandardAllocatorTest, StlVector) {
  const int init_size = 3;
  const int init_value = 10;
  std::vector<int, StandardAllocator<int>> v(init_size, init_value);
  int c = v.capacity();
  v.resize(2 * c, 0);
  for (int i = 0; i < 2 * c; ++i) {
    if (i < init_size) {
      ASSERT_EQ(v[i], init_value);
    } else {
      ASSERT_TRUE(!v[i]);
    }
  }
}

} // namespace persper

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

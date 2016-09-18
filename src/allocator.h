//
// The Persper License
//
// Copyright (c) 2016 Persper Technologies Co., Ltd.
//

#ifndef PERSPER_LIBPM_ALLOCATOR_H_
#define PERSPER_LIBPM_ALLOCATOR_H_

#include <cstddef>

namespace persper {

template <typename T>
struct StandardAllocator {
  typedef T value_type;

  StandardAllocator() { }

  template <typename U> StandardAllocator(const StandardAllocator<U>& other) { }

  T* allocate(std::size_t n) {
    return static_cast<T *>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, std::size_t n) { ::operator delete(p); }
};

template <typename T, typename U>
bool operator==(const StandardAllocator<T>&, const StandardAllocator<U>&) {
  return true;
}

template <typename T, typename U>
bool operator!=(const StandardAllocator<T>&, const StandardAllocator<U>&) {
  return false;
}

} // namespace persper

#endif // PERSPER_LIBPM_ALLOCATOR_H_

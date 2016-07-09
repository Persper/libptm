/*
 * The Persper License
 *
 * Copyright (c) 2016 Persper Technologies Co., Ltd.
*/

#include "naming.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

static void * const kPMAddr = (void *)0x6e256e200000;

//
// Utility functions for manipulating file descriptors
//
static void check_error_or_exit(uint64_t ret, const char *message = nullptr);
static size_t file_size(int fd);

// Records the persistent memory region size for unmapping
static size_t g_pm_size = 0;

void pm_open(const char *file, size_t size) {
  int fd = -1;
  if (size == (size_t)-1) { // mapping to a whole existing file
    fd = open(file, O_RDWR | O_NOATIME);
    check_error_or_exit(fd);
    size = file_size(fd);
  } else { // mapping to a (created) file with the specified size
    fd = open(file, O_RDWR | O_NOATIME | O_CREAT, S_IRWXU);
    check_error_or_exit(fd);
    if (file_size(fd) < size) { // in case the file is too small
      check_error_or_exit(ftruncate(fd, size));
    }
  }
  void *addr = mmap(kPMAddr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, fd, 0);
  check_error_or_exit((uint64_t)addr);
  g_pm_size = size;
}

int pm_close() {
  int ret = munmap(kPMAddr, g_pm_size);
  if (ret == -1) std::perror("munmap()");
  return ret;
}

void *pm_retrieve(const char *id) {
  return nullptr;
}

void pm_register(const char *id, void *addr) {

}

void pm_deregister(const char *id) {

}

//
// Implementation of utility functions
//
static void check_error_or_exit(uint64_t ret, const char *message) {
  if (ret == (uint64_t)-1) {
    std::perror(message);
    std::exit(EXIT_FAILURE);
  }
}

static size_t file_size(int fd) {
  off_t size = lseek(fd, 0, SEEK_END);
  check_error_or_exit(size);
  return size;
}

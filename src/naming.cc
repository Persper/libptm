/*
 * The Persper License
 *
 * Copyright (c) 2016 Persper Technologies Co., Ltd.
*/

#include "naming.h"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils/MurmurHash3.h"

static void * const kPMAddr = (void *)0x6e256e200000;

/**
 * @brief Max length of a string seed ID.
*/
#define SEED_MAX_LEN (240)

/**
 * @brief A pair of seed ID and memory address.
 *
 * Seed is linked in a hashtable for retrieval.
 * The size of Seed is statically set to 256 bytes.
 * @see SEED_MAX_LEN
*/
struct Seed {
  char id[SEED_MAX_LEN];
  void *address;
  Seed *next;
};

#define TABLE_SHIFT (4)
#define TABLE_SIZE (1U << TABLE_SHIFT)
#define TABLE_MASK (TABLE_SIZE - 1)

/**
 * @brief A hashtable to index seed IDs.
 *
 * Consists of a fixed number of buckets. It is not recommended to have a large
 * number of seed IDs, so a static hashtable should be able to handle them
 * efficiently. The number of buckets is defined by #TABLE_SIZE.
*/
struct SeedTable {
  Seed *buckets[TABLE_SIZE];
};

static SeedTable * const kSeedTableAddr = (SeedTable *)kPMAddr;

//
// Utility functions for manipulating file descriptors
//
static void check_error_or_exit(uint64_t ret, const char *message = nullptr);
static size_t file_size(int fd);

// Records the persistent memory region size for unmapping
static size_t g_pm_size = 0;

void pm_open(const char *file, size_t size) {
  int fd = -1;
  bool to_init = false;
  if (size == (size_t)-1) { // mapping to a whole existing file
    fd = open(file, O_RDWR | O_NOATIME);
    check_error_or_exit(fd);
    g_pm_size = file_size(fd);
  } else { // mapping to a (created) file with the specified size
    fd = open(file, O_RDWR | O_NOATIME | O_CREAT, S_IRWXU);
    check_error_or_exit(fd);
    g_pm_size = file_size(fd);
    if (!g_pm_size) { // in case the file is empty
      check_error_or_exit(ftruncate(fd, size));
      g_pm_size = size;
      to_init = true;
    }
    assert(g_pm_size == size);
  }
  void *addr = mmap(kPMAddr, g_pm_size, PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_SHARED | MAP_FIXED, fd, 0);
  check_error_or_exit((uint64_t)addr);
  assert(addr == kPMAddr);
  if (to_init) {
    // Initializes the persistent memory region
    std::memset(kSeedTableAddr, 0, sizeof(SeedTable));
  }
}

int pm_close() {
  int ret = munmap(kPMAddr, g_pm_size);
  if (ret == -1) std::perror("munmap()");
  return ret;
}

void *pm_retrieve(const char *id) {
  const int len = strnlen(id, SEED_MAX_LEN);
  if (len == SEED_MAX_LEN) {
    return nullptr;
  }
  uint32_t hash = 0;
  MurmurHash3_x86_32(id, len, 0x6e256e2, &hash);

  Seed *cur = kSeedTableAddr->buckets[hash & TABLE_MASK];
  while (cur) {
    if (std::strcmp(cur->id, id) == 0) return cur->address;
    cur = cur->next;
  }
  return nullptr;
}

void *pm_register(const char *id, void *addr) {
  const int len = strnlen(id, SEED_MAX_LEN);
  if (len == SEED_MAX_LEN) {
    return nullptr;
  }
  uint32_t hash = 0;
  MurmurHash3_x86_32(id, len, 0x6e256e2, &hash);

  Seed *cur = kSeedTableAddr->buckets[hash & TABLE_MASK];
  while (cur) {
    if (std::strcmp(cur->id, id) == 0) { // overwriting the old address
      void *old_addr = cur->address;
      cur->address = addr;
      return old_addr;
    }
    cur = cur->next;
  }

  Seed *seed = (Seed *)malloc(sizeof(Seed));
  if (seed) {
    std::strcpy(seed->id, id);
    seed->address = addr;
    seed->next = kSeedTableAddr->buckets[hash & TABLE_MASK];
    kSeedTableAddr->buckets[hash & TABLE_MASK] = seed;
  }
  return seed->address;
}

void *pm_deregister(const char *id) {
  const int len = strnlen(id, SEED_MAX_LEN);
  if (len == SEED_MAX_LEN) {
    return nullptr;
  }
  uint32_t hash = 0;
  MurmurHash3_x86_32(id, len, 0x6e256e2, &hash);

  Seed **pprev = &kSeedTableAddr->buckets[hash & TABLE_MASK];
  Seed *cur = *pprev;
  while (cur) {
    if (std::strcmp(cur->id, id) == 0) {
      *pprev = cur->next;
      void *addr = cur->address;
      free(cur);
      return addr;
    }
    pprev = &cur->next;
    cur = cur->next;
  }
  return nullptr;
}


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

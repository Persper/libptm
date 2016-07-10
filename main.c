#include <assert.h>
#include <stdio.h>
#include "naming.h"

int main() {
  pm_open("main.pmd", 1000000000);
  if (pm_close() != 0) {
    fprintf(stderr, "Fail to save persistent data!\n");
    return 1;
  }

  pm_open("main.pmd", -1);
  
  void *addr = pm_retrieve("Seed");
  assert(!addr);

  addr = pm_register("Seed", (void *)0xcafe);
  assert(addr == (void *)0xcafe);
  printf("Registered 'Seed' with %p\n", addr);

  addr = pm_register("Seed", (void *)0xc0ffee);
  assert(addr == (void *)0xcafe);
  addr = pm_retrieve("Seed");
  assert(addr == (void *)0xc0ffee);
  printf("Registered 'Seed' with %p\n", addr);

  addr = pm_deregister("Seed");
  assert(addr == (void *)0xc0ffee);

  addr = pm_retrieve("Seed");
  assert(!addr);

  if (pm_close() != 0) {
    fprintf(stderr, "Fail to save persistent data!\n");
    return 1;
  }
  return 0;
}

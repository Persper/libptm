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
  if (!addr) {
    pm_register("Seed", (void *)0xcafe);
    printf("Do registration.\n");
  } else {
    void *addr = pm_retrieve("Seed");
    printf("%p\n", addr);
    pm_deregister("Seed");
  }

  if (pm_close() != 0) {
    fprintf(stderr, "Fail to save persistent data!\n");
    return 1;
  }
  return 0;
}

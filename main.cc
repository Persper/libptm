#include <iostream>
#include "naming.h"

int main() {
  pm_open("HelloWorld.pmd");
  void *addr = pm_retrieve("Seed");
  if (!addr) {
    pm_register("Seed", (void *)0xcafe);
    std::cout << "Do registration." << std::endl;
  } else {
    void *addr = pm_retrieve("Seed");
    std::cout << addr << std::endl;
    pm_deregister("Seed");
  }
  if (pm_close() != 0) {
    std::cerr << "Fail to save persistent data!" << std::endl;
    return 1;
  }
  return 0;
}

#include <iostream>
#include "naming.h"

int main() {
  pm_open("main.pmd", 1000000000);
  if (pm_close() != 0) {
    std::cerr << "Fail to save persistent data!" << std::endl;
    return 1;
  }

  pm_open("main.pmd", -1);

  if (pm_close() != 0) {
    std::cerr << "Fail to save persistent data!" << std::endl;
    return 1;
  }
  return 0;
}

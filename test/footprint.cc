#include <cstring>
#include <iostream>

using namespace std;

long long test_store(long long *data) {
  long long temp;
  __transaction_atomic {
    temp = data[12];
    data[12] = -1;
  }
  __transaction_atomic {
    data[12] = -1;
    temp = data[12];
  }
  __transaction_atomic {
    data[13] = -1;
  }
  return temp;
}

void test_bulk(char *data) {
  __transaction_atomic {
    memset(data, 'R', 100);
  }
  __transaction_atomic {
    memmove(data + 100, data + 90, 20);
  }
  for (int i = 90; i < 120; ++i) {
    cout << (data[i] == -1 ? '.' : data[i]);
  }
  cout << endl;
}

int main() {
  char *data = new char[128];
  test_store((long long *)data);
  test_bulk(data);
}


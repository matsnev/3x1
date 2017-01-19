#include <iostream>

#include "var_uint.h"

uint64_t ComputeHeight(VariableUint x) {
  uint64_t h = 0;
  while (!x.IsOne()) {
    if (x.IsEven()) {
      x.Halve();
    } else {
      x.TimesThreePlusOne();
    }
    ++h;
  }
  return h;
}

int main() {
  VariableUint x;
  for (;;) {
    x.Inc();
    std::cout << x.to_string() << ": " << ComputeHeight(x) << std::endl;
  }
  return 0;
}

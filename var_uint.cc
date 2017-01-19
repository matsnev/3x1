#include "var_uint.h"

#include <utility>

namespace {

const char kHexDigits[] = "0123456789abcdef";

void AppendInHex(std::string* s, uint64_t x, bool pad_to_16) {
  if (pad_to_16) {
    s->resize(s->length() + 16);
    for (auto* current = &s->back(), * done = current - 16;
         current != done; --current) {
      *current = kHexDigits[x & 0xf];
      x >>= 4;
    }
  } else {
    char buffer[16];
    char* current = buffer + 16;
    int count;
    for (count = 0; x; ++count) {
      --current;
      *current = kHexDigits[x & 0xf];
      x >>= 4;
    }
    s->append(current, count);
  }	  
}

}

std::string VariableUint::to_string() const {
  std::string result;
  if (digits_.empty()) {
    result = "0";
    return result;
  }
  result.reserve(digits_.size() * 16);
  bool pad_to_16 = false;
  for (auto it = digits_.crbegin(), end = digits_.crend(); it != end; ++it) { 
    AppendInHex(&result, *it, pad_to_16);
    pad_to_16 = true;
  }
  return result;
}

void VariableUint::Inc() {
  if (digits_.empty()) {
    digits_.push_back(1);
    return;
  }
  for (auto& digit : digits_) {
    if (++digit) {
      return;
    }
  }
  digits_.push_back(1);
}

void VariableUint::Halve() {
  uint64_t carry = 0;
  const auto end = digits_.rend();
  for (auto it = digits_.rbegin(); it != end; ++it) {
    uint64_t new_carry = *it & 1;
    *it /= 2;
    *it |= (carry << 15);
    carry = new_carry;
  }
  Normalize();
}

VariableUint VariableUint::Twice() const {
  VariableUint result;
  result.digits_.reserve(digits_.size());  // might need an extra digit though
  uint64_t carry = 0;
  for (const auto& digit : digits_) {
    uint64_t new_carry = digit >> 15;
    uint64_t new_digit = digit * 2 + carry;
    new_carry += (new_digit < carry);
    result.digits_.push_back(new_digit);
    carry = new_carry;
  }
  if (carry) {
    result.digits_.push_back(carry);
  }
  return result;
}

void VariableUint::Add(const VariableUint& other) {
  uint64_t carry = 0;
  auto it = digits_.begin();
  auto end = digits_.end();
  auto other_it = other.digits_.cbegin();
  auto other_end = other.digits_.cend();
  // Add along the common parts.
  for (; it != end && other_it != other_end; ++it, ++other_it) {
    *it += *other_it;
    uint64_t new_carry = *other_it > *it;
    *it += carry;
    carry = new_carry + (carry > *it);
  }
  // Follow the one that is still remaining, if any, and note that
  // carry can bring either 0 or 1 now.
  if (it != end) {  // but |other| is exhausted
    for (; it != end; ++it) {
      *it += carry;
      carry = (carry > *it);
    }
  } else if (other_it != other_end) {  // |this| was exhausted
    for (; other_it != other_end; ++other_it) {
      uint64_t current = *other_it + carry;
      digits_.push_back(current);
      carry = carry > current;
    }
  }
  // When both vectors are exhausted, we might still have carry remining.
  if (carry) {
    digits_.push_back(carry);
  }
}

namespace {

std::pair<uint64_t, uint64_t> MultiplyByThree(uint64_t x) {
  uint64_t twice = x * 2;
  uint64_t carry = x >> 15;
  uint64_t triple = x + twice;
  return {carry + (triple < x), triple};
}

}

void VariableUint::TimesThreePlusOne() {
  Add(Twice());
  Inc();
}

void VariableUint::Normalize() {
  while (!digits_.back()) {
    digits_.pop_back();
  }
}

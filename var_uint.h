#include <string>
#include <vector>

class VariableUint {
 public:
  std::string to_string() const;

  bool IsZero() const { return digits_.empty(); }

  bool IsOne() const { return digits_.size() == 1 && digits_[0] == 1; }

  bool IsEven() const { return digits_.empty() || digits_[0] % 2 == 0; }

  void Inc();

  void Halve();

  VariableUint Twice() const;

  void Add(const VariableUint& other);

  void TimesThreePlusOne();

  void Normalize();

 private:
  std::vector<uint64_t> digits_;
};


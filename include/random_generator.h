#include "util/random.h"
#include "util/histogram.h"
#include "util/slice.h"
#include "config.h"

namespace flashbench {

class RandomGenerator {
 private:
  std::string data_;
  int pos_;

 public:
  RandomGenerator();

  Slice Generate(size_t len);
  Slice RandomString(Random* rnd, int len, std::string* dst);
  Slice CompressibleString(Random* rnd, double compressed_fraction, size_t len,
                           std::string* dst);

};

}  // namespace flashbench
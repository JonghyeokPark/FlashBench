#include "random_generator.h"

namespace flashbench {

  RandomGenerator::RandomGenerator() {
    // We use a limited amount of data over and over again and ensure
    // that it is larger than the compression window (32KB), and also
    // large enough to serve all typical value sizes we want to write.
    Random rnd(301);
    std::string piece;
    while (data_.size() < 1048576) {
      // Add a short fragment that is as compressible as specified
      // by FLAGS_compression_ratio.
      // (jhaprk): we don't manage compression jsut set compressed_fraction to 1
      CompressibleString(&rnd, 1, 100, &piece);
      data_.append(piece);
    }
    pos_ = 0;
  }

  Slice RandomGenerator::Generate(size_t len) {
    if (pos_ + len > data_.size()) {
      pos_ = 0;
      assert(len < data_.size());
    }
    pos_ += len;
    return Slice(data_.data() + pos_ - len, len);
  }

  Slice RandomGenerator::RandomString(Random* rnd, int len, std::string* dst) {
    dst->resize(len);
    for (int i = 0; i < len; i++) {
      (*dst)[i] = static_cast<char>(' ' + rnd->Uniform(95));  // ' ' .. '~'
    }
    return Slice(*dst);
  }

  Slice RandomGenerator::CompressibleString(Random* rnd, double compressed_fraction, size_t len,
                           std::string* dst) {
    int raw = static_cast<int>(len * compressed_fraction);
    if (raw < 1) raw = 1;
    std::string raw_data;
    RandomString(rnd, raw, &raw_data);

    // Duplicate the random data until we have filled "len" bytes
    dst->clear();
    while (dst->size() < len) {
      dst->append(raw_data);
    }
    dst->resize(len);
    return Slice(*dst);
  }

} // namespace flashbench
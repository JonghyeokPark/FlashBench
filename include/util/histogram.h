//
// Created by JonghyeokPark on 2020/09/26.
// E-mail: akindo19@skku.edu
// Borrow from [LevelDB source](https://github.com/google/leveldb)
//

#ifndef FLASH_BENCH_UTIL_HISTOGRAM_H_
#define FLASH_BENCH_UTIL_HISTOGRAM_H_

#include <string>

namespace flashbench {

class Histogram {
 public:
  Histogram() {}
  ~Histogram() {}

  void Clear();
  void Add(double value);
  void Merge(const Histogram& other);

  std::string ToString() const;

 private:
  enum { kNumBuckets = 154 };

  double Median() const;
  double Percentile(double p) const;
  double Average() const;
  double StandardDeviation() const;

  static const double kBucketLimit[kNumBuckets];

  double min_;
  double max_;
  double num_;
  double sum_;
  double sum_squares_;

  double buckets_[kNumBuckets];
};

}  // namespace flashbench

#endif //FLASH_BENCH_UTIL_HISTOGRAM_H_

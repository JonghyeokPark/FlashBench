#pragma once 

#include "util/random.h"
#include "util/histogram.h"
#include "util/slice.h"
#include <sys/time.h>
#include <vector>
#include "config.h"
#include "port/port_stdcxx.h"

namespace flashbench {

#if defined(__linux)
static inline Slice TrimSpace(Slice s) {
  size_t start = 0;
  while (start < s.size() && isspace(s[start])) {
    start++;
  }
  size_t limit = s.size();
  while (limit > start && isspace(s[limit - 1])) {
    limit--;
  }
  return Slice(s.data() + start, limit - start);
}
#endif

static inline void AppendWithSpace(std::string* str, Slice msg) {
  if (msg.empty()) return;
  if (!str->empty()) {
    str->push_back(' ');
  }
  str->append(msg.data(), msg.size());
}

class Stats {
 private:
  double start_;
  double finish_;
  double seconds_;
  int done_;
  int next_report_;
  int64_t bytes_;
  int64_t ops_;
  double last_op_finish_;
  Histogram hist_;
  std::string message_;

 public:
 
   Stats();
   void Start();
   uint64_t NowMicros();
   void Merge(const Stats& other);
   void Stop();

   void AddMessage(Slice msg);
   void FinishedSingleOp();
   void AddBytes(int64_t n);
   void AddOps();
   void Report(const Slice& name);
};

}
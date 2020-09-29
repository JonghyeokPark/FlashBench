#include "stat.h"

namespace flashbench {

Stats::Stats() { 
  Start(); 
}

void Stats::Start() {
  next_report_ = 100;
  hist_.Clear();
  done_ = 0;
  bytes_ = 0;
  seconds_ = 0;
  message_.clear();
  ops_ = 0;
  start_ = finish_ = last_op_finish_ = NowMicros();
}

uint64_t Stats::NowMicros()  {
  static constexpr uint64_t kUsecondsPerSecond = 1000000;
  struct ::timeval tv;
  ::gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * kUsecondsPerSecond + tv.tv_usec;
}

void Stats::Merge(const Stats& other) {
  hist_.Merge(other.hist_);
  done_ += other.done_;
  bytes_ += other.bytes_;
  seconds_ += other.seconds_;
  ops_ += other.ops_;

  if (other.start_ < start_) start_ = other.start_;
  if (other.finish_ > finish_) finish_ = other.finish_;

  // Just keep the messages from one thread
  if (message_.empty()) message_ = other.message_;
}

void Stats::Stop() {
  finish_ = NowMicros();
  seconds_ = (finish_ - start_) * 1e-6;
}

void Stats::AddMessage(Slice msg) { 
  AppendWithSpace(&message_, msg); 
}


void Stats::FinishedSingleOp() {
  if (config::FLAGS_histogram) {
    double now = NowMicros();
    double micros = now - last_op_finish_;
    hist_.Add(micros);
    if (micros > 20000) {
      std::fprintf(stderr, "long op: %.1f micros%30s\r", micros, "");
      std::fflush(stderr);
    }
    last_op_finish_ = now;
  }

  done_++;
  if (done_ >= next_report_) {
    if (next_report_ < 1000)
      next_report_ += 100;
    else if (next_report_ < 5000)
      next_report_ += 500;
    else if (next_report_ < 10000)
      next_report_ += 1000;
    else if (next_report_ < 50000)
      next_report_ += 5000;
    else if (next_report_ < 100000)
      next_report_ += 10000;
    else if (next_report_ < 500000)
      next_report_ += 50000;
    else
      next_report_ += 100000;
    std::fprintf(stderr, "... finished %d ops%30s\r", done_, "");
    std::fflush(stderr);
  }
}

void Stats::AddBytes(int64_t n) { 
  bytes_ += n; 
}

void Stats::AddOps() { 
  ops_++; 
}

void Stats::Report(const Slice& name) {

    std::string extra;
    if (bytes_ > 0) {
      // Rate is computed on actual elapsed time, not the sum of per-thread
      // elapsed times.
      double elapsed = (finish_ - start_) * 1e-6;
      char rate[100];
      std::snprintf(rate, sizeof(rate), "%6.1f MB/s",
                    (bytes_ / 1048576.0) / elapsed);
      extra = rate;
    }
    AppendWithSpace(&extra, message_);

    std::fprintf(stdout, "%-12s : %11.3f micros/op;%s%s\n",
                 name.ToString().c_str(), seconds_ * 1e6 / done_,
                 (extra.empty() ? "" : " "), extra.c_str());               

    if (config::FLAGS_histogram) {
      std::fprintf(stdout, "Microseconds per op:\n%s\n",
                   hist_.ToString().c_str());
    }
    
    std::fflush(stdout);
}

}

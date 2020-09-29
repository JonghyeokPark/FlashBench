#pragma once 

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>

#include "port/port_stdcxx.h"
#include "util/random.h"
#include "util/histogram.h"
#include "util/slice.h"
#include "util/mutexLock.h"
#include "config.h"
#include "stat.h"
#include <pthread.h>
#include <thread>
#include <vector>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef ENABLE_PCM
#include "cpucounters.h"
#endif

namespace flashbench {


// State shared by all concurrent executions of the same benchmark.
struct SharedState {
  /////////////////////////////////////////////////////////////////////
  port::Mutex mu;
  port::CondVar cv GUARDED_BY(mu);
  int total GUARDED_BY(mu);

  // Each thread goes through the following states:
  //    (1) initializing
  //    (2) waiting for others to be initialized
  //    (3) running
  //    (4) done

   int num_initialized GUARDED_BY(mu);
   int num_done GUARDED_BY(mu);
   bool start GUARDED_BY(mu);

   SharedState(int total)
      : cv(&mu), total(total), num_initialized(0), num_done(0), start(false) {}
};

// Per-thread state for concurrent executions of the same benchmark.
struct ThreadState {
  int tid;      // 0..n-1 when running in n threads
  Random rand;  // Has different seeds for different threads
  Stats stats;
  SharedState* shared;

  ThreadState(int index) : tid(index), rand(1000 + index), shared(nullptr) {}
};


struct request_node {
  unsigned char is_read;
  uint64_t offset;
  std::string val;
};

extern std::vector<request_node> request_queue[MAX_THREAD_NUM];

class Benchmark {
 private:

  int num_;
  int reads_;
  int value_size_;
  int heap_counter_;

  void PrintHeader();
  void PrintWarnings();
  void PrintEnvironment();

 public:
  Benchmark()
      : num_(config::FLAGS_num),
        reads_(config::FLAGS_reads < 0 ? config::FLAGS_num : config::FLAGS_reads),
        value_size_(config::FLAGS_value_size),
        heap_counter_(0) {
  }

  ~Benchmark() {
  }

  void Run();
  void RunBenchmark(int n, Slice name,
                    void (Benchmark::*method)(ThreadState*));

  // (jhpark): preload the benchmark workload operation options.
  void load(uint64_t total_num, bool seq);
 
  void test1(ThreadState* thread);
  //  (jhpark): add another test cases
  void test2(ThreadState* thread);
  void test3(ThreadState* thread);
  void test4(ThreadState* thread);


 private:
  struct ThreadArg {
    Benchmark* bm;
    SharedState* shared;
    ThreadState* thread;
    void (Benchmark::*method)(flashbench::ThreadState*);
  };

  static void ThreadBody(void* v);

  void StartThread(void (*thread_main)(void* thread_main_arg),
                   void* thread_main_arg, uint64_t thread_id);

}; // benchmark
} // namespace flashnench
#include <iostream>

//
// FlashBench: Evaluate Flash SSD performance on various workloads.
// Created by JonghyeokPark on 2020/09/25.
// E-mail: akindo19@skku.edu
//

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>
#include <thread>
#include <sys/resource.h>

#include "port/port_stdcxx.h"
#include "util/random.h"
#include "util/histogram.h"
#include "util/slice.h"
#include "util/mutexLock.h"

#include "stat.h"
#include "random_generator.h"
#include "benchmark.h"
#include "config.h"

#include <vector>
#ifdef ENABLE_GOOGLE_PERF
#include "gperftools/profiler.h"
#endif

int main(int argc, char** argv) {

  std::cout << "Hello, FlashBench" << std::endl;

  for (int i = 1; i < argc; i++) {
    double d;
    int n;
    char junk;

    if (flashbench::Slice(argv[i]).starts_with("--benchmarks=")) {
      flashbench::config::FLAGS_benchmarks = argv[i] + strlen("--benchmarks=");
    } else if (sscanf(argv[i], "--num=%d%c", &n, &junk) == 1) {
      flashbench::config::FLAGS_num = n;
    } else if (sscanf(argv[i], "--threads=%d%c", &n, &junk) == 1) {
      flashbench::config::FLAGS_threads = n;
    } else if (sscanf(argv[i], "--value_size=%d%c", &n, &junk) == 1) {
      flashbench::config::FLAGS_value_size = n;
    } else if (sscanf(argv[i], "--histogram=%d%c", &n, &junk) == 1 &&
        (n == 0 || n == 1)) {
      flashbench::config::FLAGS_histogram = n;
    } else if (sscanf(argv[i], "--pool_size=%d%c", &n, &junk) == 1) {
      flashbench::config::FLAGS_pool_size = n;
    } else if (strncmp(argv[i], "--pool_path=", std::strlen("--pool_path=")) == 0) {
      flashbench::config::FLAGS_pool_path = argv[i]+ std::strlen("--pool_path=");
    } else {
      std::fprintf(stderr, "Invalid flag '%s'\n", argv[i]);
      std::exit(1);
    }
  }

  flashbench::Benchmark benchmark;
  benchmark.Run();
  
  return 0;
}
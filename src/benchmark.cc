#include "benchmark.h"
#include "stat.h"
#include "random_generator.h"

namespace flashbench {
  
  std::vector<request_node> request_queue[MAX_THREAD_NUM];

  void Benchmark::PrintHeader() {
    const int kKeySize = 8;
    PrintEnvironment();
    std::fprintf(stdout, "Keys:       %d bytes each\n", kKeySize);
    std::fprintf(
        stdout, "Values:     %d bytes each \n",
        config::FLAGS_value_size);
    std::fprintf(stdout, "Entries:    %d\n", num_);
    std::fprintf(stdout, "RawSize:    %.1f MB (estimated)\n",
                 ((static_cast<int64_t>(kKeySize + config::FLAGS_value_size) * num_) /
                     1048576.0));
    std::fprintf(
        stdout, "FileSize:   %.1f MB (estimated)\n",
        (((kKeySize + config::FLAGS_value_size) * num_) /
            1048576.0));
    PrintWarnings();
    std::fprintf(stdout, "------------------------------------------------\n");
  }

  void Benchmark::PrintWarnings() {
#if defined(__GNUC__) && !defined(__OPTIMIZE__)
    std::fprintf(
        stdout,
        "WARNING: Optimization is disabled: benchmarks unnecessarily slow\n");
#endif
#ifndef NDEBUG
    std::fprintf(
        stdout,
        "WARNING: Assertions are enabled; benchmarks unnecessarily slow\n");
#endif
  }

  void Benchmark::PrintEnvironment() {
#if defined(__linux)
    time_t now = time(nullptr);
    std::fprintf(stderr, "Date:       %s",
                 ctime(&now));  // ctime() adds newline

    FILE* cpuinfo = std::fopen("/proc/cpuinfo", "r");
    if (cpuinfo != nullptr) {
      char line[1000];
      int num_cpus = 0;
      std::string cpu_type;
      std::string cache_size;
      while (fgets(line, sizeof(line), cpuinfo) != nullptr) {
        const char* sep = strchr(line, ':');
        if (sep == nullptr) {
          continue;
        }
        Slice key = TrimSpace(Slice(line, sep - 1 - line));
        Slice val = TrimSpace(Slice(sep + 1));
        if (key == "model name") {
          ++num_cpus;
          cpu_type = val.ToString();
        } else if (key == "cache size") {
          cache_size = val.ToString();
        }
      }
      std::fclose(cpuinfo);
      std::fprintf(stderr, "CPU:        %d * %s\n", num_cpus, cpu_type.c_str());
      std::fprintf(stderr, "CPUCache:   %s\n", cache_size.c_str());
    }
#endif
  }

  void Benchmark::Run() {
    PrintHeader();
    const char* benchmarks = config::FLAGS_benchmarks;
    while (benchmarks != nullptr) {
      const char* sep = strchr(benchmarks, ',');

      Slice name;
      if (sep == nullptr) {
        name = benchmarks;
        benchmarks = nullptr;
      } else {
        name = Slice(benchmarks, sep - benchmarks);
        benchmarks = sep + 1;
      }

      // Reset parameters that may be overridden below
      num_ = config::FLAGS_num;
      value_size_ = config::FLAGS_value_size;

      void (Benchmark::*method)(ThreadState*) = nullptr;
      bool fresh_db = false;
      int num_threads = config::FLAGS_threads;

      // TODO(jhpark): add benchmark type
      if (name == Slice("test1")) {
        load(num_, true);
        method = &Benchmark::test1;
      } else if (name == Slice("test2")) {
        load(num_, true);
        //method = &Benchmark::DoWrite;
      } else {
        if (!name.empty()) {  // No error message for empty name
          std::fprintf(stderr, "unknown benchmark '%s'\n",
                       name.ToString().c_str());
        }
      }
      if (method != nullptr) {

#ifdef ENABLE_GOOGLE_PERF
        ProfilerStart("./prof.out");
        RunBenchmark(num_threads, name, method);
        ProfilerFlush();
        ProfilerStop();
#elif ENABLE_PCM
        /* record status before performing evaluation */
        std::unique_ptr<pcm::SystemCounterState> before_sstate;
        before_sstate = std::make_unique<pcm::SystemCounterState>();
        *before_sstate = pcm::getSystemCounterState();

        RunBenchmark(num_threads, name, method);

        /* record status after performing evaluation  */
        std::unique_ptr<pcm::SystemCounterState> after_sstate;
        after_sstate = std::make_unique<pcm::SystemCounterState>();
        *after_sstate = pcm::getSystemCounterState();

        std::cout << "PCM Metrics:"
            << "\n"
            << "\tL3 misses: " << getL3CacheMisses(*before_sstate, *after_sstate) << "\n"
            << "\tDRAM Reads (bytes): " << getBytesReadFromMC(*before_sstate, *after_sstate) << "\n"
            << "\tDRAM Writes (bytes): " << getBytesWrittenToMC(*before_sstate, *after_sstate) << "\n"
            << "\tNVM Reads (bytes): " << getBytesReadFromPMM(*before_sstate, *after_sstate) << "\n"
            << "\tNVM Writes (bytes): " << getBytesWrittenToPMM(*before_sstate, *after_sstate) << std::endl;

#else
        RunBenchmark(num_threads, name, method);
#endif

      }
    }
  }


  void Benchmark::ThreadBody(void* v) {
    ThreadArg* arg = reinterpret_cast<ThreadArg*>(v);
    SharedState* shared = arg->shared;
    ThreadState* thread = arg->thread;
    {
      MutexLock l(&shared->mu);
      shared->num_initialized++;
      if (shared->num_initialized >= shared->total) {
        shared->cv.SignalAll();
      }
      while (!shared->start) {
        shared->cv.Wait();
      }
    }

    thread->stats.Start();
    (arg->bm->*(arg->method))(thread);
    thread->stats.Stop();
    {
      MutexLock l(&shared->mu);
      shared->num_done++;
      if (shared->num_done >= shared->total) {
        shared->cv.SignalAll();
      }
    }
  }


   void Benchmark::StartThread(void (*thread_main)(void* thread_main_arg),
                   void* thread_main_arg, uint64_t thread_id)  {
    std::thread new_thread(thread_main, thread_main_arg);
    
    // (jhpark): set 1 to 1 mapping 
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id, &cpuset);
    int rc = pthread_setaffinity_np(new_thread.native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }

    new_thread.detach();
  }

  void Benchmark::RunBenchmark(int n, Slice name,
                    void (Benchmark::*method)(ThreadState*)) {
    SharedState shared(n);

    ThreadArg* arg = new ThreadArg[n];
    for (int i = 0; i < n; i++) {
      arg[i].bm = this;
      arg[i].method = method;
      arg[i].shared = &shared;
      arg[i].thread = new ThreadState(i);
      arg[i].thread->shared = &shared;

      StartThread(ThreadBody, &arg[i], i);
    }

    shared.mu.Lock();
    while (shared.num_initialized < n) {
      shared.cv.Wait();
    }

    shared.start = true;
    shared.cv.SignalAll();
    while (shared.num_done < n) {
      shared.cv.Wait();
    }
    shared.mu.Unlock();

    for (int i = 1; i < n; i++) {
      arg[0].thread->stats.Merge(arg[i].thread->stats);
    }
    arg[0].thread->stats.Report(name);

    for (int i = 0; i < n; i++) {
      delete arg[i].thread;
    }
    delete[] arg;
  }

  void Benchmark::load(uint64_t total_num, bool seq) {
    request_node req;
    RandomGenerator gen;
    int64_t bytes = 0;
    const unsigned char SET_READ_FLAG = 0x1;
    unsigned char read_flag = 0;

    ThreadState* thd = new ThreadState(0);
    for (int n=0; n < config::FLAGS_threads; n++) {
        for (uint64_t i=0; i < total_num; i++) {
          int k = seq ? i : (thd->rand.Next() % (config::FLAGS_pool_size/config::FLAGS_value_size));
          k *= config::FLAGS_value_size;
          Slice val = gen.Generate(value_size_);
          std::string value = val.ToString();
          // (jhpark): pick the operation type (read or write)
          req.offset = k;
          req.is_read = (read_flag ^= SET_READ_FLAG);
          req.val = value;
          request_queue[n].push_back(req);
      }
    }
  std::cerr << "[INFO] loading finished : (" << total_num << ")!\n";
  delete thd;
 }


} // namespace flashbench
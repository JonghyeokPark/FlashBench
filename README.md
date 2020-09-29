# FlashBench

FlashBench is benchmark tool for exploring Flash SSD charateristics

# Building

## Compile
The `--recursive` option indicates that submodules should also be cloned. To generate the build files, type:
```bash
$ git clone --recursive https://github.com/JonghyeokPark/FlashBench.git
$ cd FlashBench
$ mkdir build
```

```bash
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make -j
```

# Running 

## Intel PCM

`FlashBench` relies on the [Intel PCM](https://github.com/opcm/pcm) to collect hardware metrics.
It needs access to model-specific registers (MSRs) that need set up by loading
the `msr` kernel module. Then, load the module:
```bash
$ modprobe msr
```
It may happen that the following message is displayed during runtime:
```
Error while reading perf data. Result is -1
Check if you run other competing Linux perf clients.
```
If so, you can comment the following line in `pcm/Makefile`:
```
CXXFLAGS += -DPCM_USE_PERF
```

---

## Run benchmark 
- The `FlashBench` executable is located in `build/src/`

```bash
$ ./src/FlashBench --benchmarks="test1" --pool_path=/dev/sdb --threads=1 \
                   --num=1000000 --pool_size=1073741824
```

- parameter 

| System Variable         | Description | 
| :-----------------------| :---------- |
| benchmarks              | Benchmark list **Comma-seperated list** |
| pool_path               | Raw device name (e.g, `/dev/sda/`)      |
| pool_size			          | Pool size                               |
| threads                 | Number of concurrent threads to run.    |
| num                     | Number of operations.                   |
| value_size              | Size of value. **512B aligned**         |
| history                 | Print histogram of operation timings    |


# Contribution: How to add Test Cases 

If you want to add another test cases you need to modify these locations

- Declare `your-test-case-name` function as a Benchmark class member function at `/include/benchmark.h`
```bash

class Benchmark {
  ...
  void test1(ThreadState* thread);
  void your-test-case-name(ThreadState* thread);
  ...
}

```

- Add `your-test-case-name` in `Benchmark::Run()` function at `/src/benchmark.cc`
```bash

  void Benchmark::Run() {
      ... 

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
      } else if (name == Slice("your-test-case-name")) {
        load(num_, true);
        method = &Benchmark::your-test-case-name
      } else {
        if (!name.empty()) {  // No error message for empty name
          std::fprintf(stderr, "unknown benchmark '%s'\n",
                       name.ToString().c_str());
        }
      }
  }

```

- Create new file `/src/your-test-case-file.cc` and define your test case function
```bash

#include "benchmark.h"

// your-test-case-name
// Please write down description of test case

namespace flashbench {

void Benchmark::your-test-case-name(ThreadState* thread) {
 // ... 
}

```

Feel free to ask and contribute :smiley:

- :email: akdino19@gmail.com


# References
- [Intel PCM description](https://github.com/sfu-dis/pibench/)
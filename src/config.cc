#include "config.h"

namespace flashbench {
namespace config {

// Number of operations 
int FLAGS_num = 100000;

// Size of each value (e.g. page size 4096 for 4KB page write)
int FLAGS_value_size = 4096;

// Number of concurrent threads to run.
int FLAGS_threads = 1;

// Print histogram of operation timings
bool FLAGS_histogram = false;

// Number of read operations to do.  If negative, do FLAGS_num reads.
int FLAGS_reads = -1;

// Size of pool (Flash area)
uint64_t FLAGS_pool_size = (1024*1024*1024*1UL); // default is 32GB

// Location of pool (Flash area)
const char* FLAGS_pool_path = nullptr;

// Benchmark name
const char* FLAGS_benchmarks =
    "test1,"
    "test2";

} // namespace config
} // namespace flashbench
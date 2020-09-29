#pragma once 

#include <stdint.h>

#define MAX_THREAD_NUM 24
#define FLASHBENCH_INFO_PRINT(fmt, args...)                  \
        fprintf(stderr, "[PMEMMMAP_INFO]: %s:%d:%s():" fmt,  \
        __FILE__, __LINE__, __func__, ##args)                \

#define FLASHBENCH_ERROR_PRINT(fmt, args...)                 \
        fprintf(stderr, "[PMEMMMAP_INFO]: %s:%d:%s():" fmt,  \
        __FILE__, __LINE__, __func__, ##args)                \


namespace flashbench {
namespace config {

// Number of operations 
extern int FLAGS_num;

// Size of each value (e.g. page size 4096 for 4KB page write)
extern int FLAGS_value_size;

// Number of concurrent threads to run.
extern int FLAGS_threads;

// Print histogram of operation timings
extern bool FLAGS_histogram;

// Number of read operations to do.  If negative, do FLAGS_num reads.
extern int FLAGS_reads;

// Size of pool (Flash area)
extern uint64_t FLAGS_pool_size;

// Location of pool (Flash area)
extern const char* FLAGS_pool_path;

// Benchmark name
extern const char* FLAGS_benchmarks;

} // namespace config 
} // namespace flashbench
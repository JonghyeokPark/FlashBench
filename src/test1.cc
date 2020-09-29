#include "benchmark.h"

// test case 1
// sequential read(N) and write(N) (i.e., N means offset)
// each operation size equals FLAGS_value_size

namespace flashbench {

void Benchmark::test1(ThreadState* thread) {
  if (config::FLAGS_pool_path == nullptr) {
    FLASHBENCH_ERROR_PRINT("The pool path is NULL\n");
  }

  int fd = open(config::FLAGS_pool_path, O_RDWR);

  if (fd < 0) {
    FLASHBENCH_ERROR_PRINT("Open error filename: %s\n", config::FLAGS_pool_path);
  }

  char buf[config::FLAGS_value_size] = {0,};

  int64_t found = 0;
  int64_t total_read = 0;
  int64_t bytes = 0;
  std::string ret;

  if (num_ != config::FLAGS_num) {
    char msg[100];
    std::snprintf(msg, sizeof(msg), "(%d ops)", num_);
    thread->stats.AddMessage(msg);
  }


  int thd_sum = 0;
  for (int i=0; i < num_; i++) {
    if (request_queue[thread->tid][i].is_read) {
      // read phase
      total_read++;
    
      // offset 
      lseek(fd, request_queue[thread->tid][i].offset, SEEK_SET);
      
      if (read(fd, buf, 1) < 0) {
        FLASHBENCH_ERROR_PRINT("Read error\n");
      }
      thread->stats.FinishedSingleOp();
    } else { 
      // write phase
      lseek(fd, request_queue[thread->tid][i].offset, SEEK_SET);
      if(write(fd, request_queue[thread->tid][i].val.c_str(), config::FLAGS_value_size) < 0) {
        FLASHBENCH_ERROR_PRINT("Write error\n");
      }
      thread->stats.FinishedSingleOp();
    
      bytes += value_size_ + sizeof(request_queue[thread->tid][i].offset);
      thread->stats.AddBytes(bytes);
      thread->stats.AddOps();
    }
  }

    char msg[100];
    std::snprintf(msg, sizeof(msg), "read : %lu write : %lu\n", total_read, num_ - total_read);
    thread->stats.AddMessage(msg);
  }

} // namespace flashbench
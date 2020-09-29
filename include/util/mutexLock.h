//
// Created by JonghyeokPark on 2020/09/26.
// E-mail: akindo19@skku.edu
// Borrow from [LevelDB source](https://github.com/google/leveldb)
// Copyright 2017 The LevelDB Authors. All rights reserved.
//


#ifndef FLASH_BENCH_UTIL_MUTEXLOCK_H_
#define FLASH_BENCH_UTIL_MUTEXLOCK_H_

#include "port/port.h"
#include "port/thread_annotations.h"

namespace flashbench {

// Helper class that locks a mutex on construction and unlocks the mutex when
// the destructor of the MutexLock object is invoked.
//
// Typical usage:
//
//   void MyClass::MyMethod() {
//     MutexLock l(&mu_);       // mu_ is an instance variable
//     ... some complex code, possibly with multiple return paths ...
//   }

class SCOPED_LOCKABLE MutexLock {
    public:
    explicit MutexLock(port::Mutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu) : mu_(mu) {
      this->mu_->Lock();
    }
    ~MutexLock() UNLOCK_FUNCTION() { this->mu_->Unlock(); }

    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

    private:
    port::Mutex* const mu_;
};

}  // namespace flashbench

#endif //FLASH_BENCH_UTIL_MUTEXLOCK_H_

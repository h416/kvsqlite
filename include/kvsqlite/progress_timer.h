//      Copyright Shinichiro Hirama 2015.
// Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//      http://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDE_KVSQLITE_PROGRESS_TIMER_H_
#define INCLUDE_KVSQLITE_PROGRESS_TIMER_H_

#include <iostream>
#include <chrono>

namespace kvsqlite {

// progress_timer
class progress_timer
{
 public:
  progress_timer():start_(std::chrono::system_clock::now()){}
  ~progress_timer(){
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count() << "us" << std::endl;
  }
 private:
  std::chrono::system_clock::time_point start_;
};

}  // namespace kvsqlite

#endif  //  INCLUDE_KVSQLITE_PROGRESS_TIMER_H_

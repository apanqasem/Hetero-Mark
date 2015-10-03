/*
 * Hetero-mark
 *
 * Copyright (c) 2015 Northeastern University
 * All rights reserved.
 *
 * Developed by:
 *   Northeastern University Computer Architecture Research (NUCAR) Group
 *   Northeastern University
 *   http://www.ece.neu.edu/groups/nucar/
 *
 * Author: Yifan Sun (yifansun@coe.neu.edu)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal with the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 *   Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimers.
 *
 *   Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimers in the 
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of NUCAR, Northeastern University, nor the names of 
 *   its contributors may be used to endorse or promote products derived 
 *   from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS WITH THE SOFTWARE.
 */

#ifndef SRC_HSA_PERSISTENT_KERNEL_PERSISTENT_KERNEL_BENCHMARK_H_
#define SRC_HSA_PERSISTENT_KERNEL_PERSISTENT_KERNEL_BENCHMARK_H_

#include <cstdio>
#include <cstdlib>
#include <hsa.h>
#include <hsa_ext_finalize.h>

#include "src/common/benchmark/benchmark.h"
#include "src/common/time_measurement/timer.h"
#include "src/common/runtime_helper/hsa_runtime_helper/hsa_runtime_helper.h"
#include "src/common/runtime_helper/hsa_runtime_helper/hsa_executable.h"
#include "src/common/runtime_helper/hsa_runtime_helper/hsa_signal.h"

class PersistentKernelBenchmark : public Benchmark {
 public:
  PersistentKernelBenchmark(HsaRuntimeHelper *runtime_helper, Timer *timer);
  void Initialize() override;
  void Run() override;
  void Verify() override;
  void Summarize() override;
  void Cleanup() override;

 private:
  float *in_;
  float *out_;

  int num_tasks_ = 10;
  int task_scheduled_ = 0;
  int task_started_ = 0;
  uint64_t *time_diff_;
  double *task_schedule_time_;
  double *task_start_time_;
  double *task_complete_time_;
  double cpu_start_;
  double cpu_end_;

  HsaRuntimeHelper *runtime_helper_;
  HsaAgent *agent_;
  HsaExecutable *executable_;
  HsaKernel *kernel_;
  AqlQueue *queue_;
  HsaSignal *task_dispatch_signal_;
  HsaSignal *task_return_signal_;

  Timer *timer_;

  bool kernel_stopped_ = false;

  void GenerateTask();
  void ScheduleTask();
};

#endif  // SRC_HSA_PERSISTENT_KERNEL_PERSISTENT_KERNEL_BENCHMARK_H_
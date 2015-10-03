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

#include "src/hsa/non_persistent_kernel/persistent_kernel_benchmark.h"

#include <iostream>
#include <thread>
#include <chrono>

PersistentKernelBenchmark::PersistentKernelBenchmark(
    HsaRuntimeHelper *runtime_helper, 
    Timer *timer) :
  runtime_helper_(runtime_helper),
  timer_(timer) {
}

void PersistentKernelBenchmark::Initialize() {

  // Generate tasks
  GenerateTask();

  // Initialize runtime
  runtime_helper_->InitializeOrDie();

  // Find GPU device
  agent_ = runtime_helper_->FindGpuOrDie();
  std::cout << agent_->GetNameOrDie() << "\n";

  // Create program from source
  executable_ = runtime_helper_->CreateProgramFromSourceOrDie(
      "kernels.brig", agent_);

  // Get Kernel
  kernel_ = executable_->GetKernel("&__OpenCL_vector_copy_kernel", 
      agent_);
  
  // Create a queue
  queue_ = agent_->CreateQueueOrDie();

  // Prepare buffers 
  task_dispatch_signal_ = runtime_helper_->CreateSignal(0);
  task_return_signal_ = runtime_helper_->CreateSignal(0);

  // Prepare to launch kernel
  kernel_->SetDimension(1);
  kernel_->SetLocalSize(1, 64);
  kernel_->SetGlobalSize(1, 64);
  kernel_->SetKernelArgument(1, 8, task_dispatch_signal_->GetNative());
  kernel_->SetKernelArgument(2, 8, task_return_signal_->GetNative());
}

void PersistentKernelBenchmark::Run() {
  // Start the kernel
  printf("CPU started: %f\n", timer_->GetTimeInSec());

  // Start the task generator
  sched_param sch;
  sch.sched_priority = 99;
  std::thread task_generation_thread = 
    std::thread([this] {this->ScheduleTask();});
  pthread_setschedparam(task_generation_thread.native_handle(), 
      SCHED_FIFO, &sch);

  while(task_started_ < num_tasks_) {
    if (task_started_ < task_scheduled_) {
      task_started_++;

      task_start_time_[task_started_] = timer_->GetTimeInSec();
      kernel_->ExecuteKernel(agent_, queue_);
      task_complete_time_[task_started_] = timer_->GetTimeInSec();
    }
  }

  // Wait fot the task generation thread stop
  task_generation_thread.join();

  // Send terminate signal
  printf("CPU ended: %f\n", timer_->GetTimeInSec());
}

void PersistentKernelBenchmark::ScheduleTask() {
  cpu_start_ = timer_->GetTimeInSec();
  for(int i = 0; i < num_tasks_; i++) {
    //for (uint64_t j = 0; j < time_diff_[i]; j++) {}
    std::this_thread::sleep_for(std::chrono::nanoseconds(time_diff_[i]));
    task_schedule_time_[i+1] = timer_->GetTimeInSec();
    task_scheduled_++;
  } 
  cpu_end_ = timer_->GetTimeInSec();
}

void PersistentKernelBenchmark::GenerateTask() {
  time_diff_ = new uint64_t[num_tasks_];
  task_schedule_time_ = new double[num_tasks_];
  task_start_time_ = new double[num_tasks_];
  task_complete_time_ = new double[num_tasks_];

  srand(1);
  for (int i = 0; i < num_tasks_; i++) {
    time_diff_[i] = rand() % 1000;
    printf("time_diff_[i] = %ld\n", time_diff_[i]);
  }
}

void PersistentKernelBenchmark::Verify() {
}

void PersistentKernelBenchmark::Summarize() {
  printf("CPU: (%f, %f)\n", cpu_start_, cpu_end_);
  for (int i = 0; i < num_tasks_; i++) {
    printf("(%f, %f, %f),\n", 
        task_schedule_time_[i], 
        task_start_time_[i],
        task_complete_time_[i]);
  }
}

void PersistentKernelBenchmark::Cleanup() {
  delete[] time_diff_;
  delete[] task_schedule_time_;
  delete[] task_start_time_;
  delete[] task_complete_time_;
}

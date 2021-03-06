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

#define NUM_VARIABLES 500


typedef struct {
  double fitness;
  double parameters[NUM_VARIABLES];
} Creature;


__kernel void Evaluate_Kernel(__global Creature *creatures, __global double *fitness_function,
			      uint count, uint num_vars) {
  uint i = get_global_id(0);
  if (i >= count) return;

  double fitness = 0;
  for (int j = 0; j < num_vars; j++) {
    double pow = 1;
    for (int k = 0; k < j + 1; k++) {
      pow *= creatures[i].parameters[j];
    }
    fitness += pow * fitness_function[j];
  }
  creatures[i].fitness = fitness;
}

__kernel void Mutate_Kernel(__global Creature *creatures, uint count,
			    uint num_vars) {
  uint i = get_global_id(0);

  if (i >= count) return;

  if (i % 7 != 0) return;
  creatures[i].parameters[i % num_vars] *= 0.5;
}

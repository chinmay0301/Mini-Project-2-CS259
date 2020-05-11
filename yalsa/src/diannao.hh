#ifndef DIANNAO_H
#define DIANNAO_H

#include "loopnest.hh"

// Performance Model for DianNao
// Not particularly accurate, but to first order okay
class DianNao {
public:
  // Define the sizes of the scratchpad buffers
  int synapse_bytes=16*16*2*64; //bytes
  int neuron_in_bytes=16*2*64;
  int neuron_out_bytes=16*2*64;

  // Other Hardware Paramters
  int mem_bw=200; //GB/s
  int comp_bw=16*16; //iters /cycle
  float frequency = 1; //ghz

  // Execution Time in seconds
  // This will be the maximum of:
  //   memory bound time & computation bound time
  float get_exec_time(Loopnest& ln, Array* arr_syn, Array* arr_in, Array* arr_out) {
    int lvl;

    // Each one of the following contributes to total memory
    // bandwidth:
    float bw_syn = ln.bandwidth_for_scratchpad(arr_syn, 2, synapse_bytes,  comp_bw, lvl);
    float bw_in  = ln.bandwidth_for_scratchpad(arr_in,  2, neuron_in_bytes,comp_bw, lvl);
    float bw_out = ln.bandwidth_for_scratchpad(arr_out, 2, neuron_out_bytes,  comp_bw, lvl);
  
    // Convert to gb/s
    float total_bw = bw_syn + bw_in + bw_out; // bytes / cycle
    float total_gbps = total_bw / (1024 * 1024 * 1024) * 
                       frequency * 1000000000;
    
    // This is how much we're slowing down the computation
    // as compared to the computation bound time
    float bw_ratio = total_gbps/mem_bw; 

    float comp_bound_cycles = ln.iters_at_level(0) / comp_bw;
    float comp_bound_seconds = comp_bound_cycles / frequency;
   
    float mem_bound_seconds = bw_ratio * comp_bound_seconds;

    // Take the max of computation bound and memory bound time
    return max(comp_bound_seconds,mem_bound_seconds);
  }

  // We can convert from execution time to flops
  float get_flops(Loopnest& ln, Array* arr_syn, Array* arr_in, Array* arr_out) {
    float seconds = get_exec_time(ln,arr_syn,arr_in,arr_out);
    int flops_per_iter = 2;
    float total_flop_count =  ln.iters_at_level(0) * flops_per_iter;
    return total_flop_count / seconds;
  }
};

#endif

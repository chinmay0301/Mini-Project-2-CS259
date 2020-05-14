#ifndef GPU_H
#define GPU_H

#include "loopnest.hh"

// Performance Model for DianNao
// Not particularly accurate, but to first order okay

class GPU {
public:
  // Define the sizes of the scratchpad buffers 
  // Other Hardware Paramters
  int L1_bytes = 96*1024*80;
  int L2_bytes = 4718592;
  float frequency_gpu = 1.46; //ghz
  float mem_bw_gpu = 653; // gbps
  float l2_bw_gpu = 4000;  // gpbs
  float comp_bw_gpu = 5120; // iters/cycle
  // Execution Time in seconds
  // This will be the maximum of:
  //   memory bound time & computation bound time
  
  float get_gpu_exec_time(Loopnest& ln, int display = 1) {
    int lvl;
    int lvl_l2;
    float alpha = 0.9;
    float total_gbps, bw_ratio;
    // Each one of the following contributes to total memory
    // bandwidth:
    float bw_L1 = ln.bandwidth_for_cache(4, L1_bytes, comp_bw_gpu, lvl); // bytes / cycle
    float l2_eff_bw = alpha*l2_bw_gpu + (1-alpha)*mem_bw_gpu;
    float bw_L2 = ln.bandwidth_for_cache(4, L2_bytes, comp_bw_gpu, lvl_l2);
    // Convert to gb/s
    if(display == 1)
    printf("bw_L1, %f, bw_L2, %f,", bw_L1, bw_L2);
    
    if(ln.dims[VarN]<=32){
      total_gbps = bw_L1 / (1024 * 1024 * 1024) * 
                       frequency_gpu * 1000000000;

      // This is how much we're slowing down the computation
      // as compared to the computation bound time
      bw_ratio = total_gbps/mem_bw_gpu;
    }
    else{
      total_gbps = bw_L2 / (1024 * 1024 * 1024) * 
                       frequency_gpu * 1000000000;
      bw_ratio = total_gbps/l2_bw_gpu;
    }
    
    if (display == 1)
    printf("bw_ratio, %f,",bw_ratio);
    long double comp_bound_cycles = ln.iters_at_level(0) / comp_bw_gpu;
    float comp_bound_seconds = comp_bound_cycles / (frequency_gpu*1000);
    
    // printf("comp_bound_seconds %f \n", comp_bound_seconds);

    float mem_bound_seconds = bw_ratio * comp_bound_seconds;

    // Take the max of computation bound and memory bound time
    return max(comp_bound_seconds,mem_bound_seconds);
  }

  void comp_bw_analysis(Loopnest& ln, int low_lim = 4500, int up_lim = 10000) {
    printf("Comp_bw, Exec_Time \n");
    float exec_time;
    for (int i = low_lim; i<=up_lim; i++) {
      comp_bw_gpu = i;
      exec_time = get_gpu_exec_time(ln, 0);
      printf("%f, %f\n", comp_bw_gpu, exec_time);
    }

  } 
  
  void mem_bw_analysis(Loopnest& ln, int low_lim = 300, int up_lim = 1200) {
    printf("mem_bw, Exec_Time \n");
    float exec_time;
    for (int i = low_lim; i<=up_lim; i++) {
      mem_bw_gpu = i;
      exec_time = get_gpu_exec_time(ln, 0);
      printf("%f, %f\n", mem_bw_gpu, exec_time);
    }

  }  

};


#endif

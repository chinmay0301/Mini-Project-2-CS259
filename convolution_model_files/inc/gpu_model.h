#ifndef GPU_H
#define GPU_H

//#include "loopnest.h"

class GPU{
public: 

// hardware params 
int L2_bytes = 4718592; //4000000; // in bytes
int L1_bytes = 80 * 1024 * 96;
float frequency_gpu = 1.46; // Ghz
float mem_bw_gpu = 653; // GBPs 
int numThreads = 1024;  // for this particular case 
float comp_bw_gpu = 5120;
float winograd_factor = 1.5; // change this for different conv kernels 
float get_exec_time_L2_bound(Loopnest& ln){
	int lvl;

    // bandwidth using L2 cache
    float bw = ln.bandwidth_for_cache(4, L2_bytes, comp_bw_gpu, lvl); // bytes / cycle
  
    // Convert to gb/s
    float total_gbps = bw / (1024 * 1024 * 1024) * 
                       frequency_gpu * 1000000000;
    
    // This is how much we're slowing down the computation
    // as compared to the computation bound time
    float bw_ratio = total_gbps/mem_bw_gpu; 
    //printf("bw cache %f \n", total_gbps);
    
    float comp_bound_cycles = ln.iters_at_level(0) / (winograd_factor*comp_bw_gpu);
    float comp_bound_seconds = comp_bound_cycles / frequency_gpu;
    
   //printf("comp_bound_seconds %f \n", comp_bound_seconds);

    float mem_bound_seconds = bw_ratio * comp_bound_seconds;

    // Take the max of computation bound and memory bound time
    return max(comp_bound_seconds,mem_bound_seconds)/1000;
	
}
	
float get_exec_time_L1_bound(Loopnest& ln) {
	int lvl;

	// bandwidth using L2 cache
	float bw = ln.bandwidth_for_cache(4, L1_bytes, comp_bw_gpu, lvl); // bytes / cycle

	// Convert to gb/s
	float total_gbps = bw / (1024 * 1024 * 1024) *
		frequency_gpu * 1000000000;

	// This is how much we're slowing down the computation
	// as compared to the computation bound time
	float bw_ratio = total_gbps / mem_bw_gpu;
	printf("bw cache %f \n", bw);

	float comp_bound_cycles = ln.iters_at_level(0) / (1.5*comp_bw_gpu);
	float comp_bound_seconds = comp_bound_cycles / frequency_gpu;

	printf("comp_bound_seconds %f \n", comp_bound_seconds);

	float mem_bound_seconds = bw_ratio * comp_bound_seconds;

	// Take the max of computation bound and memory bound time
	return max(comp_bound_seconds, mem_bound_seconds) / 1000;

}

};



#endif

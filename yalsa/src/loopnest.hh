#ifndef LOOPNEST_H
#define LOOPNEST_H

#include <vector>
#include <stdio.h>
#include <string>

// Variables are given an integer index for simplicity:
#define VarN 0 //batch
#define VarC 1 //input channels
#define VarK 2 //output channels
#define VarX 3 //Width
#define VarY 4 //Height
#define VarS 5 //Filter Row
#define VarR 6 //Filter Column
#define VarCount 7 //Filter Column

//void print_var(int v) {
//  switch(v) {
//    case VarN: printf("N");
//    case VarC: printf("C");
//    case VarK: printf("K");
//    case VarX: printf("X");
//    case VarY: printf("Y");
//    case VarS: printf("S");
//    case VarR: printf("R");
//    default: printf("?");
//  }
//}

using namespace std;

//very simple indexing expression
//assumed that indices are added together
class Array {
public:
  string name;
  vector<vector<int>> coupling;
  Array(string n,vector<vector<int>> c) {
    coupling = c;
    name=n;    
  }
};

class Loop {
public:
  int var;
  int iters;
  Loop(int v, int i) {
    var=v;
    iters=i;
  }
};

class Loopnest {
public:
  std::vector<int> dims; // how big is each dimension
  std::vector<Array> arrays;

  std::vector<Loop> loops;

  Loopnest() {
    dims.resize(VarCount);
  }

  void get_extent(std::vector<int>& extent, int start_loop_level);
  
  long int volume_at_level(Array& arr, int start_loop_level);
  
  long int iters_at_level(int start_loop_level);
  
  float bandwidth_for_cache(int datatype_bytes, int cache_bytes, int iters_per_cycle, int & lvl);
  float bandwidth_for_scratchpad(Array* array,
      int datatype_bytes, int scratchpad_bytes, int iters_per_cycle, int & lvl);

  void print_volume_analysis();
  float mm_naive_exec_time(long int compute_peak_flops, long int memory_peak_bw, int datatype_bytes);
  void print_bandwidth_analysis();

};


#endif

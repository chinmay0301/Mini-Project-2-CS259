// Yet Another Simple Loop Analysis

#include <assert.h>
//#include <getopt.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>

#include "loopnest.h"
#include "gpu_model.h"
using namespace std;

void add_example_conv(std::vector<Loopnest>& lns,int N) {
  Loopnest b;
  lns.push_back(b);
  Loopnest& ln = lns.back();

  // This is where the problem parameters are set
  //DianNao -- Conv3
  ln.dims[VarN]= N;
  ln.dims[VarC]=512;
  ln.dims[VarK]=512;
  ln.dims[VarX]=14;
  ln.dims[VarY]=14;
  ln.dims[VarR]=3;
  ln.dims[VarS]=3;

  // Arrays are defined by how they couple with variables.
  // Two variables in a list implies addition (yes, quite a limitation : )

  Array array_i = Array("i",{{VarN},{VarC},{VarY,VarR},{VarX,VarS}});  //Y' X'
  Array array_w = Array("w",{{VarK},{VarC},{VarR},{VarS}});
  Array array_o = Array("o",{{VarN},{VarK},{VarY},{VarX}}); //Y' X'

  ln.arrays.push_back(array_i);
  ln.arrays.push_back(array_w);
  ln.arrays.push_back(array_o);

  // These tiling factors matched what I assumed in the slides, they are not optimal : )
  int Tn = min(2, N);
  int Tk = 8; //inner tiling for output channels
  int Tc = 8; //tiling for input channels
  int Tkk = 64; //outer tiling for output channels
  int Tx = 4; //tiling for x dimension
  int Ty = 4; //tiling for y dimension

  // * Loop schedule is defined from outer to inner loops 
  //
  // * Loops may only iterate over one dimension of the data (another
  // limitation)
  //
  // * Loops may iterate over a subset of the data by specifying a tile size
  // less than the total extent of the data.
  //
  // * For any given data dimension, tile sizes must be decreasing as you go
  // further inward in the loop.  Otherwise bad things will happen.

  // DianNao Convolution Loop Schedule -- not the idea schedule I must say
  auto& loops = ln.loops;
  auto& dims = ln.dims;

  loops.emplace_back(VarN,dims[VarN]); // Batch loop
  loops.emplace_back(VarK, dims[VarK]); // Output Channel Loop
  loops.emplace_back(VarY,dims[VarY]); // Spatial Loops
  loops.emplace_back(VarX,dims[VarX]);

  loops.emplace_back(VarN, Tn); // Batch loop
  loops.emplace_back(VarK, Tkk); // First Tile Output Channel Loop
  loops.emplace_back(VarY,Ty); // Tile Spatial Loop
  loops.emplace_back(VarX,Tx);
  
  loops.emplace_back(VarS,dims[VarS]); // Filter Loops
  loops.emplace_back(VarR,dims[VarR]);
  loops.emplace_back(VarC,dims[VarC]); // Input Channel Loop
  loops.emplace_back(VarK,Tk); // Second Tile Output Channel Loop
  loops.emplace_back(VarC,Tc); // Tile Input Channel Loop

}

void add_example_mm(std::vector<Loopnest>& lns, int N) {
  Loopnest b;
  lns.push_back(b);
  Loopnest& ln = lns.back();
  ln.dims[VarN]=N;
  ln.dims[VarC]=4096;
  ln.dims[VarK]=1024;

  Array array_a = Array("a",{{VarN},{VarC}});
  Array array_b = Array("b",{{VarC},{VarK}});
  Array array_c = Array("c",{{VarN},{VarK}});

  ln.arrays.push_back(array_a);
  ln.arrays.push_back(array_b);
  ln.arrays.push_back(array_c);


  int Tn = max(32, N);
  int Tc=64;
  int Tk=64;

 
  ln.loops.emplace_back(VarC,ln.dims[VarC]);
  ln.loops.emplace_back(VarK,ln.dims[VarK]);
  ln.loops.emplace_back(VarN, ln.dims[VarN]);
  
  ln.loops.emplace_back(VarC,Tc);
  ln.loops.emplace_back(VarK,Tk);
  ln.loops.emplace_back(VarN, Tn);
}

int main(int argc, char* argv[]) {
 
  GPU gpu_model; 
  int i; 

#if 0
  for (i = 1; i < 1024; i *= 2) {
	  std::vector<Loopnest> lns;
	  add_example_mm(lns,i);
	
	  // GPU analysis for matrix multiply 
	  float mm_exec = gpu_model.get_exec_time_L2_bound(lns[0]);
	  printf("Batch Size %d: MM GPU execution time in us = %f\n",i, mm_exec);
	  
  }
#endif
  for (i = 1; i < 1024; i *= 2) {
	  std::vector<Loopnest> lns;
	  add_example_conv(lns, i);
	  float conv_exec = gpu_model.get_exec_time_L2_bound(lns[0]);
	  //lns[0].conv_naive_exec_time(14.9e12, 653e9, 4);
	  printf("Batch Size %d: conv GPU execution time in us = %f\n", i, conv_exec);
  }

  return 0;
}


// Yet Another Simple Loop Analysis

#include <assert.h>
#include <getopt.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>

#include "loopnest.hh"
#include "gpu.hh"

using namespace std;

void add_example_conv(std::vector<Loopnest>& lns) {
  Loopnest b;
  lns.push_back(b);
  Loopnest& ln = lns.back();

  // This is where the problem parameters are set
  //DianNao -- Conv3
  ln.dims[VarN]=1;
  ln.dims[VarC]=108;
  ln.dims[VarK]=200;
  ln.dims[VarX]=32;
  ln.dims[VarY]=32;
  ln.dims[VarR]=4;
  ln.dims[VarS]=4;

  // Arrays are defined by how they couple with variables.
  // Two variables in a list implies addition (yes, quite a limitation : )

  Array array_i = Array("i",{{VarN},{VarC},{VarY,VarR},{VarX,VarS}});  //Y' X'
  Array array_w = Array("w",{{VarK},{VarC},{VarR},{VarS}});
  Array array_o = Array("o",{{VarN},{VarK},{VarY},{VarX}}); //Y' X'

  ln.arrays.push_back(array_i);
  ln.arrays.push_back(array_w);
  ln.arrays.push_back(array_o);

  // These tiling factors matched what I assumed in the slides, they are not optimal : )
  float Tk = 16; //inner tiling for output channels
  float Tc = 16; //tiling for input channels
  float Tkk = 64; //outer tiling for output channels
  float Tx = 8; //tiling for x dimension
  float Ty = 8; //tiling for y dimension

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

  loops.emplace_back(VarY,dims[VarY]); // Spatial Loops
  loops.emplace_back(VarX,dims[VarX]);
  loops.emplace_back(VarK,dims[VarK]); // Output Channel Loop

  loops.emplace_back(VarY,Ty); // Tile Spatial Loop
  loops.emplace_back(VarX,Tx);
  loops.emplace_back(VarK,Tkk); // First Tile Output Channel Loop

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
  ln.dims[VarC]= 4096;
  ln.dims[VarK]= 1024;

  Array array_a = Array("a",{{VarN},{VarC}});
  Array array_b = Array("b",{{VarC},{VarK}});
  Array array_c = Array("c",{{VarN},{VarK}});

  ln.arrays.push_back(array_a);
  ln.arrays.push_back(array_b);
  ln.arrays.push_back(array_c);


  int Tn = (N < 32) ? 32 + N : N;
  int Tk = ln.dims[VarK]/16;
  int Tc = 4*Tk;  // ideally Tc is pow(2, int(log2(lin.dims[VarC]/lin.dims[VarK])))
  
  ln.loops.emplace_back(VarN,ln.dims[VarN]);
  ln.loops.emplace_back(VarK,ln.dims[VarK]);
  ln.loops.emplace_back(VarC,ln.dims[VarC]);
  ln.loops.emplace_back(VarN,Tn);
  ln.loops.emplace_back(VarK,Tk);
  ln.loops.emplace_back(VarC,Tc);
}



int main(int argc, char* argv[]) {
  GPU gpu_model;

  int i=1;
  float beta = 8.0 / 7;
  while(i<1024){
    std::vector<Loopnest> lns;
    add_example_mm(lns,i);

    // Change the value of display to 1 for printing the values for the get_gpu_exec_time function 
    int display = 1;
    float gpu_exec_time = gpu_model.get_gpu_exec_time(lns[0], display);
    float naive_time = lns[0].mm_naive_exec_time(14.9e12, 653e9, 4);
    if (display == 1)
      printf("N:, %d, GPU_time, %f, Naive_time, %f, \n", i,beta*gpu_exec_time, naive_time);
    if(i<10){
      i++;
    }
    else if(i == 10)
      i = 16;
    else{
      i*=2;
    }
  }

  // Architectural insight analysis
  std::vector<Loopnest> lns;
  
  // // Generating matrix with batch size 256
  add_example_mm(lns,256);
  // // Print memory bandwidth analysis
  gpu_model.mem_bw_analysis(lns[0]);
  // Print compute bandwidth analysis
  gpu_model.comp_bw_analysis(lns[0]);


  return 0;
}


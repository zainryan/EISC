#include <cstdio>
#include <cassert>
#include <insider_runtime.hpp>
#include <fcntl.h>

using namespace std;

void dram_write(unsigned int addr, unsigned int data) {
  send_input_params(1);
  send_input_params(addr);
  send_input_params(data);
  assert(dbg_peek(0) == 0);
}

unsigned int dram_read(unsigned int addr) {
  send_input_params(0);
  send_input_params(addr);
  return dbg_peek(0);
}

int main(int argc, char **argv) {
  // In this simple test, we do no need to setup 
  // any vfile. Just call setup_no_vfile provided
  // by INSIDER runtime to do a minimal setup
  // (mmap mmio and reset drive apps).
  setup_no_vfile();
  puts("Begin...");
  unsigned int addr = 0x100;
  unsigned int data = 0x88;
  dram_write(addr, data);
  assert(dram_read(addr) == data);
  puts("PASSED!");

  return 0;
}

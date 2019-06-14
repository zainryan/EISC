#include <cstdio>
#include <cassert>
#include <insider_runtime.hpp>
#include <fcntl.h>

using namespace std;

int main(int argc, char **argv) {
  // In this simple test, we do no need to setup 
  // any vfile. Just call setup_no_vfile provided
  // by INSIDER runtime to do a minimal setup
  // (mmap mmio and reset drive apps).
  setup_no_vfile();
  puts("Begin...");
  assert(dbg_peek(1) == 1);
  assert(dbg_peek(10) == 11);
  assert(dbg_peek(100) == 111);
  puts("PASSED!");

  return 0;
}

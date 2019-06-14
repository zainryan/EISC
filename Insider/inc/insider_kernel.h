#ifndef INSIDER_KERNEL_H_
#define INSIDER_KERNEL_H_

#define COSIM_DRAMA_ADDR_OFFSET (4LL << 32)
#define COSIM_DRAMB_ADDR_OFFSET (8LL << 32)
#define COSIM_DRAMC_ADDR_OFFSET (12LL << 32)
#define COSIM_DRAMD_ADDR_OFFSET (16LL << 32)

#define UNUSED_DBG_PEEK(req, resp)                                             \
  {                                                                            \
    unsigned int dummy;                                                        \
    if (req.read_nb(dummy)) {                                                  \
      resp.write_nb(dummy);                                                    \
    }                                                                          \
  }

#define UNUSED_APP_DATA(input, output)                                         \
  {                                                                            \
    APP_Data dummy;                                                            \
    if (input.read_nb(dummy)) {                                                \
      output.write_nb(dummy);                                                  \
    }                                                                          \
  }

#define UNUSED_APP_INPUT_PARAMS(queue)                                         \
  {                                                                            \
    unsigned int dummy;                                                        \
    queue.read_nb(dummy);                                                      \
  }

#define UNUSED_APP_DRAM(read_req, read_resp, write_req_data, write_req_apply)  \
  {                                                                            \
    Dram_Read_Resp dummy_0;                                                    \
    if (read_resp.read_nb(dummy_0)) {                                          \
      Dram_Read_Req dummy_1;                                                   \
      Dram_Write_Req_Data dummy_2;                                             \
      Dram_Write_Req_Apply dummy_3;                                            \
      read_req.write_nb(dummy_1);                                              \
      write_req_data.write_nb(dummy_2);                                        \
      write_req_apply.write_nb(dummy_3);                                       \
    }                                                                          \
  }

#include <insider_common.h>
#include <staccel_kernel.h>

#endif

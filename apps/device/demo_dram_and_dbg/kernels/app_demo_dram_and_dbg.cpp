#ifndef APP_PT_CPP_
#define APP_PT_CPP_
#include <insider_kernel.h>

void app_demo_dram_and_dbg(ST_Queue<unsigned int> &dbg_peek_req,
                           ST_Queue<unsigned int> &dbg_peek_resp,
                           ST_Queue<Dram_Read_Req> &app_dram_read_req,
                           ST_Queue<Dram_Read_Resp> &app_dram_read_resp,
                           ST_Queue<Dram_Write_Req_Data> &app_dram_write_req_data,
                           ST_Queue<Dram_Write_Req_Apply> &app_dram_write_req_apply,
                           ST_Queue<APP_Data> &app_input_data,
                           ST_Queue<APP_Data> &app_output_data,
                           ST_Queue<unsigned int> &app_input_params) {
  unsigned int state = 0;
  bool is_read_op = 0;
  bool got_read_resp = false;
  unsigned int read_resp_data;

  // state:
  // 0: read opcode (dram read op or dram write op)
  // 1: read addr (for both dram read op and dram write op)
  // 2: read data (only for dram write op)
  // 3: write resp to host (for both dram read op and dram write op)

  while (1) {
#pragma HLS pipeline
    UNUSED_APP_DATA(app_input_data, app_output_data);

    if (state == 0) {
      unsigned int op;
      if (app_input_params.read_nb(op)) {
        is_read_op = (op == 0);
        state = 1;
      }
    } else if (state == 1) {
      unsigned int addr;
      if (app_input_params.read_nb(addr)) {
        if (is_read_op) {
          Dram_Read_Req dram_read_req;
          dram_read_req.num = 1;
          dram_read_req.addr = addr;
          app_dram_read_req.write(dram_read_req);
          state = 3;
        } else {
          Dram_Write_Req_Apply dram_write_req_apply;
          dram_write_req_apply.num = 1;
          dram_write_req_apply.addr = addr;
          app_dram_write_req_apply.write(dram_write_req_apply);
          state = 2;
        }
      }
    } else if (state == 2) {
      unsigned int data;
      if (app_input_params.read_nb(data)) {
        Dram_Write_Req_Data dram_write_req_data;
        dram_write_req_data.last = true;
        // Only write to the first 32 bits.
        dram_write_req_data.data(31, 0) = data;
        app_dram_write_req_data.write(dram_write_req_data);
        state = 3;
      }     
    } else if (state == 3) {
      if (is_read_op) {
        Dram_Read_Resp dram_read_resp;
        if (got_read_resp ||
            (got_read_resp = app_dram_read_resp.read_nb(dram_read_resp))) {
          // Only read the first 32 bits.
          read_resp_data = dram_read_resp.data(31, 0);
          unsigned int req;
          if (dbg_peek_req.read_nb(req)) {
            // Return the read resp data to host.
            dbg_peek_resp.write(read_resp_data);
            got_read_resp = false;
            state = 0;
          }
        }
      } else {
        unsigned int req;
        if (dbg_peek_req.read_nb(req)) {
          // Simply return empty to host.
          dbg_peek_resp.write(0);
          state = 0;
        }
      }
    }
  }
}

#endif

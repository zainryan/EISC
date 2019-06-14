#ifndef APP_PT_CPP_
#define APP_PT_CPP_
#include <insider_kernel.h>

void app_demo_dbg_peek(ST_Queue<unsigned int> &dbg_peek_req,
                       ST_Queue<unsigned int> &dbg_peek_resp,
                       ST_Queue<Dram_Read_Req> &app_dram_read_req,
                       ST_Queue<Dram_Read_Resp> &app_dram_read_resp,
                       ST_Queue<Dram_Write_Req_Data> &app_dram_write_req_data,
                       ST_Queue<Dram_Write_Req_Apply> &app_dram_write_req_apply,
                       ST_Queue<APP_Data> &app_input_data,
                       ST_Queue<APP_Data> &app_output_data,
                       ST_Queue<unsigned int> &app_input_params) {
  unsigned int sum = 0;

  while (1) {
#pragma HLS pipeline
    UNUSED_APP_DATA(app_input_data, app_output_data);
    UNUSED_APP_INPUT_PARAMS(app_input_params);
    UNUSED_APP_DRAM(app_dram_read_req, app_dram_read_resp,
                    app_dram_write_req_data, app_dram_write_req_apply);

    unsigned int req;
    if (dbg_peek_req.read_nb(req)) {
      sum += req;
      dbg_peek_resp.write(sum);
    }
  }
}

#endif

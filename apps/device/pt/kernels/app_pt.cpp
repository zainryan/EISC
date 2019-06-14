#ifndef APP_PT_CPP_
#define APP_PT_CPP_
#include <insider_kernel.h>

void app_pt(ST_Queue<unsigned int> &dbg_peek_req,
            ST_Queue<unsigned int> &dbg_peek_resp,
            ST_Queue<Dram_Read_Req> &app_dram_read_req,
            ST_Queue<Dram_Read_Resp> &app_dram_read_resp,
            ST_Queue<Dram_Write_Req_Data> &app_dram_write_req_data,
            ST_Queue<Dram_Write_Req_Apply> &app_dram_write_req_apply,
            ST_Queue<APP_Data> &app_input_data,
            ST_Queue<APP_Data> &app_output_data,
            ST_Queue<unsigned int> &app_input_params) {
  while (1) {
#pragma HLS pipeline

    UNUSED_APP_DRAM(app_dram_read_req, app_dram_read_resp,
                    app_dram_write_req_data, app_dram_write_req_apply);
    UNUSED_DBG_PEEK(dbg_peek_req, dbg_peek_resp);

    unsigned int dummy;
    app_input_params.read_nb(dummy);

    APP_Data data;
    if (app_input_data.read_nb(data)) {
      app_output_data.write(data);
    }
  }
}

#endif

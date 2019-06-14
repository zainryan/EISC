#ifndef DRAM_C_D_TO_APP_CPP_
#define DRAM_C_D_TO_APP_CPP_

#include <insider_kernel.h>

void dram_C_D_to_app(ST_Queue<Dram_Read_Req> &dramC_read_req,
                     ST_Queue<Dram_Read_Resp> &dramC_read_resp,
                     ST_Queue<Dram_Write_Req_Data> &dramC_write_req_data,
                     ST_Queue<Dram_Write_Req_Apply> &dramC_write_req_apply,
                     ST_Queue<Dram_Read_Req> &dramD_read_req,
                     ST_Queue<Dram_Read_Resp> &dramD_read_resp,
                     ST_Queue<Dram_Write_Req_Data> &dramD_write_req_data,
                     ST_Queue<Dram_Write_Req_Apply> &dramD_write_req_apply,
                     ST_Queue<Dram_Read_Req> &app_dram_read_req,
                     ST_Queue<Dram_Read_Resp> &app_dram_read_resp,
                     ST_Queue<Dram_Write_Req_Data> &app_dram_write_req_data,
                     ST_Queue<Dram_Write_Req_Apply> &app_dram_write_req_apply) {

  bool valid_dram_read_req = false;
  Dram_Read_Req dram_read_req;
  bool valid_dram_read_resp = false;
  Dram_Read_Resp dram_read_resp;
  bool valid_dram_write_req_data = false;
  Dram_Write_Req_Data dram_write_req_data;
  bool valid_dram_write_req_apply = false;
  Dram_Write_Req_Apply dram_write_req_apply;

  while (1) {
#pragma HLS pipeline
    // Don't use dramD. The following code does nothing but prevents the 
    // interface from being optimized.
    Dram_Read_Req dummy_0;
    Dram_Read_Resp dummy_1;
    Dram_Write_Req_Data dummy_2;
    Dram_Write_Req_Apply dummy_3;
    if (dramD_read_resp.read_nb(dummy_1)) {
      dramD_read_req.write_nb(dummy_0);
      dramD_write_req_data.write_nb(dummy_2);
      dramD_write_req_apply.write_nb(dummy_3);
    }

    // Connect dramC with app_dram.
    // Limit the addressing space.
    if (valid_dram_read_req ||
        (valid_dram_read_req = app_dram_read_req.read_nb(dram_read_req))) {
      dram_read_req.addr &= (APP_DRAM_ADDRESSING_SPACE - 1);
      if (dramC_read_req.write_nb(dram_read_req)) {
        valid_dram_read_req = false;
      }
    }

    if (valid_dram_read_resp ||
        (valid_dram_read_resp = dramC_read_resp.read_nb(dram_read_resp))) {
      if (app_dram_read_resp.write_nb(dram_read_resp)) {
        valid_dram_read_resp = false;
      }
    }

    if (valid_dram_write_req_data ||
        (valid_dram_write_req_data =
             app_dram_write_req_data.read_nb(dram_write_req_data))) {
      if (dramC_write_req_data.write_nb(dram_write_req_data)) {
        valid_dram_write_req_data = false;
      }
    }

    if (valid_dram_write_req_apply ||
        (valid_dram_write_req_apply =
             app_dram_write_req_apply.read_nb(dram_write_req_apply))) {
      dram_write_req_apply.addr &= (APP_DRAM_ADDRESSING_SPACE - 1);
      if (dramC_write_req_apply.write_nb(dram_write_req_apply)) {
        valid_dram_write_req_apply = false;
      }
    }
  }
}

#endif

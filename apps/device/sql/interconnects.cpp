#include <insider_itc.h>

#include "constant.h"
#include "structure.h"

#include "app_sql_filter0.cpp"
#include "app_sql_filter1.cpp"
#include "app_sql_input_parser.cpp"
#include "app_sql_writer.cpp"

// describe the interconnection
void interconnects() {
  ST_Queue<SQL_Record> app_sql_filter0_input_records(4);
  ST_Queue<SQL_Record> app_sql_filter1_input_records(4);
  ST_Queue<SQL_Record> app_sql_filter0_output_records(4);
  ST_Queue<SQL_Record> app_sql_filter1_output_records(4);
  ST_Queue<Filter_Params> app_sql_filter0_params(4);
  ST_Queue<Filter_Params> app_sql_filter1_params(4);

  app_sql_input_parser(dbg_peek_req, dbg_peek_resp, app_dram_read_req,
                       app_dram_read_resp, app_dram_write_req_data,
                       app_dram_write_req_apply, app_input_data,
                       app_input_params, app_sql_filter0_input_records,
                       app_sql_filter1_input_records, app_sql_filter0_params,
                       app_sql_filter1_params);
  app_sql_filter0(app_sql_filter0_params, app_sql_filter0_input_records,
                  app_sql_filter0_output_records);
  app_sql_filter1(app_sql_filter1_params, app_sql_filter1_input_records,
                  app_sql_filter1_output_records);
  app_sql_writer(app_sql_filter0_output_records, app_sql_filter1_output_records,
                 app_output_data);
}

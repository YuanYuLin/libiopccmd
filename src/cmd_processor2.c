#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "ops_mq.h"
#include "ops_db.h"
#include "cmd_processor.h"

uint8_t CMD(sys_shcmd)(uint8_t* req_data, uint8_t* res_data)
{
	return run_new_shell(req_data, res_data);
}

uint8_t CMD(sys_shcmd_status)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_json_t* json = get_json_instance();
	//struct ops_log_t* log = get_log_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	//uint8_t* ops = json->get_json_string(reader, "ops", "get_status");
	uint8_t status_id = (uint8_t)json->get_json_int(reader, "status_id", 0xFF);
	uint8_t status = get_status(status_id);
	const uint8_t* tmpl = "{\"ops\":\"get_status\", \"status_id\":%d, \"status\":%d}";
	sprintf(res_data, tmpl, status_id, status);
	return CMD_STATUS_NORMAL;
}

uint8_t CMD(sys_dao_get)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	//uint8_t* ops = json->get_json_string(reader, "ops", "");
	uint8_t* key = json->get_json_string(reader, KV_KEY, "");
	uint16_t val_size = 0;
	//uint8_t* val = NULL;
	struct ops_db_t* db = get_db_instance();
	val_size = db->get_val(key, res_data);
	log->debug(0x01, "key: %s, val[%d]: %s\n", key, val_size, res_data);
	return CMD_STATUS_NORMAL;
}


uint8_t CMD(sys_dao_set)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	//uint8_t* ops = json->get_json_string(reader, "ops", "");
	uint8_t* key = json->get_json_string(reader, KV_KEY, "");
	uint8_t* val = json->get_json_string(reader, KV_VAL, "");
	uint16_t val_size = 0;
	struct ops_db_t* db = get_db_instance();
	log->debug(0x1, "%d:%s\n", __LINE__, req_data);
	log->debug(0x1, "%s-key: %s\n", __func__, key);
	log->debug(0x1, "%s-val: %s\n", __func__, val);
	val_size = db->set_val(key, val);
	log->debug(0x1, "%s-val size: %d\n", __func__, val_size);
	return CMD_STATUS_NORMAL;
}

#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "ops_mq.h"
#include "ops_db.h"
#include "ops_rfb.h"
#include "cmd_processor.h"

uint8_t CMD(sys_shcmd)(uint8_t* req_data, uint8_t* res_data)
{
	return run_new_shell(req_data, res_data);
}

uint8_t CMD(rfb_daemon)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_mq_t* mq = get_mq_instance();
	log->debug(0x01, "AAA %s\n", req_data);
	json_reader_t* reader = json->create_json_reader(req_data);
	uint8_t index = (uint8_t)json->get_json_int(reader, "index", 0);
	uint8_t action = (uint8_t)json->get_json_int(reader, "action", 0);
	struct queue_msg_t queue_req;
	struct queue_msg_t queue_res;
	memset(&queue_req, 0, sizeof(struct queue_msg_t));
	memset(&queue_res, 0, sizeof(struct queue_msg_t));
	struct msg_t* req = &queue_req.msg;
	struct msg_t* res = &queue_res.msg;
	req->data_size = sizeof(struct req_rfb_msg_t);
	struct req_rfb_msg_t *req_rfb_msg = (struct req_rfb_msg_t*)&req->data;
	struct res_rfb_msg_t *res_rfb_msg = (struct res_rfb_msg_t*)&res->data;
	req_rfb_msg->index = index;
	req_rfb_msg->action = action;
	queue_req.index = 0;
	queue_req.magic = 0;

	log->debug(0x01, "BBB %d, %d\n", index, action);
	strcpy(queue_req.src, QUEUE_NAME_RFBCLIENT);
	strcpy(queue_req.dst, QUEUE_NAME_RFBSERVER);
	mq->set_to(QUEUE_NAME_RFBSERVER, &queue_req);
	mq->get_from(QUEUE_NAME_RFBCLIENT, &queue_res);
	const uint8_t* tmpl = "{\"status\":\"%d\", \"ops\":\"rfb\", \"index\":%d, \"action\":%d}";
	sprintf(res_data, tmpl, res_rfb_msg->status, res_rfb_msg->index, res_rfb_msg->action);

	return CMD_STATUS_NORMAL;
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

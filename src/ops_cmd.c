
#include "ops_mq.h"
#include "ops_log.h"
#include "ops_cmd.h"
#include "cmd_processor.h"

static struct cmd_processor_t processor_list[] = {
	{ CMD_FN_2,	CMD_NO_4,	CMD(sys_dao_get) },
	{ CMD_FN_2,	CMD_NO_5,	CMD(sys_dao_set) },
	{ CMD_FN_2,	CMD_NO_1,	CMD(sys_shcmd) },
	//{ CMD_FN_2,	CMD_NO_2,	CMD(sys_dao) },
	{ CMD_FN_2,	CMD_NO_3,	CMD(sys_shcmd_status) },

	{ CMD_FN_1,	CMD_NO_1,	CMD(get_gpio) },
	{ CMD_FN_1,	CMD_NO_2,	CMD(put_gpio) },
	{ CMD_FN_1,	CMD_NO_3,	CMD(get_gpio_list) },

	{ CMD_FN_RSVD, 	CMD_NO_RSVD,	NULL }
};

static void init(void)
{
}

static void show_all(void)
{
}

static uint8_t process(struct msg_t* req, struct msg_t* res)
{
	uint8_t cmd_found = 0;
	uint8_t cmd_status = CMD_STATUS_ERR_UNKNOW;
	int i = 0;
	uint32_t processor_size = sizeof(processor_list)/sizeof(struct cmd_processor_t);
	struct cmd_processor_t* cmd_proc = NULL;
	struct ops_log_t* log = get_log_instance();
	for(i = 0;i < processor_size;i++) {
		cmd_proc = &processor_list[i];
		if((cmd_proc->fn == req->fn) && (cmd_proc->cmd == req->cmd)) {
			cmd_found = 1;
			break;
		}
	}

	if(cmd_found) {
		log->debug(0x01, "%s-%s-%d:cmd[%x:%x] found\n", __FILE__, __func__, __LINE__, req->fn, req->cmd);
		if(cmd_proc->processor)
			cmd_status = cmd_proc->processor(req->data, res->data);
	} else {
		log->debug(0x01, "%s-%s-%d:cmd[%x:%x] NOT found\n", __FILE__, __func__, __LINE__,req->fn, req->cmd);
		cmd_status = CMD_STATUS_NOT_FOUND;
	}

	res->fn = 0x80 | req->fn;
	res->cmd = req->cmd;
	res->status = cmd_status;
	res->data_size = strlen(res->data);
	res->crc32 = 0;
	return cmd_status;
}

//static uint8_t exec_shell(int16_t index, uint8_t* json) 
//{
	//_exec_shell(index, json);
//}

static struct ops_cmd_t *obj;
struct ops_cmd_t *get_cmd_instance()
{
	if (!obj) {
		obj = malloc(sizeof(struct ops_cmd_t));
		obj->init = init;
		obj->show_all = show_all;
		obj->process = process;
		//obj->exec_shell = exec_shell;
	}

	return obj;
}

void del_cmd_instance()
{
	if (obj)
		free(obj);
}

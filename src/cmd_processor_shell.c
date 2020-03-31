#include <pthread.h>
#include <sys/stat.h>
#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "ops_db.h"
#include "cmd_processor.h"
#include "shell_common.h"
#include "shell_lxc.h"
#include "shell_qemu.h"
#include "shell_samba.h"
#include "shell_drbd.h"
#include "shell_ssh.h"
#include "shell_netifc.h"
#include "shell_mount.h"
#include "shell_misc.h"
#include "shell_debug.h"

static struct syscmd_t list[] = {
	SHELL_CMD_NETIFC
	SHELL_CMD_STORAGE
	SHELL_CMD_DRBD
	SHELL_CMD_SSH
	SHELL_CMD_LXC
	SHELL_CMD_MISC
	SHELL_CMD_QEMU
	SHELL_CMD_SAMBA

	SHELL_CMD_DEBUG
	SHELL_CMD_END
};

uint8_t run_new_shell(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct syscmd_t* cmd = NULL;
	json_reader_t* reader = json->create_json_reader(req_data);
	int list_size = sizeof(list)/sizeof(struct syscmd_t);
	uint8_t* ops = json->get_json_string(reader, "ops", "");
	uint8_t i = 0;
	uint8_t is_found = 0;

	log->debug(0x01, __FILE__, __func__, __LINE__, "%s, [%d]%s", req_data, strlen(ops), ops);
	for(i=0;i<list_size;i++) {
		cmd = &list[i];
		log->debug(0x01, __FILE__, __func__, __LINE__, "[%d]%s", strlen(cmd->cmd), cmd->cmd);
		if( (strcmp(cmd->cmd, ops) == 0) ) {
			is_found = 1;
			break;
		}
	}
	if(is_found) {
		//log->debug(0xFF, __FILE__, __func__, __LINE__, "cmd begin [%d]: %s", cmd->status_id, cmd->json_param);
		//memset(&cmd->json_param[0], 0, strlen(req_data));
		//memcpy(&cmd->json_param[0], req_data, strlen(req_data));
		//cmd->syscmd_func(cmd->status_id, &cmd->json_param[0]);
		cmd->syscmd_func(cmd->status_id, req_data);
	}
	return CMD_STATUS_NOT_FOUND;
}


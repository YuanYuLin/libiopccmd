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
#include "shell_drbd.h"
#include "shell_ssh.h"
#include "shell_netifc.h"
#include "shell_mount.h"
#include "shell_misc.h"

#if 0
static int none_ifc(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    ifc = json->get_json_string(reader, "ifc", "");
    sprintf(cmd, "ifconfig %s 0.0.0.0", ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int dhcp_ifc(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    ifc = json->get_json_string(reader, "ifc", "");
    sprintf(cmd, "udhcpc -b -n -p /var/run/udhcpc.%s.pid -i %s", ifc, ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int static_ifc(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    //uint8_t ip_type = 4;
    uint8_t* address = NULL;
    uint8_t* netmask = NULL;
    json_reader_t* reader = json->create_json_reader(args);
    //uint8_t* gateway = NULL;
    //uint8_t* dns1 = NULL;
    //uint8_t* dns2 = NULL;
    //uint8_t* dns3 = NULL;
    memset(&cmd[0], 0, CMDLEN);
    ifc = json->get_json_string(reader, "ifc", "");
    //ip_type = json->get_json_int(reader, "ip_type", 4);
    address = json->get_json_string(reader, "address", "0.0.0.0");
    netmask = json->get_json_string(reader, "netmask", "255.255.255.0");
    //gateway = json->get_json_string(reader, "gateway", "0.0.0.0");
    //dns1 = json->get_json_string(reader, "dns1", "0.0.0.0");
    //dns2 = json->get_json_string(reader, "dns2", "0.0.0.0");
    //dns3 = json->get_json_string(reader, "dns3", "0.0.0.0");

    sprintf(cmd, "ifconfig %s %s netmask %s", ifc, address, netmask);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

/*
 * 1. "add_bridge"
 * {
 * "ops":"add_bridge",
 * "bridge":"br0"
 * }
 * 2. "del_bridge"
 * {
 * "ops":"del_bridge",
 * "bridge":"br0"
 * }
 * 3. "add_ifc"
 * {
 * "ops":"add_ifc",
 * "bridge":"br0",
 * "ifc":"eth0"
 * }
 * 4. "del_ifc"
 * {
 * "ops":"del_ifc",
 * "bridge":"br0",
 * "ifc":"eth0"
 * }
 */

static int add_bridge(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* bridge = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    bridge = json->get_json_string(reader, "bridge", "");
    sprintf(cmd, "brctl addbr %s", bridge);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int del_bridge(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* bridge = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    bridge = json->get_json_string(reader, "bridge", "");
    sprintf(cmd, "brctl delbr %s", bridge);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int add_brifc(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* bridge = NULL;
    uint8_t* ifc = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    bridge = json->get_json_string(reader, "bridge", "");
    ifc = json->get_json_string(reader, "ifc", "");
    sprintf(cmd, "brctl addif %s %s", bridge, ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int del_brifc(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* bridge = NULL;
    uint8_t* ifc = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    bridge = json->get_json_string(reader, "bridge", "");
    ifc = json->get_json_string(reader, "ifc", "");
    sprintf(cmd, "brctl delif %s %s", bridge, ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

/*
 * 1. "add_valn"
 * {
 * "ops":"add_vlan",
 * "ifc":"eth0",
 * "tag":100
 * }
 * 2. "del_vlan"
 * {
 * "ops":"del_valn",
 * "ifc":"eth0",
 * "tag":"100",
 * }
 */
static int add_vlan(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    int tag = 0;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    ifc = json->get_json_string(reader, "ifc", "");
    tag = json->get_json_int(reader, "tag", 0);
    sprintf(cmd, "vconfig add %s %d", ifc, tag);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}

static int del_vlan(uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    int tag = 0;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    ifc = json->get_json_string(reader, "ifc", "");
    tag = json->get_json_int(reader, "tag", 0);
    sprintf(cmd, "vconfig rem %s.%d", ifc, tag);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}
#endif

static struct syscmd_t list[] = {
	SHELL_CMD_NETIFC
	SHELL_CMD_STORAGE
	SHELL_CMD_DRBD
	SHELL_CMD_SSH
	SHELL_CMD_LXC
	SHELL_CMD_MISC
	SHELL_CMD_QEMU

	SHELL_CMD_END
};

static void* exec_shell(void* arg)
{
	//uint8_t idx = *((uint8_t*)arg);
	struct syscmd_t* cmd = (struct syscmd_t*)arg;
	struct ops_log_t* log = get_log_instance();
	log->debug(0x01, "[%s, %s-%d] cmd begin : %s\n", __FILE__, __func__, __LINE__, cmd->json_param);
	if(cmd->syscmd_func) {
		log->debug(0x01, "[%s, %s-%d] cmd running\n", __FILE__, __func__, __LINE__);
		cmd->syscmd_func(cmd->status_id, &cmd->json_param[0]);
	}
	log->debug(0x01, "[%s, %s-%d] cmd end\n", __FILE__, __func__, __LINE__);
	return NULL;
}

uint8_t run_new_shell(uint8_t* req_data, uint8_t* res_data)
{
	//struct queue_msg_t qreq;
	//struct msg_t* req = &qreq.msg;
	struct ops_log_t* log = get_log_instance();
	//struct ops_mq_t* mq = get_mq_instance();
	struct ops_json_t* json = get_json_instance();
	struct syscmd_t* cmd = NULL;
	json_reader_t* reader = json->create_json_reader(req_data);
	int list_size = sizeof(list)/sizeof(struct syscmd_t);
	uint8_t* ops = json->get_json_string(reader, "ops", "");
	uint8_t i = 0;
	uint8_t is_found = 0;

	log->debug(0x01, "[%s, %s-%d] %s, %s\n", __FILE__, __func__, __LINE__, req_data, ops);
	for(i=0;i<list_size;i++) {
		cmd = &list[i];
		log->debug(0x01, "[%s, %s-%d] %s, %d\n", __FILE__, __func__, __LINE__, cmd->cmd, strcmp(cmd->cmd, ops));
		if( (strcmp(cmd->cmd, ops) == 0) ) {
			log->debug(0x01, "[%s, %s-%d] %s\n", __FILE__, __func__, __LINE__, ops);
			is_found = 1;
			break;
		}
	}
	if(is_found) {
		//if(cmd->dao_hook_syscmd_func)
		//	cmd->dao_hook_syscmd_func(req_data, res_data);
		set_status_waiting(cmd->status_id);
		//memset(&qreq, 0, sizeof(struct queue_msg_t));
		//req->data_size = strlen(req_data);
		//memcpy(&req->data, req_data, req->data_size);
		//req->fn = 0;
		//req->cmd = i;
		//if(0) {
		//	mq->set_to(QUEUE_NAME_SYSCMD, &qreq);
		//} else {
			memcpy(&cmd->json_param[0], req_data, strlen(req_data));
			log->debug(0x01, "[%s, %s-%d] pthread begin %s\n", __FILE__, __func__, __LINE__, cmd->json_param);
			pthread_create(&cmd->pid, NULL, &exec_shell, (void*)cmd);
			log->debug(0x01, "[%s, %s-%d] pthread  end %s\n", __FILE__, __func__, __LINE__, ops);
		//}
		return CMD_STATUS_NORMAL;
	}
	return CMD_STATUS_NOT_FOUND;
}
#if 0
uint8_t _exec_shell(int8_t index, uint8_t* json_param)
{
	int list_size = sizeof(list)/sizeof(struct syscmd_t);
	struct syscmd_t* cmd = NULL;
	if((index >= list_size) || (index < 0)) 
		return 1;
	cmd = &list[index];
	cmd->syscmd_func(cmd->status_id, json_param);
	return 0;
}
#endif

#ifdef SUPPORT_LXC
#include <lxccontainer.h>
#include "shell_common.h"

static int parse_args(uint8_t *args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	log->debug(0x01, "[%s-%s-%d] %s\n", __FILE__, __func__, __LINE__, args);
	json_reader_t* reader = json->create_json_reader(args);
	uint8_t cfg_idx = json->get_json_int(reader, "index", 0);
	return cfg_idx;
}

static int get_lxc_item(uint8_t cfg_idx, uint8_t *val)
{
	struct ops_db_t* db = get_db_instance();
	struct ops_log_t* log = get_log_instance();
	uint8_t db_key[10];
	memset(&db_key[0], 0, sizeof(db_key));
	sprintf(db_key, "lxc_%d", cfg_idx);
	log->debug(0x01, "[%s-%s-%d] %s\n", __FILE__, __func__, __LINE__, db_key);
	db->get_val(db_key, val);
	log->debug(0x01, "[%s-%s-%d] %s\n", __FILE__, __func__, __LINE__, val);
	return strlen(val);
}

static int save_lxc_cfg(uint8_t *vm_name, uint8_t *vm_rootfs, uint8_t *vm_fstab, uint8_t *vm_nettype, 
		        uint8_t *vm_nethwlink, uint8_t *vm_nethwaddr, uint8_t *vm_ipaddress, uint8_t *vm_gateway)
{
	struct ops_log_t* log = get_log_instance();
	struct lxc_container *ctx = NULL;
	log->debug(0x01, "[%s-%s-%d] %s, %s, %s, %s, %s, %s, %s, %s\n", __FILE__, __func__, __LINE__, vm_name, vm_rootfs, vm_fstab, vm_nettype, vm_nethwlink, vm_nethwaddr, vm_ipaddress, vm_gateway);

	mkdir(vm_rootfs, 0755);
        ctx = (struct lxc_container*)lxc_container_new(vm_name, "/var/lib/lxc");

	if(ctx) {
		log->debug(0x01, "ctx[%s] created\n", vm_name);
		if(ctx->is_defined(ctx)) {
			printf("1.vm thought it was defined\n");
			fprintf(stderr, "1.vm thought it was defined\n");
			return 1;
		}

		ctx->clear_config(ctx);
		ctx->set_config_item(ctx, "lxc.utsname", vm_name);
		ctx->set_config_item(ctx, "lxc.rootfs", vm_rootfs);
		ctx->set_config_item(ctx, "lxc.pts", "256");
		ctx->set_config_item(ctx, "lxc.autodev", "1");
		ctx->set_config_item(ctx, "lxc.init_cmd", "/init");
		ctx->set_config_item(ctx, "lxc.mount", vm_fstab);

		//network interface eth0
		ctx->set_config_item(ctx, "lxc.network.type", vm_nettype);
		ctx->set_config_item(ctx, "lxc.network.flags", "up");
		ctx->set_config_item(ctx, "lxc.network.link", vm_nethwlink);
		ctx->set_config_item(ctx, "lxc.network.name", "eth0");
		ctx->set_config_item(ctx, "lxc.network.hwaddr", vm_nethwaddr);

		//network interface eth1
		ctx->set_config_item(ctx, "lxc.network.type", "veth");
		ctx->set_config_item(ctx, "lxc.network.flags", "up");
		ctx->set_config_item(ctx, "lxc.network.link", "br1");
		ctx->set_config_item(ctx, "lxc.network.name", "eth1");
		ctx->set_config_item(ctx, "lxc.network.ipv4", vm_ipaddress);
		ctx->set_config_item(ctx, "lxc.network.ipv4.gateway", vm_gateway);

		ctx->save_config(ctx, NULL);
		printf("vm fstab cfg %s\n", vm_fstab);
		printf("save config[%s]\n", vm_name);

	} else {
		log->debug(0x01, "ctx failed to create\n");
		fprintf(stderr, "1.Failed to setup lxc_container struct\n");
	}

	return 0;
}

int gen_lxc_cfg(uint8_t status_id, uint8_t* args)
{
	uint8_t cfg_idx = parse_args(args);
	struct ops_json_t* json = get_json_instance();
	uint8_t db_val[DBVALLEN];
	memset(&db_val[0], 0, DBVALLEN);
	get_lxc_item(cfg_idx, &db_val[0]);

	json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
	uint8_t* vm_name = json->get_json_string(db_reader, "name", "");
	uint8_t* vm_rootfs = json->get_json_string(db_reader, "rootfs", "");
	uint8_t* vm_fstab = json->get_json_string(db_reader, "fstab", "");
	uint8_t* vm_nettype = json->get_json_string(db_reader, "nettype", "");
	uint8_t* vm_nethwlink = json->get_json_string(db_reader, "nethwlink", "");
	uint8_t* vm_nethwaddr = json->get_json_string(db_reader, "nethwaddr", "");
	uint8_t* vm_ipaddress = json->get_json_string(db_reader, "ipaddress", "");
	uint8_t* vm_gateway = json->get_json_string(db_reader, "gateway", "");
	
	save_lxc_cfg(vm_name, vm_rootfs, vm_fstab, vm_nettype, vm_nethwlink, vm_nethwaddr, vm_ipaddress, vm_gateway);

	return 0;
}

static int run_lxc(uint8_t *vm_name)
{
        struct ops_misc_t* misc = get_misc_instance();
	struct ops_log_t* log = get_log_instance();
        uint8_t cmd[CMDLEN] = {0};
        memset(&cmd[0], 0, CMDLEN);
        sprintf(cmd, "lxc-start -n %s -f /var/lib/lxc/%s/config -d", vm_name, vm_name);
        log->debug(0x01, "%s - %s\n", __func__, cmd);
        misc->syscmd(cmd);

	return 0;
}

int start_lxc(uint8_t status_id, uint8_t* args)
{
	uint8_t cfg_idx = parse_args(args);
	struct ops_json_t* json = get_json_instance();
	uint8_t db_val[DBVALLEN];
	memset(&db_val[0], 0, DBVALLEN);

	get_lxc_item(cfg_idx, &db_val[0]);
	json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
	uint8_t* vm_name = json->get_json_string(db_reader, "name", "");
	
	run_lxc(vm_name);
	return 0;
}
#endif

#include <pthread.h>

#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "ops_db.h"
#include "cmd_processor.h"

struct syscmd_t {
        uint8_t cmd[CMD_NAME_LEN];
	uint8_t json_param[DBVALLEN];
	pthread_t pid;
	uint8_t status_id;
	int (*syscmd_func)(uint8_t status_id, uint8_t* json);
} __attribute__ ((packed));

struct syscmd_status_t {
	uint8_t status;
};

#define ID_STATUS_UNSPEC	0xFF
#define ID_STATUS_NETIFC	0x00
#define ID_STATUS_DRBD		0x01
#define ID_STATUS_SSH		0x02

#define STATUS_SSH_GENKEY_FINISHED	0x01
#define STATUS_SSH_RUNNING		0x02

#define STATUS_STOP		0x00
#define STATUS_ERROR		0x40
#define STATUS_WAITING		0x80

static struct syscmd_status_t syscmd_status[0xFF] = {
	0x40
};

uint8_t get_status(uint8_t id)
{
	if(id == ID_STATUS_UNSPEC) 
		return STATUS_ERROR;

	struct syscmd_status_t *obj = NULL;
	obj = &syscmd_status[id];
	return obj->status;
}

static uint8_t _set_status(uint8_t id, uint8_t status)
{
	struct syscmd_status_t *obj = NULL;
	obj = &syscmd_status[id];
	obj->status = status;
	return obj->status;
}

static uint8_t _set_status_stop(uint8_t id)
{
	return _set_status(id, STATUS_STOP);
}

static uint8_t _set_status_waiting(uint8_t id)
{
	uint8_t status = get_status(id);
	return _set_status(id, STATUS_WAITING | status);
}

uint8_t get_status_drbd()
{
	return get_status(ID_STATUS_DRBD);
}

/*
 * 1. "status_ifc"
 * {
 * "ops":"status_ifc",
 * }
 * 2. "up_ifc"
 * {
 * "ops":"up_ifc",
 * "ifc":"br0",
 * }
 * 3. "down_ifc",
 * {
 * "ops":"down_static",
 * "ifc":"br0"
 * }
 * 4. "none_ifc"
 * {
 * "ops":"none_ifc",
 * "ifc":"br0"
 * }
 * 5. "dhcp_ifc"
 * {
 * "ops":"dhcp_ifc",
 * "ifc":"br0"
 * }
 * 6. "static_ifc"
 * {
 * "ops":"static_ifc",
 * "ifc":"br0",
 * "ip_type":4,  // 4/6
 * "address":"192.168.1.1",
 * "netmask":"255.255.255.0",
 * "gateway":"192.168.1.254",
 * "dns1":"4.4.4.4",
 * "dns2":"8.8.8.8",
 * "dns3":"8.8.4.4"
 * }
 */
static int up_ifc(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    struct ops_db_t* db = get_db_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t db_val[DBVALLEN];
    uint8_t db_val2[DBVALLEN];
    uint8_t db_val3[DBVALLEN];
    uint8_t net_ifc_name[8];
    uint8_t* ifc = NULL;
    int count = 0;
    uint8_t* str_ptr = NULL;
    int i = 0;
    int x = 0;
    uint8_t* net_type = NULL;
    uint8_t* net_name = NULL;
    uint8_t net_vlan = 0;
    int net_tag = 0;
    uint8_t* net_src = NULL;
    uint8_t* net_address = NULL;
    uint8_t* net_netmask = NULL;
    memset(&cmd[0], 0, CMDLEN);
    memset(&db_val[0], 0, DBVALLEN);
    memset(&db_val2[0], 0, DBVALLEN);
    memset(&db_val3[0], 0, DBVALLEN);
    memset(&net_ifc_name[0], 0, sizeof(net_ifc_name));
    db->get_val("netifc_count", &db_val[0]);
    json_reader_t* reader = json->create_json_reader(args);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    ifc = json->get_json_string(reader, "ifc", "");
    count = json->get_json_array_count(db_reader);
    sprintf(cmd, "ifconfig %s up", ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    log->debug(0x01, "%s - %d\n", __func__, count);
    for(i=0;i<count;i++){
	    str_ptr = json->get_json_array_string_by_index(db_reader, i, "");
	    log->debug(0x01, "%s - %s\n", __func__, str_ptr);
	    db->get_val(str_ptr, &db_val2[0]);
	    json_reader_t* net_reader = json->create_json_reader(&db_val2[0]);
    	    json->debug_json(net_reader);
	    net_type = json->get_json_string(net_reader, "type", "");
	    net_name = json->get_json_string(net_reader, "name", "");
	    net_vlan = json->get_json_boolean(net_reader, "vlan", 0);
	    net_tag = json->get_json_int(net_reader, "tag", 0);
	    net_src = json->get_json_string(net_reader, "src", "none");

	    if( (strlen(net_type) == strlen("eth")) && (memcmp(net_type, "eth", strlen("eth")) == 0) ) {
		    if(net_vlan) {
			    sprintf(&net_ifc_name[0], "%s.%d", net_name, net_tag);

			    memset(&cmd[0], 0, CMDLEN);
			    sprintf(cmd, "vconfig add %s %d", net_name, net_tag);
			    misc->syscmd(cmd);

		    } else {
			    sprintf(&net_ifc_name[0], "%s", net_name);
		    }
	    } else if( (strlen(net_type) == strlen("bridge")) && (memcmp(net_type, "bridge", strlen("bridge")) == 0) ) {
		    sprintf(&net_ifc_name[0], "%s", net_name);

		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "brctl addbr %s", net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
		    json_reader_t* net_devices_reader = json->get_json_array(net_reader, "devices", NULL);
		    if(net_devices_reader){
			    int net_devices_count = json->get_json_array_count(net_devices_reader);
			    log->debug(0x01, "%s - %d\n", __func__, net_devices_count);
			    for(x=0;x<net_devices_count;x++) {
				    uint8_t* net_device = json->get_json_array_string_by_index(net_devices_reader, x, "");
				    memset(&cmd[0], 0, CMDLEN);
				    sprintf(cmd, "brctl addif %s %s", net_ifc_name, net_device);
				    log->debug(0x01, "%s - %s\n", __func__, cmd);
				    misc->syscmd(cmd);
			    }
		    }
	    } else {
	    }

    	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "ifconfig %s up", net_ifc_name);
	    log->debug(0x01, "%s - %s\n", __func__, cmd);
	    misc->syscmd(cmd);

	    if( (strlen(net_src) == strlen("none")) && (memcmp(net_src, "none", strlen("none")) == 0) ) {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "ifconfig %s 0.0.0.0", net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else if( (strlen(net_src) == strlen("dhcp")) && (memcmp(net_src, "dhcp", strlen("dhcp")) == 0) ) {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "udhcpc -b -n -p /var/run/udhcpc.%s.pid -i %s", net_ifc_name, net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else if( (strlen(net_src) == strlen("static")) && (memcmp(net_src, "static", strlen("static")) == 0) ) {
		    net_address = json->get_json_string(net_reader, "address", "0.0.0.0");
		    net_netmask = json->get_json_string(net_reader, "netmask", "255.255.255.0");
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "ifconfig %s %s netmask %s", net_ifc_name, net_address, net_netmask);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else if( (strlen(net_src) == strlen("storage")) && (memcmp(net_src, "storage", strlen("storage")) == 0) ) {
		    uint8_t* net_cfg_json = NULL;
		    json_reader_t* net_cfg_reader = NULL;
		    if(db->get_val("ext_net_cfg_json", &db_val3[0]) <= 0){
			    net_cfg_json = json->get_json_string(net_reader, "net_cfg_json", "");
			    log->debug(0x01, "[%s - %d] %s\n", __func__, __LINE__, net_cfg_json);
			    net_cfg_reader = json->create_json_reader_by_file(net_cfg_json);
			    json->out_json_to_bytes(net_cfg_reader, &db_val3[0]);
			    db->set_val("ext_net_cfg_json", &db_val3[0]);
		    } else {
			    net_cfg_reader = json->create_json_reader(&db_val3[0]);
		    }
		    net_address = json->get_json_string(net_cfg_reader, "address", "0.0.0.0");
		    net_netmask = json->get_json_string(net_cfg_reader, "netmask", "255.255.255.0");
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "ifconfig %s %s netmask %s", net_ifc_name, net_address, net_netmask);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else {
	    }
    }

    return 0;
}

static int down_ifc(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t* ifc = NULL;
    memset(&cmd[0], 0, CMDLEN);
    json_reader_t* reader = json->create_json_reader(args);
    ifc = json->get_json_string(reader, "ifc", "");
    sprintf(cmd, "ifconfig %s down", ifc);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);

    return 0;
}
#if 1
static int set_hostname(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    struct ops_db_t* db = get_db_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t db_val[DBVALLEN];
    uint8_t db_val2[DBVALLEN];

    uint8_t* hostname_src = NULL;
    uint8_t* hostname = NULL;
    memset(&cmd[0], 0, CMDLEN);
    memset(&db_val[0], 0, DBVALLEN);
    memset(&db_val2[0], 0, DBVALLEN);
    db->get_val("hostname_cfg", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    hostname_src = json->get_json_string(db_reader, "src", "");
    if( (strlen(hostname_src) == strlen("storage")) && (memcmp(hostname_src, "storage", strlen("storage")) == 0) ) {
	    uint8_t* hostname_cfg_json = NULL;
	    json_reader_t* hostname_cfg_reader = NULL;
	    if(db->get_val("ext_hostname_cfg_json", &db_val2[0]) <= 0){
		    hostname_cfg_json = json->get_json_string(db_reader, "hostname_cfg_json", "");
		    log->debug(0x01, "[%s - %d] %s\n", __func__, __LINE__, hostname_cfg_json);
		    hostname_cfg_reader = json->create_json_reader_by_file(hostname_cfg_json);
		    json->out_json_to_bytes(hostname_cfg_reader, &db_val2[0]);
		    db->set_val("ext_hostname_cfg_json", &db_val2[0]);
	    } else {
		    hostname_cfg_reader = json->create_json_reader(&db_val2[0]);
	    }
	    hostname = json->get_json_string(hostname_cfg_reader, "hostname", "iopc");
    } else {
	    hostname = json->get_json_string(db_reader, "hostname", "iopc");
    }
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "hostname %s", hostname);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
	return 0;
}
#endif
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
/*
 * 1. "mount_hdd"
 * {
 * "ops":"mount_hdd",
 * "src":"/dev/sda1",
 * "dst":"/hdd/sys",
 * "type":"fat"
 * }
 * 2. "umount_hdd"
 */
static int mount_hdd(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_db_t* db = get_db_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t db_val[DBVALLEN];
    uint8_t db_val2[DBVALLEN];
    uint8_t db_val3[DBVALLEN];
    int count = 0;
    int i = 0;
    int x = 0;
    uint8_t* str_ptr = NULL;
    uint8_t* part_type = NULL;
    uint8_t* part_src = NULL;
    uint8_t* part_dst = NULL;
    memset(&cmd[0], 0, CMDLEN);
    memset(&db_val[0], 0, DBVALLEN);
    memset(&db_val2[0], 0, DBVALLEN);
    memset(&db_val3[0], 0, DBVALLEN);
    //json_reader_t* reader = json->create_json_reader(args);
    db->get_val("storage_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    count = json->get_json_array_count(db_reader);
    for(i=0;i<count;i++){
	    str_ptr = json->get_json_array_string_by_index(db_reader, i, "");
	    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, str_ptr);
	    db->get_val(str_ptr, &db_val2[0]);
	    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, db_val2);
	    json_reader_t* storage_reader = json->create_json_reader(&db_val2[0]);
    	    json->debug_json(storage_reader);
	    json_reader_t* storage_partitions_reader = json->get_json_array(storage_reader, "partitions", NULL);
	    if(storage_partitions_reader) {
	    	    log->debug(0x01, "[%s %d] \n", __func__, __LINE__);
	    	    //json->debug_json(storage_partitions_reader);
		    int storage_partitions_count = json->get_json_array_count(storage_partitions_reader);
	    	    log->debug(0x01, "[%s - %d] %d\n", __func__, __LINE__, storage_partitions_count);
		    for(x=0;x<storage_partitions_count;x++) {
			    uint8_t* storage_partition_name = json->get_json_array_string_by_index(storage_partitions_reader, x, "");
			    db->get_val(storage_partition_name, &db_val3[0]);
			    json_reader_t* partition_reader = json->create_json_reader(&db_val3[0]);
	    		    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, db_val3);
			    json->debug_json(partition_reader);
			    part_type = json->get_json_string(partition_reader, "type", "");
			    part_src = json->get_json_string(partition_reader, "src", "");
			    part_dst = json->get_json_string(partition_reader, "dst", "");
	    		    if( (strlen(part_type) == strlen("fat")) && (memcmp(part_type, "fat", strlen("fat")) == 0) ) {
    	    			    memset(&cmd[0], 0, CMDLEN);
				    sprintf(cmd, "mkdir -p %s", part_dst);
				    log->debug(0x01, "%s - %s\n", __func__, cmd);
				    misc->syscmd(cmd);

    	    			    memset(&cmd[0], 0, CMDLEN);
				    sprintf(cmd, "mount -t vfat %s %s", part_src, part_dst);
				    log->debug(0x01, "%s - %s\n", __func__, cmd);
				    misc->syscmd(cmd);
			    }
		    }
	    } else {
	    	    log->debug(0x01, "[%s %d] \n", __func__, __LINE__);
	    }
    }
}

static int umount_hdd(uint8_t status_id, uint8_t* args)
{
}

#define DRBD_CFG "/tmp/drbd.cfg"

static void write_drbd_cfg(uint8_t* host1, uint8_t* drbd1, uint8_t* disk1, uint8_t* ip1, uint8_t* host2, uint8_t* drbd2, uint8_t* disk2, uint8_t* ip2)
{
	FILE* fp = fopen("/tmp/drbd_tmp.cfg", "w");
	if(fp){
		fprintf(fp, "global {\n");
		fprintf(fp, "usage-count no;\n");
		fprintf(fp, "}\n");
		fprintf(fp, "common {\n");
		fprintf(fp, "net {\n");
		fprintf(fp, "verify-alg md5;\n");
		fprintf(fp, "protocol C;\n");
		fprintf(fp, "}\n");
		fprintf(fp, "}\n");
		fprintf(fp, "resource ha {\n");
		fprintf(fp, "on %s {\n", host1);
		fprintf(fp, "device %s;\n", drbd1);
		fprintf(fp, "disk %s;\n", disk1);
		fprintf(fp, "address %s:1234;\n", ip1);
		fprintf(fp, "meta-disk internal;\n");
		fprintf(fp, "}\n");
		fprintf(fp, "on %s {\n", host2);
		fprintf(fp, "device %s;\n", drbd2);
		fprintf(fp, "disk %s;\n", disk2);
		fprintf(fp, "address %s:1234;\n", ip2);
		fprintf(fp, "meta-disk internal;\n");
		fprintf(fp, "}\n");
		fprintf(fp, "}\n");
		fclose(fp);
		rename("/tmp/drbd_tmp.cfg", DRBD_CFG);
	}
}

#define DRBD_SLAVE	0x01
#define DRBD_MASTER	0x02
static int start_drbd(uint8_t status_id, uint8_t* args)
{
    struct ops_db_t* db = get_db_instance();
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t db_val[DBVALLEN];
    memset(&cmd[0], 0, CMDLEN);
    memset(&db_val[0], 0, DBVALLEN);

    db->get_val("drbd_cfg", &db_val[0]);
    json_reader_t* reader = json->create_json_reader(args);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    //uint8_t start_on_boot = json->get_json_boolean(db_reader, "bootup", 0);
    uint8_t is_master = json->get_json_boolean(reader, "is_master", 0);
    uint8_t* mounted_dir = json->get_json_string(reader, "mounted_dir", "/mnt");

    if(1) {
	    uint8_t* hostname_local = json->get_json_string(db_reader, "hostname_local", "");
	    uint8_t* drbd_local = json->get_json_string(db_reader, "drbd_local", "/dev/drbd0");
	    uint8_t* disk_local = json->get_json_string(db_reader, "disk_local", "");
	    uint8_t* ipaddress_local = json->get_json_string(db_reader, "ipaddress_local", "");
	    uint8_t* hostname_remote = json->get_json_string(db_reader, "hostname_remote", "");
	    uint8_t* drbd_remote = json->get_json_string(db_reader, "drbd_remote", "/dev/drbd0");
	    uint8_t* disk_remote = json->get_json_string(db_reader, "disk_remote", "");
	    uint8_t* ipaddress_remote = json->get_json_string(db_reader, "ipaddress_remote", "");
	    write_drbd_cfg(hostname_local, drbd_local, disk_local, ipaddress_local, hostname_remote, drbd_remote, disk_remote, ipaddress_remote);

	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "hostname %s", hostname_local);
	    log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	    misc->syscmd(cmd);

	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "mkdir -p %s", "/usr/local/var/lib/drbd/");
	    log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	    misc->syscmd(cmd);

	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "mkdir -p %s", mounted_dir);
	    log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	    misc->syscmd(cmd);

    	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "drbdadm create-md -c %s -W --force all", DRBD_CFG);
	    misc->syscmd(cmd);

    	    memset(&cmd[0], 0, CMDLEN);
	    sprintf(cmd, "drbdadm up -c %s all", DRBD_CFG);
	    misc->syscmd(cmd);

	    if(is_master) {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "drbdadm primary --force -c %s all", DRBD_CFG);
		    //sprintf(cmd, "drbdadm primary -c %s all", DRBD_CFG);
		    misc->syscmd(cmd);

		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "mount %s %s", drbd_local, mounted_dir);
		    misc->syscmd(cmd);
		    _set_status(status_id, DRBD_MASTER);
	    } else {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "umount %s", mounted_dir);
		    misc->syscmd(cmd);

		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "drbdadm secondary  -c %s all", DRBD_CFG);
		    misc->syscmd(cmd);
		    _set_status(status_id, DRBD_SLAVE);
	    }
    }

	return 0;
}

static int stop_drbd(uint8_t status_id, uint8_t* args)
{
	_set_status_stop(status_id);
	return 0;
}

#define MAGIC_STR	"aa55"
static int reboot_system(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	json_reader_t* reader = json->create_json_reader(args);
	uint8_t* magic = json->get_json_string(reader, "magic", "");

	if(strcmp(magic, MAGIC_STR) == 0) {
		sprintf(cmd, "reboot");
		log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
		misc->syscmd(cmd);
	} else {
		log->debug(0x01, "[%s-%d] magic string error:%s\n", __func__, __LINE__, magic);
	}
	return 0;
}

static int sync_datetime(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	json_reader_t* reader = json->create_json_reader(args);
	uint8_t* ntp_server = json->get_json_string(reader, "ntp_server", "");

	sprintf(cmd, "/usr/sbin/ntpd -p %s -qNn", ntp_server);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);
	return 0;
}

#define SSH_DROPBEAR_KEY_FILE	"/tmp/ssh_dropbear_rsa.key"
#define SSH_OPENSSH_KEY_FILE	"/tmp/ssh_openssh_rsa_key"
#define SSH_DROPBEAR_PID_FILE	"/tmp/ssh_dropbear.pid"
static int gen_ssh_key(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	sprintf(cmd, "/bin/dropbearkey -t rsa -f %s", SSH_DROPBEAR_KEY_FILE);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);

	sprintf(cmd, "/bin/dropbearconvert dropbear openssh %s %s", SSH_DROPBEAR_KEY_FILE, SSH_OPENSSH_KEY_FILE);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);
	_set_status(status_id, STATUS_SSH_GENKEY_FINISHED);

	return 0;
}

static int start_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	if((get_status(status_id) & STATUS_SSH_GENKEY_FINISHED) == STATUS_SSH_GENKEY_FINISHED) {
		sprintf(cmd, "/bin/dropbear -F -s -g -r %s -P %s", SSH_DROPBEAR_KEY_FILE, SSH_DROPBEAR_PID_FILE);
		log->debug(0x01, "[%s-%d] %d - %s\n", __func__, __LINE__, status_id, cmd);
		_set_status(status_id, STATUS_SSH_RUNNING);
		misc->syscmd(cmd);
	}
	return 0;
}

static int stop_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	FILE *fp = NULL;
	uint16_t pid = 0;
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	log->debug(0x1, "%s-%s-%d-%s", __FILE__, __func__, __LINE__, SSH_DROPBEAR_PID_FILE);

	fp = fopen(SSH_DROPBEAR_PID_FILE, "r");
	if(!fp) {
		log->debug(0x01, "[%s-%s-%d] open %s failed\n", __FILE__, __func__, __LINE__, SSH_DROPBEAR_PID_FILE);
		return 0;
	}
	fscanf(fp, "%d", &pid);
	fclose(fp);

	sprintf(cmd, "kill -9 %d", pid);
	log->debug(0x01, "[%s, %s-%d] %s\n", __FILE__, __func__, __LINE__, cmd);
	misc->syscmd(cmd);

	_set_status_stop(status_id);
	return 0;
}

static struct syscmd_t list[] = {
        {"up_ifc", 	"", 0, ID_STATUS_NETIFC, up_ifc},
        {"down_ifc",	"", 0, ID_STATUS_NETIFC, down_ifc},

	{"mount_hdd",	"", 0, ID_STATUS_UNSPEC, mount_hdd},
	{"umount_hdd",	"", 0, ID_STATUS_UNSPEC, umount_hdd},

        {"start_drbd",	"", 0, ID_STATUS_DRBD, start_drbd},
        {"stop_drbd",	"", 0, ID_STATUS_DRBD, stop_drbd},

	{"gen_ssh_key",	"", 0, ID_STATUS_SSH, gen_ssh_key},
	{"start_ssh",	"", 0, ID_STATUS_SSH, start_ssh},
	{"stop_ssh",	"", 0, ID_STATUS_SSH, stop_ssh},

	{"set_hostname", "", 0, ID_STATUS_UNSPEC, set_hostname},
	{"reboot_system", "", 0, ID_STATUS_UNSPEC, reboot_system},
	{"sync_datetime", "", 0, ID_STATUS_UNSPEC, sync_datetime},

        {"", "", 0, ID_STATUS_UNSPEC, NULL}
};

static void* exec_shell(void* arg)
{
	struct syscmd_t* cmd = (struct syscmd_t*)arg;
	cmd->syscmd_func(cmd->status_id, &cmd->json_param[0]);
	return NULL;
}

uint8_t run_new_shell(uint8_t* req_data, uint8_t* res_data)
{
	struct queue_msg_t qreq;
	struct msg_t* req = &qreq.msg;
	//struct ops_log_t* log = get_log_instance();
	//struct ops_mq_t* mq = get_mq_instance();
	struct ops_json_t* json = get_json_instance();
	struct syscmd_t* cmd = NULL;
	json_reader_t* reader = json->create_json_reader(req_data);
	int list_size = sizeof(list)/sizeof(struct syscmd_t);
	uint8_t* ops = json->get_json_string(reader, "ops", "");
	uint8_t i = 0;
	uint8_t is_found = 0;

	for(i=0;i<list_size;i++) {
		cmd = &list[i];
		if( (!strcmp(cmd->cmd, ops)) && (cmd->syscmd_func) ) {
			is_found = 1;
			break;
		}
	}
	if(is_found) {
		//if(cmd->dao_hook_syscmd_func)
		//	cmd->dao_hook_syscmd_func(req_data, res_data);
		_set_status_waiting(cmd->status_id);
		//memset(&qreq, 0, sizeof(struct queue_msg_t));
		//req->data_size = strlen(req_data);
		//memcpy(&req->data, req_data, req->data_size);
		//req->fn = 0;
		//req->cmd = i;
		//if(0) {
		//	mq->set_to(QUEUE_NAME_SYSCMD, &qreq);
		//} else {
			memcpy(cmd->json_param, req_data, req->data_size);
			pthread_create(&cmd->pid, NULL, &exec_shell, cmd);
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

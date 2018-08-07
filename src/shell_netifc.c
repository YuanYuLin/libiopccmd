#include "shell_common.h"
#include "shell_netifc.h"

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
static int get_netifc_count()
{
    uint8_t db_val[DBVALLEN] = {0};
    int count = 0;
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("netifc_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    count = json->get_json_array_count(db_reader);
    return count;
}

static uint8_t* get_netifc_by_index(uint8_t index)
{
    uint8_t db_val[DBVALLEN] = {0};
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    uint8_t *str_ptr = NULL;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("netifc_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    str_ptr = json->get_json_array_string_by_index(db_reader, index, "");
    return str_ptr;
}

static int parse_args(uint8_t* args)
{
    struct ops_json_t* json = get_json_instance();
    json_reader_t* reader = json->create_json_reader(args);
    json->debug_json(reader);
    //uint8_t* ifc = json->get_json_string(reader, "ifc", "");
	return 0;
}

static int up_netifc_by_item(uint8_t *key_item)
{
    struct ops_db_t* db = get_db_instance();
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t db_val[DBVALLEN];
    uint8_t cmd[CMDLEN] = {0};
    uint8_t net_ifc_name[8];
    memset(&db_val[0], 0, DBVALLEN);
    memset(&net_ifc_name[0], 0, sizeof(net_ifc_name));
    memset(&cmd[0], 0, CMDLEN);
    db->get_val(key_item, &db_val[0]);
    json_reader_t* net_reader = json->create_json_reader(&db_val[0]);
    json->debug_json(net_reader);

    uint8_t* net_address = NULL;
    uint8_t* net_netmask = NULL;
    uint8_t *net_type = json->get_json_string(net_reader, "type", "");
    uint8_t *net_name = json->get_json_string(net_reader, "name", "");
    uint8_t net_vlan = json->get_json_boolean(net_reader, "vlan", 0);
    int net_tag = json->get_json_int(net_reader, "tag", 0);
    uint8_t *net_src = json->get_json_string(net_reader, "src", "none");

    //if( (strlen(net_type) == strlen("eth")) && (memcmp(net_type, "eth", strlen("eth")) == 0) ) 
    //else if( (strlen(net_type) == strlen("bridge")) && (memcmp(net_type, "bridge", strlen("bridge")) == 0) ) 
    if(strcmp(net_type, "eth") == 0) {
	    if(net_vlan) {
		    sprintf(&net_ifc_name[0], "%s.%d", net_name, net_tag);

		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "vconfig add %s %d", net_name, net_tag);
		    misc->syscmd(cmd);

	    } else {
		    sprintf(&net_ifc_name[0], "%s", net_name);
	    }
    } else if( strcmp(net_type, "bridge") == 0) {
		    sprintf(&net_ifc_name[0], "%s", net_name);

		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "brctl addbr %s", net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
		    json_reader_t* net_devices_reader = json->get_json_array(net_reader, "devices", NULL);
		    if(net_devices_reader){
			    int net_devices_count = json->get_json_array_count(net_devices_reader);
			    log->debug(0x01, "%s - %d\n", __func__, net_devices_count);
			    for(int x=0;x<net_devices_count;x++) {
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

	    if( strcmp(net_src, "none") == 0) {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "ifconfig %s 0.0.0.0", net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else if( strcmp(net_src, "dhcp") == 0) {
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "udhcpc -b -n -p /var/run/udhcpc.%s.pid -i %s", net_ifc_name, net_ifc_name);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    } else if( strcmp(net_src, "static") == 0) {
		    net_address = json->get_json_string(net_reader, "address", "0.0.0.0");
		    net_netmask = json->get_json_string(net_reader, "netmask", "255.255.255.0");
		    memset(&cmd[0], 0, CMDLEN);
		    sprintf(cmd, "ifconfig %s %s netmask %s", net_ifc_name, net_address, net_netmask);
		    log->debug(0x01, "%s - %s\n", __func__, cmd);
		    misc->syscmd(cmd);
	    }

	return 0;
}

int up_netifc(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    int count = 0;

    parse_args(args);
    count = get_netifc_count();
    for(int i=0;i<count;i++){
	    uint8_t *str_ptr = get_netifc_by_index(i);
	    log->debug(0x01, "%s - %s\n", __func__, str_ptr);
	    up_netifc_by_item(str_ptr);
    }

    return 0;
}

int down_netifc(uint8_t status_id, uint8_t* args)
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


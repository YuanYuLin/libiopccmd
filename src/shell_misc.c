#include "shell_common.h"
#include "shell_misc.h"

static int run_set_hostname(uint8_t* hostname)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "hostname %s", hostname);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
    return 0;
}

int set_hostname(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_db_t* db = get_db_instance();
    uint8_t db_val[DBVALLEN];
    uint8_t db_val2[DBVALLEN];

    uint8_t* hostname_src = NULL;
    uint8_t* hostname = NULL;
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

    run_set_hostname(hostname);
	return 0;
}

static int run_reboot_system()
{
	struct ops_log_t* log = get_log_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	sprintf(cmd, "reboot");
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);
	return 0;
}

#define MAGIC_STR	"aa55"
int reboot_system(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(args);
	uint8_t* magic = json->get_json_string(reader, "magic", "");

	if(strcmp(magic, MAGIC_STR) == 0) {
		run_reboot_system();
	} else {
		log->debug(0x01, "[%s-%d] magic string error:%s\n", __func__, __LINE__, magic);
	}
	return 0;
}

static int run_sync_datetime(uint8_t *ntp_server)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	sprintf(cmd, "/usr/sbin/ntpd -p %s -qNn", ntp_server);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);
	return 0;
}

int sync_datetime(uint8_t status_id, uint8_t* args)
{
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(args);
	uint8_t* ntp_server = json->get_json_string(reader, "ntp_server", "");

	run_sync_datetime(ntp_server);
	return 0;
}


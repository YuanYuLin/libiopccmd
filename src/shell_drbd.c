#include "shell_common.h"

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

static int set_hostname(uint8_t *hostname_local)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "hostname %s", hostname_local);
    log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
    misc->syscmd(cmd);
    return 0;
}

static int create_dir(uint8_t *path)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mkdir -p %s", path);
    log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
    misc->syscmd(cmd);
    return 0;
}

static int init_drbd()
{
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "drbdadm create-md -c %s -W --force all", DRBD_CFG);
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "drbdadm up -c %s all", DRBD_CFG);
    misc->syscmd(cmd);

    return 0;
}

static int run_as_master(uint8_t *drbd_local, uint8_t *mounted_dir)
{
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "drbdadm primary --force -c %s all", DRBD_CFG);
    //sprintf(cmd, "drbdadm primary -c %s all", DRBD_CFG);
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount %s %s", drbd_local, mounted_dir);
    misc->syscmd(cmd);

    return 0;
}

static int run_as_secondary(uint8_t *drbd_local, uint8_t *mounted_dir)
{
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "umount %s", mounted_dir);
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "drbdadm secondary  -c %s all", DRBD_CFG);
    misc->syscmd(cmd);

    return 0;
}

#define DRBD_SLAVE	0x01
#define DRBD_MASTER	0x02
int start_drbd(uint8_t status_id, uint8_t* args)
{
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
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
	    set_hostname(hostname_local);
	    create_dir("/usr/local/var/lib/drbd/");
	    create_dir(mounted_dir);
	    init_drbd();

	    if(is_master) {
		    run_as_master(drbd_local, mounted_dir);
		    set_status(status_id, DRBD_MASTER);
	    } else {
		    run_as_secondary(drbd_local, mounted_dir);
		    set_status(status_id, DRBD_SLAVE);
	    }
    }

	return 0;
}

int stop_drbd(uint8_t status_id, uint8_t* args)
{
	set_status_stop(status_id);
	return 0;
}

uint8_t get_status_drbd()
{
	return get_status(ID_STATUS_DRBD);
}


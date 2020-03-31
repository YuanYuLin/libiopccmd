#include "shell_common.h"
#include "shell_samba.h"

#ifdef SUPPORT_SAMBA

static int get_samba_count()
{
    struct ops_db_t *db = get_db_instance();
    struct ops_json_t *json = get_json_instance();
    uint8_t db_val[DBVALLEN];
    int count = 0;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("samba_count", &db_val[0]);
    json_reader_t *db_reader = json->create_json_reader(&db_val[0]);
    count = json->get_json_array_count(db_reader);
    return count;
}

static uint8_t* get_samba_item_name_by_index(uint8_t index)
{
    uint8_t db_val[DBVALLEN] = {0};
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    uint8_t *str_ptr = NULL;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("samba_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    json_reader_t* array_reader = json->get_json_array_object_by_index(db_reader, index);
    str_ptr = json->get_json_string(array_reader, NULL, "");
    return str_ptr;
}
#define SAMBA_CFG		"/tmp/smb.cfg"
#define PRINTCAP		"/tmp/samba/printcap"
#define SAMBA_PID		"/usr/local/samba/var/run/smbd.pid"
static void gen_samba_cfg(FILE* fp, uint8_t *name, uint8_t *type, uint8_t *path)
{
	if(fp) {
		if(strcmp("global", type) == 0) {
			fprintf(fp, "[global]\n");
			fprintf(fp, "workgroup = MYGROUP\n");
			fprintf(fp, "guest account = root\n");
			fprintf(fp, "security = user\n");
			fprintf(fp, "map to guest = Bad Password\n");
			//fprintf(fp, "dos charset = CP950\n");
			fprintf(fp, "unix charset = UTF8\n");
			//fprintf(fp, "printcap name = %s\n", PRINTCAP);
			//fprintf(fp, "bind interfaces only = yes\n");
			//fprintf(fp, "interfaces = 127.0.0.1 host_ip_address\n");
			fprintf(fp, "\n");
		} else if(strcmp("shrdir", type) == 0) {
			fprintf(fp, "[%s]\n", name);
			fprintf(fp, "comment = Shared Directories\n");
			fprintf(fp, "path = %s\n", path);
			fprintf(fp, "guest ok = yes\n");
			fprintf(fp, "browseable = yes\n");
			fprintf(fp, "writable = yes\n");
			fprintf(fp, "\n");
		}
	}
}

static int create_samba_cfg(FILE* fp, uint8_t idx)
{
    struct ops_log_t *log = get_log_instance();
    struct ops_db_t *db = get_db_instance();
    struct ops_json_t *json = get_json_instance();
    uint8_t db_item[DBVALLEN];
    memset(&db_item[0], 0, DBVALLEN);
    uint8_t *item = get_samba_item_name_by_index(idx);
    db->get_val(item, &db_item[0]);
    json_reader_t *qemu_reader = json->create_json_reader(&db_item[0]);

    int enable = json->get_json_int(qemu_reader, "enable", 0);
    uint8_t *name = json->get_json_string(qemu_reader, "name", "");
    uint8_t *type = json->get_json_string(qemu_reader, "type", "");
    uint8_t *smbpath = json->get_json_string(qemu_reader, "path", "");
    log->debug(0x01, __FILE__, __func__, __LINE__, "en:%d, %s, %s, %s", enable, name, type, smbpath);
    if(enable)
        gen_samba_cfg(fp, name, type, smbpath);
    return enable;
}

static int execute_samba()
{
    #undef L_STRLEN
    #define L_STRLEN	30
    struct ops_shell_t *shell = get_shell_instance();
    uint8_t cmd[L_STRLEN] = { 0 };
    memset(&cmd[0], 0, L_STRLEN);
    snprintf(cmd, L_STRLEN, "smbd -D -s %s", SAMBA_CFG);
    //snprintf(cmd, L_STRLEN, "smbd -i -S -s %s", SAMBA_CFG);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
    return 0;
}

int start_samba(uint8_t status_id, uint8_t * args)
{
    #undef L_STRLEN
    #define L_STRLEN	30
    int samba_count = get_samba_count();
    uint8_t path[L_STRLEN] = { 0 };
    FILE* fp = NULL;
    struct ops_misc_t *misc = get_misc_instance();

    memset(&path[0], 0, L_STRLEN);
    snprintf(path, L_STRLEN, SAMBA_CFG);
    fp = fopen(path, "w");
    for(int idx=0;idx<samba_count;idx++) {
	    create_samba_cfg(fp, idx);
    }
    fclose(fp);

    misc->create_dir_recursive("/tmp/samba", 0755);
    misc->create_dir_recursive("/usr/local/samba/var", 0755);
    misc->create_dir_recursive("/usr/local/samba/private", 0755);

    memset(&path[0], 0, L_STRLEN);
    snprintf(path, L_STRLEN, PRINTCAP);
    fp = fopen(path, "w");
    fclose(fp);

    execute_samba();

    return 0;
}

int stop_samba(uint8_t status_id, uint8_t * args)
{
    #undef L_STRLEN
    #define L_STRLEN	40
	struct ops_log_t* log = get_log_instance();
	struct ops_misc_t* misc = get_misc_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t cmd[L_STRLEN] = {0};
	uint16_t pid = 0;
	memset(&cmd[0], 0, L_STRLEN);

	pid = misc->get_pid_by_path(SAMBA_PID);

	if(pid > 0) {
		snprintf(cmd, L_STRLEN, "kill -9 %d", pid);
		log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
		shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);

		set_status_stop(status_id);
	}
	return 0;
}

#endif

#include "shell_common.h"
#include "shell_mount.h"

static int run_setup_zram(uint8_t zram_idx)
{
    uint8_t cmd[CMDLEN] = {0};
    struct ops_misc_t* misc = get_misc_instance();

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/mkswap /dev/zram%d", zram_idx);
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/swapon /dev/zram%d", zram_idx);
    misc->syscmd(cmd);
    return 0;
}

static void setup_zram(long val_kb, uint8_t zram_idx)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sys/block/zram%d/disksize", zram_idx);
    log->error(0x01, "set %ld to %s\n", val_kb * 1024, cmd);

    FILE *fp = fopen(cmd, "r+");

    if(fp == NULL) {
            log->error(0x01, "Can not open /sys/block/zram%d/disksize\n", zram_idx);
            return ;
    }
    fprintf(fp, "%ld", val_kb * 1024);
    fclose(fp);

    run_setup_zram(zram_idx);
}

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
int format_storage(uint8_t status_id, uint8_t* args)
{
	return 0;
}

static int get_mount_storage_count()
{
    uint8_t db_val[DBVALLEN] = {0};
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    int count = 0;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("storage_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    count = json->get_json_array_count(db_reader);
    return count;
}

static uint8_t* get_mount_storage_item(uint8_t index)
{
    uint8_t db_val[DBVALLEN] = {0};
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    uint8_t *ptr = NULL;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("storage_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    ptr = json->get_json_array_string_by_index(db_reader, index, "");
    return ptr;
}

static int mount_storage_by_item(uint8_t *item_name)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_db_t* db = get_db_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    uint8_t db_val[DBVALLEN] = {0};
    memset(&db_val[0], 0, DBVALLEN);
    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, item_name);
    db->get_val(item_name, &db_val[0]);
    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, db_val);
    json_reader_t* storage_reader = json->create_json_reader(&db_val[0]);
    json->debug_json(storage_reader);
    uint8_t *part_type = json->get_json_string(storage_reader, "type", "");
    uint8_t *part_src = json->get_json_string(storage_reader, "src", "");
    uint8_t *part_dst = json->get_json_string(storage_reader, "dst", "");
    uint8_t swap_device = 0;
    uint32_t swap_size = 0;
    if(strcmp(part_type, "fat") == 0) {
        memset(&cmd[0], 0, CMDLEN);
        sprintf(cmd, "mkdir -p %s", part_dst);
        log->debug(0x01, "%s - %s\n", __func__, cmd);
        misc->syscmd(cmd);

        memset(&cmd[0], 0, CMDLEN);
        sprintf(cmd, "mount -t vfat %s %s", part_src, part_dst);
        log->debug(0x01, "%s - %s\n", __func__, cmd);
        misc->syscmd(cmd);
    }else if(strcmp(part_type, "ext4") == 0) {
        memset(&cmd[0], 0, CMDLEN);
        sprintf(cmd, "mkdir -p %s", part_dst);
        log->debug(0x01, "%s - %s\n", __func__, cmd);
        misc->syscmd(cmd);

        memset(&cmd[0], 0, CMDLEN);
        sprintf(cmd, "mount -t ext4 %s %s", part_src, part_dst);
        log->debug(0x01, "%s - %s\n", __func__, cmd);
        misc->syscmd(cmd);
    }else if(strcmp(part_type, "zram") == 0) {
        swap_size = json->get_json_int(storage_reader, "size_kb", 20480);
        swap_device = json->get_json_int(storage_reader, "device", 0);
        setup_zram(swap_size, swap_device);
    }
}

int mount_storage(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    int count = get_mount_storage_count();
    for(int i=0;i<count;i++){
	    uint8_t *str_ptr = get_mount_storage_item(i);
	    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, str_ptr);
	    mount_storage_by_item(str_ptr);
    }
    return 0;
}

int umount_storage(uint8_t status_id, uint8_t* args)
{
}
/*
static void setup_zram(long val_kb, uint8_t zram_idx)
{
    uint8_t cmd[CMDLEN] = {0};
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sys/block/zram%d/disksize", zram_idx);

    FILE *fp = fopen(cmd, "r+");

    if(fp == NULL) {
            log->error(0x01, "Can not open /sys/block/zram%d/disksize\n", zram_idx);
            return ;
    }
    fprintf(fp, "%ld", val_kb * 1024);
    fclose(fp);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/mkswap /dev/zram%d", zram_idx);
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/swapon /dev/zram%d", zram_idx);
    misc->syscmd(cmd);
}

int mount_swap(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    struct ops_db_t* db = get_db_instance();
    uint8_t db_val[DBVALLEN];
    uint8_t* swap_type = NULL;
    uint8_t* swap_device = NULL;
    long swap_size = 0;

    memset(&db_val[0], 0, DBVALLEN);

    db->get_val("swap_cfg", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    swap_type = json->get_json_string(db_reader, "type", "");
    swap_size = json->get_json_int(db_reader, "size_kb", 20480);
    swap_device = json->get_json_string(db_reader, "device", "");
    log->debug(0x01, "swap type %s, size %d, device %s\n", swap_type, swap_size, swap_device);
    if(strcmp(swap_type, "zram") == 0) {
	    setup_zram(swap_size, (uint8_t)strtol(swap_device, NULL, 10));
    }
}
*/

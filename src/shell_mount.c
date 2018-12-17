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

static int get_devices_string(json_reader_t* devices_reader, int device_count, uint8_t* devices)
{
    struct ops_json_t* json = get_json_instance();
    for(int i=0;i<device_count;i++) {
        uint8_t* device = json->get_json_array_string_by_index(devices_reader, i, "");
        sprintf(devices, "%s %s", devices, device);
    }
    return strlen(devices);
}

static int get_devices_string_option(json_reader_t* devices_reader, int device_count, uint8_t* devices)
{
    struct ops_json_t* json = get_json_instance();
    for(int i=0;i<device_count;i++) {
        uint8_t* device = json->get_json_array_string_by_index(devices_reader, i, "");
	if(i == 0) 
            sprintf(devices, "device=%s", device);
        else
            sprintf(devices, "%s,device=%s", devices, device);
    }
    return strlen(devices);
}

/*
 * {"ops":"format_storage",
 *  "type":"",
 *  "devices":[]
 * }
 */
int format_storage(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t cmd[DBVALLEN] = { 0 };
    uint8_t devices[DBVALLEN] = { 0 };
    uint8_t *json_param = (uint8_t*)args;
    struct ops_json_t* json = get_json_instance();
    struct ops_misc_t* misc = get_misc_instance();
    json_reader_t* reader = json->create_json_reader(json_param);
    uint8_t *part_type = json->get_json_string(reader, "type", "");
    uint8_t *label = json->get_json_string(reader, "label", "");
    memset(&devices[0], DBVALLEN, 0);
    memset(&cmd[0], DBVALLEN, 0);

    json_reader_t* devices_reader = json->get_json_array(reader, "devices", NULL);
    int device_count = json->get_json_array_count(devices_reader);
    if(strcmp(part_type, "btrfs_raid10") == 0) {
        if(device_count < 4) {
            log->debug(0x01, "btrfs_raid10 count[%d] < 4\n", device_count);
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, DBVALLEN, "mkfs.btrfs -d raid10 -m raid10 -f -L %s %s", label, devices);
    } else if(strcmp(part_type, "ext4") == 0) {
        if(device_count != 1) {
            log->debug(0x01, "ext4 count[%d] != 1\n", device_count);
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, DBVALLEN, "mkfs.ext4 -L %s", devices);
    } else {
        log->debug(0x01, "Error : not supported format[%s]\n", part_type);
    }
    printf("%s\n", cmd);
    misc->syscmd(cmd);
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

static int create_mount_point(uint8_t *part_dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mkdir -p %s", part_dst);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
    return 0;
}

static int mount_vfat(uint8_t *src, uint8_t *dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount -t vfat %s %s", src, dst);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
}

static int mount_ext4(uint8_t *src, uint8_t *dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount -t ext4 %s %s", src, dst);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
}

static int mount_btrfs_raid10(uint8_t *src, uint8_t *dst, uint8_t *devices)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_misc_t* misc = get_misc_instance();
    uint8_t cmd[CMDLEN] = {0};

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "btrfs device scan --all-devices");
    misc->syscmd(cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount -t btrfs -o %s %s %s", devices, src, dst);
    log->debug(0x01, "%s - %s\n", __func__, cmd);
    misc->syscmd(cmd);
}

static int mount_storage_by_json_string(uint8_t *json_str)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    json_reader_t* storage_reader = json->create_json_reader(json_str);
    json->debug_json(storage_reader);
    uint8_t enable = json->get_json_int(storage_reader, "enable", 0);
    log->debug(0x01, "[%s-%d] enabled=%d\n", __func__, __LINE__, enable);
    if(enable) {
        uint8_t *part_type = json->get_json_string(storage_reader, "type", "");
        uint8_t *part_src = json->get_json_string(storage_reader, "src", "");
        uint8_t *part_dst = json->get_json_string(storage_reader, "dst", "");
    	log->debug(0x01, "[%s-%d] type=%s,src=%s,dst=%s\n", __func__, __LINE__, part_type, part_src, part_dst);
        if(strcmp(part_type, "fat") == 0) {
            create_mount_point(part_dst);
	    mount_vfat(part_src, part_dst);
        }else if(strcmp(part_type, "ext4") == 0) {
            create_mount_point(part_dst);
	    mount_ext4(part_src, part_dst);
        }else if(strcmp(part_type, "zram") == 0) {
            uint32_t swap_size = json->get_json_int(storage_reader, "size_kb", 20480);
            uint8_t swap_device = json->get_json_int(storage_reader, "device", 0);
            setup_zram(swap_size, swap_device);
        }else if(strcmp(part_type, "btrfs_raid10") == 0) {
            json_reader_t* devices_reader = json->get_json_array(storage_reader, "devices", NULL);
	    int device_count = 0;
	    if(devices_reader)
                device_count = json->get_json_array_count(storage_reader);

            if(device_count == 0) {
		log->error(0x01, "device_count is 0\n");
                return -1;
	    }

	    uint8_t devices[DBVALLEN] = { 0 };
	    memset(&devices[0], 0, DBVALLEN);
            get_devices_string_option(devices_reader, device_count, devices);

            create_mount_point(part_dst);
	    mount_btrfs_raid10(part_src, part_dst, devices);
	}
    } else {
        return -2;
    }
    return 0;
}

static int mount_storage_by_item(uint8_t *item_name)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_db_t* db = get_db_instance();
    uint8_t db_val[DBVALLEN] = {0};
    memset(&db_val[0], 0, DBVALLEN);
    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, item_name);
    db->get_val(item_name, &db_val[0]);
    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, db_val);
    mount_storage_by_json_string(&db_val[0]);
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

int mount_storage_ex(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t *json_param = (uint8_t*)args;
    log->debug(0x01, "[%s %d] - %s\n", __func__, __LINE__, json_param);
    mount_storage_by_json_string(json_param);
}

int umount_storage(uint8_t status_id, uint8_t* args)
{
}

int umount_storage_ex(uint8_t status_id, uint8_t* args)
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

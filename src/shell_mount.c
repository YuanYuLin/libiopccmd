#include "shell_common.h"
#include "shell_mount.h"
#include <time.h>

static int run_setup_zram(uint8_t zram_idx)
{
    uint8_t cmd[CMDLEN] = {0};
    struct ops_shell_t* shell = get_shell_instance();

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/mkswap /dev/zram%d", zram_idx);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sbin/swapon /dev/zram%d", zram_idx);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
    return 0;
}

static void setup_zram(long val_kb, uint8_t zram_idx)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "/sys/block/zram%d/disksize", zram_idx);
    log->debug(0x01, __FILE__, __func__, __LINE__, "set %ld to %s\n", val_kb * 1024, cmd);

    FILE *fp = fopen(cmd, "r+");

    if(fp == NULL) {
            log->error(0xFF, __FILE__, __func__, __LINE__, "Can not open /sys/block/zram%d/disksize\n", zram_idx);
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
        json_reader_t* array_reader = json->get_json_array_object_by_index(devices_reader, i);
        uint8_t* device = json->get_json_string(array_reader, NULL,  "");
        sprintf(devices, "%s %s", devices, device);
    }
    return strlen(devices);
}
#if 0
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
#endif
/*
 * {"ops":"format_storage",
 *  "type":"",
 *  "devices":[]
 * }
 */
int format_storage(uint8_t status_id, uint8_t* args)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t is_error = 0;
    uint8_t cmd[CMDLEN] = { 0 };
    uint8_t devices[CMDLEN] = { 0 };
    log->error(0x01, __FILE__, __func__, __LINE__, "args:%s", args);
    uint8_t *json_param = (uint8_t*)args;
    struct ops_json_t* json = get_json_instance();
    struct ops_shell_t* shell = get_shell_instance();
    json_reader_t* reader = json->create_json_reader(json_param);
    uint8_t *part_type = json->get_json_string(reader, "type", "");
    uint8_t *label = json->get_json_string(reader, "label", "");
    log->error(0x01, __FILE__, __func__, __LINE__, "pt: %s, label:%s", part_type, label);
    memset(&devices[0], CMDLEN, 0);
    memset(&cmd[0], CMDLEN, 0);

    json_reader_t* devices_reader = json->get_json_array(reader, "devices", NULL);
    int device_count = 0;
    if(devices_reader)
        device_count = json->get_json_array_count(devices_reader);
    if(strcmp(part_type, "btrfs") == 0) {
        //Deprecate
        if(device_count < 1) {
            log->error(0x01, __FILE__, __func__, __LINE__, "btrfs count[%d] < 1", device_count);
	    is_error = 1;
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, CMDLEN, "mkfs.btrfs -f -L %s %s", label, devices);
#if 0
    } else if(strcmp(part_type, "btrfs_raid10") == 0) { // Deprecate
        //Deprecate
        if(device_count < 4) {
            log->error(0xFF, __FILE__, __func__, __LINE__, "btrfs_raid10 count[%d] < 4", device_count);
	    is_error = 1;
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, CMDLEN, "mkfs.btrfs -d raid10 -m raid10 -f -L %s %s", label, devices);
#endif
    } else if(strcmp(part_type, "btrfs_raid1") == 0) {
        if(device_count < 2) {
            log->error(0x01, __FILE__, __func__, __LINE__, "btrfs_raid1 count[%d] < 2", device_count);
	    is_error = 1;
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, CMDLEN, "mkfs.btrfs -d raid1 -m raid1 -f -L %s %s", label, devices);
    } else if(strcmp(part_type, "btrfs_raid1_attach_hdd") == 0) {
        if(device_count < 1) {
            log->error(0x01, __FILE__, __func__, __LINE__, "btrfs_attach count[%d] < 1", device_count);
	    is_error = 1;
	}
        uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
        get_devices_string(devices_reader, device_count, devices);
	snprintf(cmd, CMDLEN, "btrfs device add %s %s", devices, mount_dir);
    } else if(strcmp(part_type, "btrfs_raid1_detach_hdd") == 0) {
        if(device_count < 1) {
            log->error(0x01, __FILE__, __func__, __LINE__, "btrfs_attach count[%d] < 1", device_count);
	    is_error = 1;
        }
        uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
        get_devices_string(devices_reader, device_count, devices);
	snprintf(cmd, CMDLEN, "btrfs device delete %s %s", devices, mount_dir);
    } else if(strcmp(part_type, "btrfs_raid1_balance_hdd") == 0) {
        uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
	snprintf(cmd, CMDLEN, "btrfs balance start -dconvert=raid1 -mconvert=raid1 %s", mount_dir);
    } else if(strcmp(part_type, "btrfs_subvolume_create") == 0) {
	uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
	uint8_t *subvolume = json->get_json_string(reader, "subvolume", "");
	snprintf(cmd, CMDLEN, "btrfs subvolume create %s/%s", mount_dir, subvolume);
    } else if(strcmp(part_type, "btrfs_subvolume_delete") == 0) {
	uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
	uint8_t *subvolume = json->get_json_string(reader, "subvolume", "");
	snprintf(cmd, CMDLEN, "btrfs subvolume delete %s/%s", mount_dir, subvolume);
    } else if(strcmp(part_type, "btrfs_subvolume_snapshot") == 0) {
	uint8_t *mount_dir = json->get_json_string(reader, "mount_dir", "");
	uint8_t *subvolume = json->get_json_string(reader, "subvolume", "");
	time_t time_now;
	time(&time_now);
	struct tm *now = gmtime(&time_now);
	snprintf(cmd, CMDLEN, "btrfs subvolume snapshot %s/%s %s/%s.snap.%04d%02d%02d_%02d%02d%02d", mount_dir, subvolume, mount_dir, subvolume, (1900 + now->tm_year), now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
#if 0
    } else if(strcmp(part_type, "ext4") == 0) {
        //Deprecate
        if(device_count != 1) {
            log->error(0xFF, __FILE__, __func__, __LINE__, "ext4 count[%d] != 1", device_count);
	    is_error = 1;
        }
	get_devices_string(devices_reader, device_count, devices);
        snprintf(cmd, CMDLEN, "mkfs.ext4 -L %s", devices);
#endif
    } else {
        log->error(0x01, __FILE__, __func__, __LINE__, "Error : not supported format[%s]", part_type);
        is_error = 1;
    }

    if(is_error) {
	log->debug(0x01, __FILE__, __func__, __LINE__, "cmd: %s", cmd);
    } else {
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
    }
    return 0;
}

int attach_hdd_to_btrfs_raid1()
{
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
    json_reader_t* array_reader = json->get_json_array_object_by_index(db_reader, index);
    ptr = json->get_json_string(array_reader, NULL, "");
    return ptr;
}

static int create_mount_point(uint8_t *part_dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_shell_t* shell = get_shell_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mkdir -p %s", part_dst);
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
    return 0;
}

static int mount_vfat(uint8_t *src, uint8_t *dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_shell_t* shell = get_shell_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount -t vfat %s %s", src, dst);
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
}

static int mount_ext4(uint8_t *src, uint8_t *dst)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_shell_t* shell = get_shell_instance();
    uint8_t cmd[CMDLEN] = {0};
    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "mount -t ext4 %s %s", src, dst);
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
}

static int mount_btrfs_raid(uint8_t *src, uint8_t *dst, uint8_t *devices)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_shell_t* shell = get_shell_instance();
    uint8_t cmd[CMDLEN] = {0};

    memset(&cmd[0], 0, CMDLEN);
    sprintf(cmd, "btrfs device scan --all-devices");
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);

    memset(&cmd[0], 0, CMDLEN);
    if(devices == NULL) {
        sprintf(cmd, "mount -t btrfs %s %s", src, dst);
    } else {
        sprintf(cmd, "mount -t btrfs -o %s %s %s", devices, src, dst);
    }
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
}

static int mount_storage_by_json_string(uint8_t *json_str)
{
    struct ops_log_t* log = get_log_instance();
    struct ops_json_t* json = get_json_instance();
    json_reader_t* storage_reader = json->create_json_reader(json_str);
    json->debug_json(storage_reader);
    uint8_t enable = json->get_json_int(storage_reader, "enable", 0);
    log->debug(0x01, __FILE__, __func__, __LINE__, "enabled=%d", enable);
    if(enable) {
        uint8_t *part_type = json->get_json_string(storage_reader, "type", "");
        uint8_t *part_src = json->get_json_string(storage_reader, "src", "");
        uint8_t *part_dst = json->get_json_string(storage_reader, "dst", "");
    	log->debug(0x01, __FILE__, __func__, __LINE__, "type=%s,src=%s,dst=%s", part_type, part_src, part_dst);
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
        }else if(strcmp(part_type, "btrfs_raid1") == 0) {
#if 0
            json_reader_t* devices_reader = json->get_json_array(storage_reader, "devices", NULL);
	    int device_count = 0;
	    if(devices_reader)
                device_count = json->get_json_array_count(storage_reader);

            if(device_count == 0) {
		log->error(0xFF, __FILE__, __func__, __LINE__, "device_count is 0");
                return -1;
	    }

	    uint8_t devices[DBVALLEN] = { 0 };
	    memset(&devices[0], 0, DBVALLEN);
            get_devices_string_option(devices_reader, device_count, devices);

            create_mount_point(part_dst);
	    mount_btrfs_raid(part_src, part_dst, devices);
#else
            create_mount_point(part_dst);
	    mount_btrfs_raid(part_src, part_dst, NULL);
#endif
	}else {
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
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", item_name);
    db->get_val(item_name, &db_val[0]);
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", db_val);
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
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", json_param);
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

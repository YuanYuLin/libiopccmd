#include "shell_common.h"
#include "shell_qemu.h"

#ifdef SUPPORT_QEMU

static int get_qemu_count()
{
    struct ops_db_t *db = get_db_instance();
    struct ops_json_t *json = get_json_instance();
    uint8_t db_val[DBVALLEN];
    int count = 0;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("qemu_count", &db_val[0]);
    json_reader_t *db_reader = json->create_json_reader(&db_val[0]);
    count = json->get_json_array_count(db_reader);
    return count;
}

static uint8_t* get_qemu_item_name_by_index(uint8_t index)
{
    uint8_t db_val[DBVALLEN] = {0};
    struct ops_db_t* db = get_db_instance();
    struct ops_json_t* json = get_json_instance();
    uint8_t *str_ptr = NULL;
    memset(&db_val[0], 0, DBVALLEN);
    db->get_val("qemu_count", &db_val[0]);
    json_reader_t* db_reader = json->create_json_reader(&db_val[0]);
    json_reader_t* ao_reader = json->get_json_array_object_by_index(db_reader, index);
    str_ptr = json->get_json_string(ao_reader, NULL, "");
    //str_ptr = json->get_json_array_string_by_index(db_reader, index, "");
    return str_ptr;
}
#define QEMU_SH		"/tmp/qemu_%d.sh"
#define QEMU_IFC_SH	"/tmp/qifc_%d.%d.sh"

static void gen_qemu_sh_cdrom(FILE* fp, uint8_t cdrom_idx, uint8_t boot_idx)
{
	fprintf(fp, "PCI=$PCI'  -drive media=cdrom,id=cdrom-drive-%d,index=%d'\n", cdrom_idx, boot_idx);
	//fprintf(fp, "PCI=$PCI'  -drive media=cdrom,id=cdrom-drive-%d,index=%d,file=%s'\n", cdrom_idx, boot_idx, cdrom);
}

static void gen_qemu_sh_hdd(FILE* fp, uint8_t fs_idx, uint8_t *root, uint8_t boot_idx)
{
	const uint8_t *sys_rootfs_id = "disk-virtio";
	fprintf(fp, "PCI=$PCI' -drive file=%s,format=qcow2,if=none,id=device-%s-%d'\n", 
			root, sys_rootfs_id, fs_idx);
	fprintf(fp, "PCI=$PCI' -device virtio-blk-pci,scsi=off,drive=device-%s-%d,id=%s-%d,bootindex=%d'\n", 
			sys_rootfs_id, fs_idx, sys_rootfs_id, fs_idx, boot_idx);
}

static void gen_qemuifc(FILE* fp, uint8_t *gwifc)
{
	fprintf(fp, "#!/bin/sh\n\n");

	fprintf(fp, "BRIDGE=\"%s\"\n", gwifc);
	fprintf(fp, "NETIFC=\"$1\"\n");
	fprintf(fp, "/sbin/ifconfig $NETIFC promisc 0.0.0.0\n");
	fprintf(fp, "/usr/sbin/brctl addif $BRIDGE $NETIFC\n");
	fprintf(fp, "/usr/sbin/brctl stp $BRIDGE off\n");
}

static void gen_qemu_sh_net(FILE* fp, uint8_t net_idx, uint8_t *nethwaddr, uint8_t *qemuifc_path)
{
	const uint8_t *sys_netdev_id = "net-virtio";
	fprintf(fp, "PCI=$PCI' -netdev type=tap,script=%s,id=device-%s-%d'\n", 
			qemuifc_path, sys_netdev_id, net_idx);
	fprintf(fp, "PCI=$PCI' -device virtio-net-pci,netdev=device-%s-%d,id=%s-%d,mac=%s'\n", 
			sys_netdev_id, net_idx, sys_netdev_id, net_idx, nethwaddr);
}

static void gen_qemu_sh_misc(FILE* fp)
{
	fprintf(fp, "PCI=$PCI' -device virtio-balloon-pci,id=balloon0 -msg timestamp=on'\n");
	fprintf(fp, "QEMU=$QEMU' -usb -device usb-tablet'\n");
	//fprintf(fp, "QEMU=$QEMU' -device ipmi-bmc-sim,id=bmc0 -device isa-ipmi-kcs,bmc=bmc0,irq=5'\n");
}

static void gen_qemu_sh_begin(FILE* fp, uint8_t idx, uint8_t *name, uint16_t smp, uint32_t memory)
{
	fprintf(fp, "#!/bin/sh\n");

	fprintf(fp, "SUPPORT_KVM='qemu-system-x86_64'\n");

	fprintf(fp, "SUPPORT_KVM=$SUPPORT_KVM' -daemonize'\n");
	fprintf(fp, "if [ \"$SUPPORT_HW_VM\" == \"1\" ]; then\n");
	fprintf(fp, "SUPPORT_KVM=$SUPPORT_KVM' -enable-kvm -cpu host'\n");
	fprintf(fp, "fi\n");

	fprintf(fp, "QEMU=$QEMU' -M q35 -realtime mlock=off -no-user-config -nodefaults'\n");
	fprintf(fp, "QEMU=$QEMU' -uuid 20180801-0000-0000-0000-%012d'\n", idx);
	fprintf(fp, "QEMU=$QEMU' -smp %d -m %d -vnc :%d -vga virtio'\n", smp, memory, idx);
	fprintf(fp, "QEMU=$QEMU' -qmp unix:/var/run/qemu%d.uds,server,nowait'\n", idx);
	fprintf(fp, "QEMU=$QEMU' -monitor tcp:127.0.0.1:%d,server,nowait'\n", (2300+idx));
}

static void gen_qemu_sh_end(FILE* fp, uint8_t idx, uint8_t *name)
{
	fprintf(fp, "$SUPPORT_KVM $QEMU $PCI\n");
}

static int create_qemu_sh(uint8_t idx)
{
#define QEMU_STR_LEN	20
    struct ops_log_t *log = get_log_instance();
    struct ops_db_t *db = get_db_instance();
    struct ops_json_t *json = get_json_instance();
    uint8_t path[QEMU_STR_LEN] = { 0 };
    uint8_t qemuifc_path[QEMU_STR_LEN] = { 0 };
    memset(&path[0], 0, QEMU_STR_LEN);
    memset(&qemuifc_path[0], 0, QEMU_STR_LEN);
    uint8_t cdrom_count = 2;

    uint8_t db_item[DBVALLEN];
    memset(&db_item[0], 0, DBVALLEN);
    uint8_t *item = get_qemu_item_name_by_index(idx);
    db->get_val(item, &db_item[0]);
    json_reader_t *qemu_reader = json->create_json_reader(&db_item[0]);

    int enable = json->get_json_int(qemu_reader, "enable", 0);
    log->debug(0x01, __FILE__, __func__, __LINE__, "qemu enable : %d\n", enable);
    if(enable){
        uint8_t *name = json->get_json_string(qemu_reader, "name", "");
        uint32_t memory = json->get_json_int(qemu_reader, "memory", 0);
        uint16_t smp = json->get_json_int(qemu_reader, "smp", 0);

        snprintf(path, QEMU_STR_LEN, QEMU_SH, idx);
        FILE* fp = fopen(path, "w");
        gen_qemu_sh_begin(fp, idx, name, smp, memory);
	gen_qemu_sh_misc(fp);

	for(int x=0;x<cdrom_count;x++) {
		gen_qemu_sh_cdrom(fp, x, x);
	}

        json_reader_t* rootfs_reader = json->get_json_array(qemu_reader, "rootfs", NULL);
        if(rootfs_reader) {
	    int rootfs_count = json->get_json_array_count(rootfs_reader);
	    for(int x=0;x<rootfs_count;x++) {
		    json_reader_t *array_reader = json->get_json_array_object_by_index(rootfs_reader, x);
		    uint8_t* rootfs = json->get_json_string(array_reader, NULL, "");
		    gen_qemu_sh_hdd(fp, x, rootfs, x + cdrom_count);
	    }
        }

	json_reader_t* netifcs_reader = json->get_json_array(qemu_reader, "netifcs", NULL);
	if(netifcs_reader) {
            int netifcs_count = json->get_json_array_count(netifcs_reader);
	    for(int x=0;x<netifcs_count;x++) {
                    json_reader_t* array_reader = json->get_json_array_object_by_index(netifcs_reader, x);
		    uint8_t* hwaddr = json->get_json_string(array_reader, "hwaddr", "00:00:00:00:00:00");
		    uint8_t* gwifc = json->get_json_string(array_reader, "gwifc", "");
		    snprintf(qemuifc_path, QEMU_STR_LEN, QEMU_IFC_SH, idx, x);
		    FILE* netfp = fopen(qemuifc_path, "w");
		    gen_qemuifc(netfp, gwifc);
		    fclose(netfp);
		    chmod(qemuifc_path, S_IRUSR | S_IWUSR | S_IXUSR);
		    gen_qemu_sh_net(fp, x, hwaddr, qemuifc_path);
	    }
	}

        gen_qemu_sh_end(fp, idx, name);
        fclose(fp);
        chmod(path, S_IRUSR | S_IWUSR | S_IXUSR);
    }
    return enable;
}

static int execute_qemu_sh(int idx)
{
    struct ops_shell_t *shell = get_shell_instance();
    uint8_t path[20] = { 0 };
    memset(&path[0], 0, 20);
    sprintf(path, QEMU_SH, idx);
    //sprintf(path, "%s &", path);
    shell->send_sh(SHELL_INSTANCE, strlen(path), path);
    return 0;
}

int start_qemu(uint8_t status_id, uint8_t *args)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t *json_param = (uint8_t*)args;
    struct ops_json_t* json = get_json_instance();
    json_reader_t* reader = json->create_json_reader(json_param);
    int qemu_index = json->get_json_int(reader, "qemu_index", -1);

    if(qemu_index < 0) {
	    log->debug(0x01, __FILE__, __func__, __LINE__, "qemu index %d error ", qemu_index);
	    return 0;
    }

    if(create_qemu_sh(qemu_index)) {
	    execute_qemu_sh(qemu_index);
    }

    return 0;
}

int start_all_qemu(uint8_t status_id, uint8_t *args)
{
    int qemu_count = get_qemu_count();
    for(int idx=0;idx<qemu_count;idx++) {
	    if(create_qemu_sh(idx)) {
		    execute_qemu_sh(idx);
	    }
    }

    return 0;
}

int add_qemu_img(uint8_t status_id, uint8_t *args)
{
    struct ops_log_t* log = get_log_instance();
    uint8_t cmd[DBVALLEN] = { 0 };
    uint8_t devices[DBVALLEN] = { 0 };
    uint8_t *json_param = (uint8_t*)args;
    struct ops_json_t* json = get_json_instance();
    struct ops_shell_t* shell = get_shell_instance();
    json_reader_t* reader = json->create_json_reader(json_param);
    uint8_t *format = json->get_json_string(reader, "format", "qcow2");
    uint8_t *disk_path = json->get_json_string(reader, "disk_path", "");
    uint8_t *size_unit = json->get_json_string(reader, "size_uint", "G");
    uint16_t size = json->get_json_int(reader, "size", 0);
    memset(&devices[0], DBVALLEN, 0);
    memset(&cmd[0], DBVALLEN, 0);

    sprintf(cmd, "qemu-img create -f %s %s %d%s", format, disk_path, size, size_unit);
    log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
    shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
}

#endif

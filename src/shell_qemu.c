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
    str_ptr = json->get_json_array_string_by_index(db_reader, index, "");
    return str_ptr;
}
#if 0
#define QEMU_IFUP	"/tmp/qemu-ifup"
static void write_qemu_ifup()
{
	struct ops_misc_t *misc = get_misc_instance();
	uint8_t path[20] = { 0 };
	FILE *fp = NULL;
	if(!misc->is_file_exist(path)) {
		memset(&path[0], 0, 20);
		sprintf(path, QEMU_IFUP);
		fp = fopen(path, "w");
		if(fp) {
			fprintf(fp, "#!/bin/sh\n");
			fprintf(fp, "BRIDGE='br0'\n");
			fprintf(fp, "NETIFC=$1\n");
			fprintf(fp, "/sbin/ifconfig $NETIFC promisc 0.0.0.0\n");
			fprintf(fp, "/usr/sbin/brctl addif $BRIDGE $NETIFC\n");
			fprintf(fp, "/usr/sbin/brctl stp $BRIDGE off\n");
			fclose(fp);
			chmod(path, S_IRUSR | S_IWUSR | S_IXUSR);
		}
	}
}
#endif
#define QEMU_SH		"/tmp/qemu_%d.sh"
#if 0
static void write_qemu_sh(uint8_t idx, uint8_t *name, uint8_t *rootfs, uint8_t *nethwaddr, uint16_t memory)
{
	const uint8_t *sys_rootfs_id = "disk-virtio-sys";
	const uint8_t *sys_netdev_id = "net-virtio-sys";
	uint8_t path[20] = { 0 };
	memset(&path[0], 0, 20);
	sprintf(path, QEMU_SH, idx);
	FILE* fp = fopen(path, "w");

	if(fp) {
		fprintf(fp, "#!/bin/sh\n");

		fprintf(fp, "/bin/grep 'vmx' /proc/cpuinfo\n");
		fprintf(fp, "support_intel_kvm=$?\n");
		fprintf(fp, "if [ $support_intel_kvm == 0 ]; then\n");
		fprintf(fp, "SUPPORT_KVM='-enable-kvm -cpu host'\n");
		fprintf(fp, "fi\n");

		fprintf(fp, "/bin/grep 'svm' /proc/cpuinfo\n");
		fprintf(fp, "support_amd_kvm=$?\n");
		fprintf(fp, "if [ $support_amd_kvm == 0 ]; then\n");
		fprintf(fp, "SUPPORT_KVM='-enable-kvm -cpu host'\n");
		fprintf(fp, "fi\n");

		fprintf(fp, "QEMU1='qemu-system-x86_64 -M q35 -realtime mlock=off -no-user-config -nodefaults'\n");
		fprintf(fp, "QEMU2='-uuid 20180801-0000-0000-0000-%012d'\n", idx);
		fprintf(fp, "QEMU3='-m %d -vnc :%d -vga std'\n", memory, idx);
		fprintf(fp, "QEMU4='-qmp unix:/var/run/qemu%d.uds,server,nowait'\n", idx);
		fprintf(fp, "QEMU5='-device ipmi-bmc-sim,id=bmc0 -device isa-ipmi-kcs,bmc=bmc0,irq=5'\n");

		fprintf(fp, "PCI1='-device virtio-balloon-pci,id=balloon0 -msg timestamp=on'\n");
		fprintf(fp, "PCI2='-drive file=%s,format=qcow2,if=none,id=device-%s'\n", rootfs, sys_rootfs_id);
		fprintf(fp, "PCI21='-device virtio-blk-pci,scsi=off,drive=device-%s,id=%s,bootindex=2'\n", sys_rootfs_id, sys_rootfs_id);
		fprintf(fp, "PCI3='-netdev type=tap,script=/etc/qemu-ifup,id=device-%s'\n", sys_netdev_id);
		fprintf(fp, "PCI31='-device virtio-net-pci,netdev=device-%s,id=%s,mac=%s'\n", sys_netdev_id, sys_netdev_id, nethwaddr);

		fprintf(fp, "$QEMU1 $QEMU2 $QEMU3 $QEMU4 $QEMU5 $SUPPORT_KVM $PCI1 $PCI2 $PCI21 $PCI3 $PCI31\n");
		fclose(fp);
		chmod(path, S_IRUSR | S_IWUSR | S_IXUSR);
	}
}
#endif
static void gen_qemu_sh_hdd(FILE* fp, uint8_t fs_idx, uint8_t *root)
{
	const uint8_t *sys_rootfs_id = "disk-virtio";
	fprintf(fp, "PCI=$PCI' -drive file=%s,format=qcow2,if=none,id=device-%s-%d'\n", 
			root, sys_rootfs_id, fs_idx);
	fprintf(fp, "PCI=$PCI' -device virtio-blk-pci,scsi=off,drive=device-%s-%d,id=%s-%d,bootindex=%d'\n", 
			sys_rootfs_id, fs_idx, sys_rootfs_id, fs_idx);
}

static void gen_qemu_sh_net(FILE* fp, uint8_t net_idx, uint8_t *nethwaddr)
{
	const uint8_t *sys_netdev_id = "net-virtio";
	fprintf(fp, "PCI=$PCI' -netdev type=tap,script=/etc/qemu-ifup,id=device-%s-%d'\n", 
			sys_netdev_id, net_idx);
	fprintf(fp, "PCI=$PCI' -device virtio-net-pci,netdev=device-%s-%d,id=%s-%d,mac=%s'\n", 
			sys_netdev_id, net_idx, sys_netdev_id, net_idx, nethwaddr);
}

static void gen_qemu_sh_begin(FILE* fp, uint8_t idx, uint8_t *name, uint16_t smp, uint32_t memory)
{
	fprintf(fp, "#!/bin/sh\n");

	fprintf(fp, "SUPPORT_KVM='qemu-system-x86_64'\n");
	fprintf(fp, "/bin/grep 'vmx' /proc/cpuinfo\n");
	fprintf(fp, "support_intel_kvm=$?\n");
	fprintf(fp, "if [ $support_intel_kvm == 0 ]; then\n");
	fprintf(fp, "SUPPORT_KVM=$SUPPORT_KVM' -enable-kvm -cpu host'\n");
	fprintf(fp, "fi\n");

	fprintf(fp, "/bin/grep 'svm' /proc/cpuinfo\n");
	fprintf(fp, "support_amd_kvm=$?\n");
	fprintf(fp, "if [ $support_amd_kvm == 0 ]; then\n");
	fprintf(fp, "SUPPORT_KVM=$SUPPORT_KVM' -enable-kvm -cpu host'\n");
	fprintf(fp, "fi\n");

	fprintf(fp, "QEMU=$QEMU' -M q35 -realtime mlock=off -no-user-config -nodefaults'\n");
	fprintf(fp, "QEMU=$QEMU' -uuid 20180801-0000-0000-0000-%012d'\n", idx);
	fprintf(fp, "QEMU=$QEMU' -smp %d -m %d -vnc :%d -vga virtio'\n", smp, memory, idx);
	fprintf(fp, "QEMU=$QEMU' -qmp unix:/var/run/qemu%d.uds,server,nowait'\n", idx);
	//fprintf(fp, "QEMU=$QEMU' -device ipmi-bmc-sim,id=bmc0 -device isa-ipmi-kcs,bmc=bmc0,irq=5'\n");

	fprintf(fp, "PCI='-device virtio-balloon-pci,id=balloon0 -msg timestamp=on'\n");
}

static void gen_qemu_sh_end(FILE* fp, uint8_t idx, uint8_t *name)
{
	fprintf(fp, "$SUPPORT_KVM $QEMU $PCI\n");
}

static int create_qemu_sh(uint8_t idx)
{
    //struct ops_log_t *log = get_log_instance();
    struct ops_db_t *db = get_db_instance();
    struct ops_json_t *json = get_json_instance();
    uint8_t path[20] = { 0 };
    memset(&path[0], 0, 20);
    snprintf(path, 20, QEMU_SH, idx);

    uint8_t db_item[DBVALLEN];
    memset(&db_item[0], 0, DBVALLEN);
    uint8_t *item = get_qemu_item_name_by_index(idx);
    db->get_val(item, &db_item[0]);
    json_reader_t *qemu_reader = json->create_json_reader(&db_item[0]);

    int enable = json->get_json_int(qemu_reader, "enable", 0);
    //uint8_t *name = json->get_json_string(qemu_reader, "name", "");
    //uint8_t *rootfs = json->get_json_string(qemu_reader, "rootfs", "");
    //uint8_t *nethwaddr = json->get_json_string(qemu_reader, "nethwaddr", "");
    //uint16_t memory = json->get_json_int(qemu_reader, "memory", 0);
    if(enable){
        uint8_t *name = json->get_json_string(qemu_reader, "name", "");
        uint8_t *nethwaddr = json->get_json_string(qemu_reader, "nethwaddr", "");
        uint32_t memory = json->get_json_int(qemu_reader, "memory", 0);
        uint16_t smp = json->get_json_int(qemu_reader, "smp", 0);
        FILE* fp = fopen(path, "w");
        gen_qemu_sh_begin(fp, idx, name, smp, memory);
        json_reader_t* rootfs_reader = json->get_json_array(qemu_reader, "rootfs", NULL);
        if(rootfs_reader) {
	    int rootfs_count = json->get_json_array_count(rootfs_reader);
	    for(int x=0;x<rootfs_count;x++) {
		    uint8_t* rootfs = json->get_json_array_string_by_index(rootfs_reader, x, "");
		    gen_qemu_sh_hdd(fp, x, rootfs);
	    }
        }
	gen_qemu_sh_net(fp, 0, nethwaddr);
        gen_qemu_sh_end(fp, idx, name);
        fclose(fp);
        chmod(path, S_IRUSR | S_IWUSR | S_IXUSR);
    }
    //log->debug(0x01, "en:%d, %s, %s, %s\n", enable, name, rootfs, nethwaddr);
    //write_qemu_sh(idx, name, rootfs, nethwaddr, memory);
    return enable;
}

static int execute_qemu_sh(int idx)
{
    struct ops_misc_t *misc = get_misc_instance();
	uint8_t path[20] = { 0 };
	memset(&path[0], 0, 20);
	sprintf(path, QEMU_SH, idx);
	misc->syscmd(path);
	return 0;
}

int start_qemu(uint8_t status_id, uint8_t * args)
{
#if 0
    write_qemu_ifup();
#endif
    int qemu_count = get_qemu_count();
    for(int idx=0;idx<qemu_count;idx++) {
	    if(create_qemu_sh(idx)) {
		    execute_qemu_sh(idx);
	    }
    }

    return 0;
}

#endif

#ifndef __SHELL_QEMU_H__
#define __SHELL_QEMU_H__

#define SUPPORT_QEMU	1

#ifdef SUPPORT_QEMU
	int start_qemu(uint8_t status_id, uint8_t* args);

	#define SHELL_CMD_QEMU		{"start_qemu",	"", 0, ID_STATUS_QEMU, start_qemu},
#else
	#define SHELL_CMD_QEMU
#endif

#endif

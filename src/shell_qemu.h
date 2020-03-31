#ifndef __SHELL_QEMU_H__
#define __SHELL_QEMU_H__

#define SUPPORT_QEMU	1

#ifdef SUPPORT_QEMU
	int start_all_qemu(uint8_t status_id, uint8_t* args);
	int start_qemu(uint8_t status_id, uint8_t* args);
	int add_qemu_img(uint8_t status_id, uint8_t* args);

	#define SHELL_CMD_QEMU		{"start_all_qemu",	0, ID_STATUS_QEMU, start_all_qemu},	\
					{"start_qemu", 		0, ID_STATUS_QEMU, start_qemu},		\
					{"add_qemu_img", 0, ID_STATUS_QEMU, add_qemu_img},
#else
	#define SHELL_CMD_QEMU
#endif

#endif

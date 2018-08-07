#ifndef __SHELL_LXC_H__
#define __SHELL_LXC_H__

#ifdef SUPPORT_LXC
	int gen_lxc_cfg(uint8_t status_id, uint8_t* args);
	int start_lxc(uint8_t status_id, uint8_t* args);

	#define SHELL_CMD_LXC 	{"gen_lxc_cfg", "", 0, ID_STATUS_LXC, gen_lxc_cfg}, \
				{"start_lxc",	"", 0, ID_STATUS_LXC, start_lxc},
#else
	#define SHELL_CMD_LXC
#endif

#endif

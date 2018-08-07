#ifndef __SHELL_MISC_H__
#define __SHELL_MISC_H__

int set_hostname(uint8_t status_id, uint8_t* args);
int reboot_system(uint8_t status_id, uint8_t* args);
int sync_datetime(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_MISC		{"set_hostname", "", 0, ID_STATUS_UNSPEC, set_hostname},	\
				{"reboot_system", "", 0, ID_STATUS_UNSPEC, reboot_system},	\
				{"sync_datetime", "", 0, ID_STATUS_UNSPEC, sync_datetime},
#endif

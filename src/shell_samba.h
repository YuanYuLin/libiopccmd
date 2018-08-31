#ifndef __SHELL_SAMBA_H__
#define __SHELL_SAMBA_H__

#define SUPPORT_SAMBA	1

#ifdef SUPPORT_SAMBA
	int start_samba(uint8_t status_id, uint8_t* args);
	int stop_samba(uint8_t status_id, uint8_t* args);

	#define SHELL_CMD_SAMBA		{"start_samba",	"", 0, ID_STATUS_SAMBA, start_samba},	\
					{"stop_samba", "", 0, ID_STATUS_SAMBA, stop_samba},
#else
	#define SHELL_CMD_SAMBA
#endif

#endif

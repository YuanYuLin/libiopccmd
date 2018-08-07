#ifndef __SHELL_DRBD_H__
#define __SHELL_DRBD_H__

int start_drbd(uint8_t status_id, uint8_t* args);
int stop_drbd(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_DRBD		{"start_drbd",	"", 0, ID_STATUS_DRBD, start_drbd},	\
				{"stop_drbd",	"", 0, ID_STATUS_DRBD, stop_drbd},

#endif

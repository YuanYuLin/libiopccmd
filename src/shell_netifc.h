#ifndef __SHELL_NETIFC_H__
#define __SHELL_NETIFC_H__

int up_netifc(uint8_t status_id, uint8_t* args);
int down_netifc(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_NETIFC	{"up_netifc", 	0, ID_STATUS_NETIFC, up_netifc},	\
				{"down_netifc",	0, ID_STATUS_NETIFC, down_netifc},

#endif

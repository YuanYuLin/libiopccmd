#ifndef __SHELL_DEBUG_H__
#define __SHELL_DEBUG_H__

int debug_env(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_DEBUG		{"debug_env",	0, ID_STATUS_DEBUG, debug_env},

#endif

#ifndef __SHELL_SSH_H__
#define __SHELL_SSH_H__

int gen_ssh_key(uint8_t status_id, uint8_t* args);
int set_authsalt(uint8_t status_id, uint8_t* args);
int set_authhash(uint8_t status_id, uint8_t* args);
int set_authname(uint8_t status_id, uint8_t* args);
int start_ssh(uint8_t status_id, uint8_t* args);
int stop_ssh(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_SSH		{"gen_ssh_key",	0, ID_STATUS_SSH, gen_ssh_key},	\
				{"set_authname", 0, ID_STATUS_SSH, set_authname}, \
				{"set_authsalt", 0, ID_STATUS_SSH, set_authsalt}, \
				{"set_authhash", 0, ID_STATUS_SSH, set_authhash}, \
				{"start_ssh",	0, ID_STATUS_SSH, start_ssh},	\
				{"stop_ssh",	0, ID_STATUS_SSH, stop_ssh},

#endif

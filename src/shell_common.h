#ifndef __SHELL_COMMON_H__
#define __SHELL_COMMON_H__

#include <pthread.h>
#include <sys/stat.h>
#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "ops_db.h"
#include "cmd_processor.h"
#include "shell_lxc.h"

struct syscmd_t {
        uint8_t cmd[CMD_NAME_LEN];
	uint8_t json_param[DBVALLEN];
	pthread_t pid;
	uint8_t status_id;
	int (*syscmd_func)(uint8_t status_id, uint8_t* json);
} __attribute__((packed)); 

struct syscmd_status_t {
	uint8_t status;
};

#define ID_STATUS_UNSPEC	0xFF
#define ID_STATUS_NETIFC	0x00
#define ID_STATUS_DRBD		0x01
#define ID_STATUS_SSH		0x02
#define ID_STATUS_LXC		0x03
#define ID_STATUS_STORAGE	0x04
#define ID_STATUS_QEMU		0x05
#define ID_STATUS_SAMBA		0x06

#define STATUS_SSH_GENKEY_FINISHED	0x01
#define STATUS_SSH_RUNNING		0x02

#define STATUS_STOP		0x00
#define STATUS_ERROR		0x40
#define STATUS_WAITING		0x80

uint8_t get_status(uint8_t id);
uint8_t set_status(uint8_t id, uint8_t status);
uint8_t set_status_stop(uint8_t id);
uint8_t set_status_waiting(uint8_t id);

#define SHELL_CMD_END	{"", "", 0, ID_STATUS_UNSPEC, NULL}

#endif

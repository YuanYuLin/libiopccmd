#ifndef OPS_CMD_H
#define OPS_CMD_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "ops_mq.h"

#define CMD_STATUS_NORMAL		0x00
#define CMD_STATUS_NOT_FOUND		0x01
#define CMD_STATUS_ERR_PARAM_NOT_FOUND	0x02
#define CMD_STATUS_ERR_UNKNOW		0xFF

struct ops_cmd_t {
	void (*init) (void);
	void (*show_all) (void);
	uint8_t (*process)(struct msg_t*, struct msg_t*);
	uint8_t (*exec_shell)(int16_t index, uint8_t* json);
};

struct ops_cmd_t *get_cmd_instance();
void del_cmd_instance();

struct cmd_processor_t {
	uint8_t fn;
	uint8_t cmd;
	uint8_t (*processor)(uint8_t* req_data, uint8_t* res_data);
};

#define CMD(NAME)	cmd_processor_ ## NAME

#define CMD_FN_RSVD			0x00
#define CMD_FN_1			0x01
#define CMD_FN_2			0x02

#define CMD_NO_RSVD			0x00
#define CMD_NO_1			0x01
#define CMD_NO_2			0x02
#define CMD_NO_3			0x03
#define CMD_NO_4			0x04
#define CMD_NO_5			0x05
#define CMD_NO_6			0x06
#define CMD_NO_7			0x07

#endif

#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define CMD_NAME_LEN	20

uint8_t CMD(get_gpio)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(put_gpio)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(get_gpio_list)(uint8_t* req_data, uint8_t* res_data);

uint8_t CMD(sys_shcmd)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(sys_shcmd_status)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(sys_dao_save)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(sys_dao_reset)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(sys_dao_get)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(sys_dao_set)(uint8_t* req_data, uint8_t* res_data);
uint8_t CMD(qemu_cmd)(uint8_t* req_data, uint8_t* res_data);

uint8_t run_new_shell(uint8_t* req_data, uint8_t* res_data);
//uint8_t _exec_shell(int8_t index, uint8_t* json);

uint8_t get_status(uint8_t id);

struct req_rfb_msg_t {
    uint8_t index;
    uint8_t action;
} __attribute__ ((packed));

struct res_rfb_msg_t {
    uint8_t status;
    uint8_t index;
    uint8_t action;
} __attribute__ ((packed));

#endif

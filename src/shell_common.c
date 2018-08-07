#include "shell_common.h"

static struct syscmd_status_t syscmd_status[0xFF] = {
	0x40
};

uint8_t get_status(uint8_t id)
{
	if(id == ID_STATUS_UNSPEC) 
		return STATUS_ERROR;

	struct syscmd_status_t *obj = NULL;
	obj = &syscmd_status[id];
	return obj->status;
}

uint8_t set_status(uint8_t id, uint8_t status)
{
	struct syscmd_status_t *obj = NULL;
	obj = &syscmd_status[id];
	obj->status = status;
	return obj->status;
}

uint8_t set_status_stop(uint8_t id)
{
	return set_status(id, STATUS_STOP);
}

uint8_t set_status_waiting(uint8_t id)
{
	uint8_t status = get_status(id);
	return set_status(id, STATUS_WAITING | status);
}
/*
uint8_t get_status_drbd()
{
	return get_status(ID_STATUS_DRBD);
}
*/

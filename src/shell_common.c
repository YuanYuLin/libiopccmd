#include "shell_common.h"

static struct syscmd_status_t syscmd_status[0xFF] = {
	0x00
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
	obj->status |= status;
	return obj->status;
}

uint8_t is_status_stop(uint8_t id)
{
	if((syscmd_status[id].status & 0xF0) == STATUS_STOP)
		return 1;
	return 0;
}

uint8_t is_status_init(uint8_t id)
{
	if((syscmd_status[id].status & 0xF0) == STATUS_INIT)
		return 1;
	return 0;
}

uint8_t is_status_prerun(uint8_t id)
{
	if((syscmd_status[id].status & 0xF0) == STATUS_PRERUN)
		return 1;
	return 0;
}

uint8_t is_status_run(uint8_t id)
{
	if((syscmd_status[id].status & 0xF0) == STATUS_RUN)
		return 1;
	return 0;
}

uint8_t is_status_postrun(uint8_t id)
{
	if((syscmd_status[id].status & 0xF0) == STATUS_POSTRUN)
		return 1;
	return 0;
}

uint8_t set_status_stop(uint8_t id)
{
	if(is_status_postrun(id)) {
		syscmd_status[id].status = STATUS_STOP;
	}
	return syscmd_status[id].status;
}

uint8_t set_status_init(uint8_t id)
{
	if(is_status_stop(id)) {
		syscmd_status[id].status = STATUS_INIT;
	}
	return syscmd_status[id].status;
}

uint8_t set_status_prerun(uint8_t id)
{
	if(is_status_init(id)) {
		syscmd_status[id].status = STATUS_PRERUN;
	}
	return syscmd_status[id].status;
}

uint8_t set_status_run(uint8_t id)
{
	if(is_status_prerun(id)) {
		syscmd_status[id].status = STATUS_RUN;
	}
	return syscmd_status[id].status;
}

uint8_t set_status_postrun(uint8_t id)
{
	if(is_status_run(id)) {
		syscmd_status[id].status = STATUS_POSTRUN;
	}
	return syscmd_status[id].status;
}

uint8_t reset_status(uint8_t id)
{
	syscmd_status[id].status = STATUS_STOP;
	return syscmd_status[id].status;
}


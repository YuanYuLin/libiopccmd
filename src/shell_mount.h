#ifndef __SHELL_MOUNT_H__
#define __SHELL_MOUNT_H__

int format_storage(uint8_t status_id, uint8_t* args);
int mount_storage(uint8_t status_id, uint8_t* args);
int umount_storage(uint8_t status_id, uint8_t* args);
int mount_storage_ex(uint8_t status_id, uint8_t* args);
int umount_storage_ex(uint8_t status_id, uint8_t* args);
//int mount_swap(uint8_t status_id, uint8_t* args);

#define SHELL_CMD_STORAGE	{"format_storage", 0, ID_STATUS_STORAGE, format_storage},	\
				{"mount_storage", 0, ID_STATUS_STORAGE, mount_storage},	\
				{"umount_storage", 0, ID_STATUS_STORAGE, umount_storage},	\
				{"mount_storage_ex", 0, ID_STATUS_STORAGE, mount_storage_ex},	\
				{"umount_storage_ex", 0, ID_STATUS_STORAGE, umount_storage_ex},
//				{"mount_swap", 0, ID_STATUS_UNSPEC, mount_swap},
#endif

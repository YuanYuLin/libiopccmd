#include "shell_common.h"

#define SSH_DROPBEAR_KEY_FILE	"/tmp/ssh_dropbear_rsa.key"
#define SSH_OPENSSH_KEY_FILE	"/tmp/ssh_openssh_rsa_key"
#define SSH_DROPBEAR_PID_FILE	"/tmp/ssh_dropbear.pid"
int gen_ssh_key(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

        misc->create_dir_recursive("/tmp/dropbear", 0755);

	snprintf(cmd, CMDLEN, "/bin/dropbearkey -t rsa -f %s", SSH_DROPBEAR_KEY_FILE);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);

	snprintf(cmd, CMDLEN, "/bin/dropbearconvert dropbear openssh %s %s", SSH_DROPBEAR_KEY_FILE, SSH_OPENSSH_KEY_FILE);
	log->debug(0x01, "[%s-%d] %s\n", __func__, __LINE__, cmd);
	misc->syscmd(cmd);
	set_status(status_id, STATUS_SSH_GENKEY_FINISHED);

	return 0;
}

int start_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	if((get_status(status_id) & STATUS_SSH_GENKEY_FINISHED) == STATUS_SSH_GENKEY_FINISHED) {
		snprintf(cmd, CMDLEN, "/bin/dropbear -F -s -g -r %s -P %s", SSH_DROPBEAR_KEY_FILE, SSH_DROPBEAR_PID_FILE);
		log->debug(0x01, "[%s-%d] %d - %s\n", __func__, __LINE__, status_id, cmd);
		set_status(status_id, STATUS_SSH_RUNNING);
		misc->syscmd(cmd);
	}
	return 0;
}

int stop_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	uint8_t cmd[CMDLEN] = {0};
	uint32_t pid = 0;
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	log->debug(0x1, "%s-%s-%d-%s", __FILE__, __func__, __LINE__, SSH_DROPBEAR_PID_FILE);

	pid = misc->get_pid_by_path(SSH_DROPBEAR_PID_FILE);

	if(pid > 0){
		snprintf(cmd, CMDLEN, "kill -9 %d", pid);
		log->debug(0x01, "[%s, %s-%d] %s\n", __FILE__, __func__, __LINE__, cmd);
		misc->syscmd(cmd);

		set_status_stop(status_id);
	}
	return 0;
}

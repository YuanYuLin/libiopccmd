#include "shell_common.h"

#define SSH_DROPBEAR_KEY_FILE	"/tmp/dropbear/ssh_dropbear_rsa.key"
#define SSH_OPENSSH_KEY_FILE	"/tmp/dropbear/ssh_openssh_rsa_key"
#define SSH_DROPBEAR_PID_FILE	"/tmp/dropbear/ssh_dropbear.pid"
int gen_ssh_key(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

        misc->create_dir_recursive("/tmp/dropbear", 0755);
        misc->create_dir_recursive("/var/log/wtmp", 0755);

	snprintf(cmd, CMDLEN, "/bin/dropbearkey -t rsa -f %s", SSH_DROPBEAR_KEY_FILE);
	log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
	//misc->syscmd(cmd);
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);

	set_status(status_id, STATUS_SSH_GENKEY_FINISHED);

	return 0;
}

int set_authname(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t *hash = NULL;
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	json_reader_t* reader = json->create_json_reader(args);
	hash = json->get_json_string(reader, "name", "qemu");

	sprintf(cmd, "export SSH_AUTH_NAME=%s", hash);
	log->debug(0x01, __FILE__, __func__, __LINE__, "hash : %s", cmd);
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
	return 0;
}

int set_authsalt(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t *hash = NULL;
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	json_reader_t* reader = json->create_json_reader(args);
	hash = json->get_json_string(reader, "salt", "$6$01234$56789");

	sprintf(cmd, "export SSH_AUTH_SALT=%s", hash);
	log->debug(0x01, __FILE__, __func__, __LINE__, "hash : %s", cmd);
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
	return 0;
}

int set_authhash(uint8_t status_id, uint8_t* args)
{
//	SSH_AUTH_NAME
//	SSH_AUTH_HASH
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t *hash = NULL;
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	json_reader_t* reader = json->create_json_reader(args);
	hash = json->get_json_string(reader, "hash", "$6$01234$56789");

	sprintf(cmd, "export SSH_AUTH_HASH=%s", hash);
	log->debug(0x01, __FILE__, __func__, __LINE__, "hash : %s", cmd);
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
	return 0;
}

int start_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	if((get_status(status_id) & STATUS_SSH_GENKEY_FINISHED) == STATUS_SSH_GENKEY_FINISHED) {
		//snprintf(cmd, CMDLEN, "/bin/dropbear -F -B -E -j -k -a -r %s -P %s", SSH_DROPBEAR_KEY_FILE, SSH_DROPBEAR_PID_FILE);
		snprintf(cmd, CMDLEN, "/bin/dropbear -r %s -P %s", SSH_DROPBEAR_KEY_FILE, SSH_DROPBEAR_PID_FILE);
		log->debug(0x01, __FILE__, __func__, __LINE__, "%d - %s", status_id, cmd);
		set_status(status_id, STATUS_SSH_RUNNING);
		shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
	}
	return 0;
}

int stop_ssh(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	struct ops_misc_t* misc = get_misc_instance();
	struct ops_shell_t* shell = get_shell_instance();
	uint8_t cmd[CMDLEN] = {0};
	uint32_t pid = 0;
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	log->debug(0x1, __FILE__, __func__, __LINE__, "%s", SSH_DROPBEAR_PID_FILE);

	pid = misc->get_pid_by_path(SSH_DROPBEAR_PID_FILE);

	if(pid > 0){
		snprintf(cmd, CMDLEN, "kill -9 %d", pid);
		log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
		//misc->syscmd(cmd);
		shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);

		set_status_stop(status_id);
	}
	return 0;
}

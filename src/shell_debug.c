#include "shell_common.h"

int debug_env(uint8_t status_id, uint8_t* args)
{
	struct ops_log_t* log = get_log_instance();
	//struct ops_json_t* json = get_json_instance();
	//struct ops_misc_t* misc = get_misc_instance();
	struct ops_shell_t* shell = get_shell_instance();
	//time_t seconds;
	uint8_t cmd[CMDLEN] = {0};
	memset(&cmd[0], 0, CMDLEN);
	//json_reader_t* reader = json->create_json_reader(args);

	//seconds = time(NULL);
	//snprintf(cmd, CMDLEN, "/usr/bin/env > /var/log/env.0x%016lx", seconds);
	snprintf(cmd, CMDLEN, "/usr/bin/env > /var/log/env.txt");
	log->debug(0x01, __FILE__, __func__, __LINE__, "%s", cmd);
	//misc->syscmd(cmd);
	shell->send_sh(SHELL_INSTANCE, strlen(cmd), cmd);
	return 0;
}


#include "ops_log.h"
#include "ops_misc.h"
#include "ops_json.h"
#include "ops_cmd.h"
#include "cmd_processor.h"

#define JS_TYPE			"type"
#define JS_PORT			"port"
#define JS_PIN			"pin"
#define JS_VALUE		"value"
#define JS_DIRECTION		"direction"
#define JS_COMMENT		"comment"
#define JS_NAME			"name"
#define JS_UNIX_TIMESTAMP	"unix_timestamp"
/*
Io_type             string  `json:"type"`
Port                int     `json:"port"`
Pin                 int     `json:"pin"`
Value               int     `json:"value"`
Direction           string  `json:"direction"`
Comment             string  `json:"comment"`
Name                string  `json:"name"`
Unix_timestamp      string  `json:"unix_timestamp"`
*/
#define DIRECTION_OUT	1
#define DIRECTION_IN	0
uint8_t CMD(get_gpio)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	json_writer_t* writer = json->create_json_writer();
	uint8_t* req_gpio_type = NULL;
	uint8_t req_gpio_port = 0;
	uint8_t req_gpio_pin = 0;

	uint8_t res_gpio_type[20] = { 0 };
	uint8_t res_gpio_port = 0;
	uint8_t res_gpio_pin = 0;
	uint8_t res_gpio_value = 1;
	uint8_t res_gpio_direction[20] = { 0 };
	uint8_t res_gpio_comment[20] = { 0 };
	uint8_t res_gpio_name[20] = { 0 };
	uint8_t res_gpio_unix_timestamp[20] = { 0 };
	uint32_t res_len = 0;

	// Get Request Begin
	log->debug(0x01, __FILE__, __func__, __LINE__, "json: %s", req_data);
	req_gpio_type = json->get_json_string(reader, JS_TYPE, "");
	req_gpio_port = json->get_json_int(reader, JS_PORT, 0);
	req_gpio_pin = json->get_json_int(reader, JS_PIN, 0);
	// Get Request End

	// Process Request Begin
	sprintf(res_gpio_type, "%s", req_gpio_type);
	res_gpio_port = req_gpio_port;
	res_gpio_pin = req_gpio_pin;
	res_gpio_value = 1;
	sprintf(res_gpio_direction, "%s", "out");
	sprintf(res_gpio_comment, "%s", "test");
	sprintf(res_gpio_name, "gpio_%d_%d", req_gpio_port, req_gpio_pin);
	sprintf(res_gpio_unix_timestamp, "%s", "20171225101900");
	// Process Request End

	// Create Response Begin
	json->set_json_string(writer, JS_TYPE, res_gpio_type);
	json->set_json_int(writer, JS_PORT, res_gpio_port);
	json->set_json_int(writer, JS_PIN, res_gpio_pin);
	json->set_json_int(writer, JS_VALUE, res_gpio_value);
	json->set_json_string(writer, JS_DIRECTION, res_gpio_direction);
	json->set_json_string(writer, JS_COMMENT, res_gpio_comment);
	json->set_json_string(writer, JS_NAME, res_gpio_name);
	json->set_json_string(writer, JS_UNIX_TIMESTAMP, res_gpio_unix_timestamp);

	res_len = json->out_json_to_bytes(writer, res_data);
	log->debug(0x01, __FILE__, __func__, __LINE__, "len =%ld, data:%s", res_len, res_data);
	// Create Response End

	return CMD_STATUS_NORMAL;
}

uint8_t CMD(put_gpio)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	json_writer_t* writer = json->create_json_writer();

	uint8_t* req_gpio_type = NULL;
	uint8_t req_gpio_port = 0;
	uint8_t req_gpio_pin = 0;
	uint8_t req_gpio_value = 0;
	uint8_t* req_gpio_direction = NULL;
	uint8_t* req_gpio_comment = NULL;
	uint8_t* req_gpio_name = NULL;
	uint8_t* req_gpio_unix_timestamp;

	uint8_t res_gpio_type[20] = { 0 };
	uint8_t res_gpio_port = 0;
	uint8_t res_gpio_pin = 0;
	uint8_t res_gpio_value = 1;
	uint8_t res_gpio_direction[20] = { 0 };
	uint8_t res_gpio_comment[20] = { 0 };
	uint8_t res_gpio_name[20] = { 0 };
	uint8_t res_gpio_unix_timestamp[20] = { 0 };
	uint32_t res_len = 0;

	// Get Request Begin
	log->debug(0x01, __FILE__, __func__, __LINE__, "json: %s", req_data);
	req_gpio_type = json->get_json_string(reader, JS_TYPE, "");
	req_gpio_port = json->get_json_int(reader, JS_PORT, 0);
	req_gpio_pin = json->get_json_int(reader, JS_PIN, 0);
	req_gpio_value = json->get_json_int(reader, JS_VALUE, 0);
	req_gpio_direction = json->get_json_string(reader, JS_DIRECTION, "out");
	req_gpio_comment = json->get_json_string(reader, JS_COMMENT, "");
	req_gpio_name = json->get_json_string(reader, JS_NAME, "");
	req_gpio_unix_timestamp = json->get_json_string(reader, JS_UNIX_TIMESTAMP, "");
	// Get Request End

	// Process Request Begin
	sprintf(res_gpio_type, "%s", req_gpio_type);
	res_gpio_port = req_gpio_port;
	res_gpio_pin = req_gpio_pin;
	res_gpio_value = req_gpio_value;
	sprintf(res_gpio_direction, "%s", req_gpio_direction);
	sprintf(res_gpio_comment, "%s", req_gpio_comment);
	sprintf(res_gpio_name, "%s", req_gpio_name);
	sprintf(res_gpio_unix_timestamp, "%s", req_gpio_unix_timestamp);
	// Process Request End

	// Create Response Begin
	json->set_json_string(writer, JS_TYPE, res_gpio_type);
	json->set_json_int(writer, JS_PORT, res_gpio_port);
	json->set_json_int(writer, JS_PIN, res_gpio_pin);
	json->set_json_int(writer, JS_VALUE, res_gpio_value);
	json->set_json_string(writer, JS_DIRECTION, res_gpio_direction);
	json->set_json_string(writer, JS_COMMENT, res_gpio_comment);
	json->set_json_string(writer, JS_NAME, res_gpio_name);
	json->set_json_string(writer, JS_UNIX_TIMESTAMP, res_gpio_unix_timestamp);

	res_len = json->out_json_to_bytes(writer, res_data);
	log->debug(0x01, __FILE__, __func__, __LINE__, "len =%ld, data:%s", res_len, res_data);
	// Create Response End

	return CMD_STATUS_NORMAL;
}

uint8_t CMD(get_gpio_list)(uint8_t* req_data, uint8_t* res_data)
{
	struct ops_log_t* log = get_log_instance();
	struct ops_json_t* json = get_json_instance();
	json_reader_t* reader = json->create_json_reader(req_data);
	json_writer_t* array_writer = json->create_json_array_writer();
	json_writer_t* writer = NULL;
	int i  = 0;
	uint8_t* req_gpio_type = NULL;

	uint8_t res_gpio_type[20] = { 0 };
	uint8_t res_gpio_port = 0;
	uint8_t res_gpio_pin = 0;
	uint8_t res_gpio_value = 1;
	uint8_t res_gpio_direction[20] = { 0 };
	uint8_t res_gpio_comment[20] = { 0 };
	uint8_t res_gpio_name[20] = { 0 };
	uint8_t res_gpio_unix_timestamp[20] = { 0 };
	uint32_t res_len = 0;

	// Get Request Begin
	log->debug(0x01, __FILE__, __func__, __LINE__, "json: %s", req_data);
	req_gpio_type = json->get_json_string(reader, JS_TYPE, "");
	// Get Request End

	// Process Request Begin
	for(i=0;i<10;i++) {
		memset(&res_gpio_type[0], 0, 20);
		memset(&res_gpio_direction[0], 0, 20);
		memset(&res_gpio_comment[0], 0, 20);
		memset(&res_gpio_name[0], 0, 20);
		memset(&res_gpio_unix_timestamp[0], 0, 20);

		sprintf(res_gpio_type, "%s", req_gpio_type);
		res_gpio_port = 1;
		res_gpio_pin = i + 1;
		res_gpio_value = 1;
		if(i % 2) 
			sprintf(res_gpio_direction, "%s", "out");
		else
			sprintf(res_gpio_direction, "%s", "in");
		sprintf(res_gpio_comment, "%s%d", "test", res_gpio_pin);
		sprintf(res_gpio_name, "gpio_%d_%d", res_gpio_port, res_gpio_pin);
		sprintf(res_gpio_unix_timestamp, "%s%02d", "201712251019", i);

		writer = json->create_json_writer();
		json->set_json_string(writer, JS_TYPE, res_gpio_type);
		json->set_json_int(writer, JS_PORT, res_gpio_port);
		json->set_json_int(writer, JS_PIN, res_gpio_pin);
		json->set_json_int(writer, JS_VALUE, res_gpio_value);
		json->set_json_string(writer, JS_DIRECTION, res_gpio_direction);
		json->set_json_string(writer, JS_COMMENT, res_gpio_comment);
		json->set_json_string(writer, JS_NAME, res_gpio_name);
		json->set_json_string(writer, JS_UNIX_TIMESTAMP, res_gpio_unix_timestamp);
		json->set_json_array(array_writer, writer);

	}
	// Process Request End

	// Create Response Begin
	res_len = json->out_json_to_bytes(array_writer, res_data);
	log->debug(0x01, __FILE__, __func__, __LINE__, "len =%ld, data:%s", res_len, res_data);
	// Create Response End

	return CMD_STATUS_NORMAL;
}


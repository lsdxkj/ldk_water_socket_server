#include "header_file_list.h"

extern char client_ip_str[IP_STRING_LEN];
extern uint16_t client_port;

static void generate_code(struct device_communicate * device_comm, uint8_t device_id[DEVICE_ID_LEN], uint8_t op, uint16_t data_len, uint8_t * data)
{
	uint16_t crc;
	uint16_t len;

	memset(device_comm, 0, sizeof(struct device_communicate));
	memcpy(device_comm->code.device_id, device_id, DEVICE_ID_LEN);
	device_comm->code.op = op;
	device_comm->code.data_len = htons(data_len) & 0xFFFF;
	if(data_len > 0)
	{
		memcpy(device_comm->code.data, data, data_len);
	}

	len = DEVICE_ID_LEN + 3 + data_len;
	crc = get_crc16((uint8_t *)&device_comm->code, len);
	*(uint16_t *)&device_comm->code.data[data_len] = htons(crc);

	device_comm->len = len + 2;
}

void deal_with_php_server(int connect_fd)
{
	uint8_t buffer[DEVICE_MSG_CODE_MAX_LEN];
	ssize_t read_count;
	ssize_t write_count;
	uint8_t response = 0;

	memset(buffer, 0, sizeof(buffer));
	read_count = read(connect_fd, buffer, sizeof(buffer));
	if(read_count == -1)
	{
		write_log(LOG_TYPE_FATAL, "read from php server");
		exit(EXIT_FAILURE);
	}
	else if(read_count > 0)
	{
		write_log(LOG_TYPE_MESSAGE, "[Connect] PHP server connected from %s:%d", client_ip_str, client_port);

		char * mem_str_buffer = malloc(read_count * 4);
		mem_to_string(mem_str_buffer, read_count * 4, (uint8_t *)buffer, read_count, true, true);
		write_log(LOG_TYPE_MESSAGE, "[Receive] Received from php server %s:%d --\n%s\ntotal: %ld bytes", client_ip_str, client_port, mem_str_buffer, read_count);
		free(mem_str_buffer);

		struct api_args_struct * api_args = (struct api_args_struct *)buffer;
		struct device_operation device_op;
		memset(&device_op, 0, sizeof(struct device_operation));

		char device_status_file_name[DEVICE_ID_LEN * 2 + 2];
		mem_to_string(device_status_file_name, sizeof(device_status_file_name), api_args->device_id, DEVICE_ID_LEN, true, false);
		device_status_file_name[DEVICE_ID_LEN * 2] = 'S';
		device_status_file_name[DEVICE_ID_LEN * 2 + 1] = '\0';

		char device_operation_file_name[DEVICE_ID_LEN * 2 + 2];
		mem_to_string(device_operation_file_name, sizeof(device_operation_file_name), api_args->device_id, DEVICE_ID_LEN, true, false);
		device_operation_file_name[DEVICE_ID_LEN * 2] = 'O';
		device_operation_file_name[DEVICE_ID_LEN * 2 + 1] = '\0';

		// 如果设备状态文件不存在，说明设备离线
		if(access(device_status_file_name, F_OK) == -1)
		{
			response = PHP_RESPONSE_OFFLINE;
			goto LABEL_write_response_to_php_server;
		}

		// 如果设备操作文件存在，说明设备繁忙
		if(access(device_operation_file_name, F_OK) == 0)
		{
			response = PHP_RESPONSE_BUSY;
			goto LABEL_write_response_to_php_server;
		}

		device_op.status = DEVICE_OPERATION_STATUS_NEW;
		memcpy(device_op.device_id, api_args->device_id, DEVICE_ID_LEN);
		device_op.op = api_args->op;
		switch(api_args->op)
		{
			case 0x02:	// 绑定套餐
			{
				device_op.data_len = 11;

				struct device_communicate device_comm;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, api_args->data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x03:	// 关闭屏幕
			case 0x04:	// 打开屏幕
			case 0x05:	// 关机
			case 0x06:	// 开机
			case 0x07:	// 强冲
			{
				device_op.data_len = 1;

				struct device_communicate device_comm;
				uint8_t data = 0;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, &data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x08:	// 充正值
			case 0x09:	// 充负值
			case 0x0B:	// 用时同步
			{
				device_op.data_len = 2;

				struct device_communicate device_comm;
				uint16_t data = *(uint16_t *)api_args->data;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, (uint8_t *)&data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x0E:	// 滤芯复位与修改
			{
				device_op.data_len = 3;

				struct device_communicate device_comm;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, api_args->data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x0F:	// 模式切换
			{
				device_op.data_len = 1;

				struct device_communicate device_comm;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, api_args->data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x0D:	// 查询设备运行信息（立即返回心跳）
			{
				device_op.data_len = 1;

				struct device_communicate device_comm;
				uint8_t data = 0;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, &data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			case 0x10:	// 恢复出厂模式
			{
				device_op.data_len = 1;

				struct device_communicate device_comm;
				uint8_t data = 0;
				generate_code(&device_comm, api_args->device_id, api_args->op, device_op.data_len, &data);
				memcpy(&device_op.request_comm.code, &device_comm.code, device_comm.len);
				device_op.request_comm.len = device_comm.len;

				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
			}
			break;
			default:
			{
				response = PHP_RESPONSE_INVALID_OP_CODE;
				goto LABEL_write_response_to_php_server;
			}
			break;
		}
		
		// 等待设备应答
		memset(&device_op, 0, sizeof(struct device_operation));
		while(true)
		{
			if(access(device_operation_file_name, F_OK) == -1)
			{
				continue;
			}
			read_from_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation));
			if(device_op.status == DEVICE_OPERATION_STATUS_ANSWERED)
				break;
		}
		
		if(device_op.op == 0x0D)
		{
			char buffer[DEVICE_HEARTBEAT_CODE_LEN];
			read_from_file(device_status_file_name, (uint8_t *)buffer, DEVICE_HEARTBEAT_CODE_LEN);
			write_count = write(connect_fd, buffer, DEVICE_HEARTBEAT_CODE_LEN);
			if(write_count != DEVICE_HEARTBEAT_CODE_LEN)
			{
				write_log(LOG_TYPE_FATAL, "write heartbeat to php server");
			}

			if(remove(device_operation_file_name) == -1)
			{
				write_log(LOG_TYPE_FATAL, "remove");
			}

			if(close(connect_fd) == -1)
			{
				write_log(LOG_TYPE_FATAL, "close php server socket");
				exit(EXIT_FAILURE);
			}
			write_log(LOG_TYPE_MESSAGE, "[Disconnect] PHP server disconnected from %s:%d", client_ip_str, client_port);
			exit(EXIT_SUCCESS);
		}
		else
		{
			if(memcmp(&device_op.response_comm.code, &device_op.request_comm.code, device_op.request_comm.len) == 0)
			{
				response = 0x00;
				if(remove(device_operation_file_name) == -1)
				{
					write_log(LOG_TYPE_FATAL, "remove device operation file");
					exit(EXIT_FAILURE);
				}
			}
		}
		

LABEL_write_response_to_php_server:
		write_count = write(connect_fd, &response, sizeof(uint8_t));
		if(write_count != sizeof(uint8_t))
		{
			write_log(LOG_TYPE_FATAL, "write response to php server (writed %ld bytes)", write_count);
		}
		write_log(LOG_TYPE_MESSAGE, "[Send] Sended response to php server: %.2X， %ld bytes", response, sizeof(response));

		if(close(connect_fd) == -1)
		{
			write_log(LOG_TYPE_FATAL, "close php server socket");
			exit(EXIT_FAILURE);
		}
		write_log(LOG_TYPE_MESSAGE, "[Disconnect] PHP server disconnected from %s:%d", client_ip_str, client_port);
		exit(EXIT_FAILURE);
	}
	else
	{
		exit(EXIT_SUCCESS);
	}
}
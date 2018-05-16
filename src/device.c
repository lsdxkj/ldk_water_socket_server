#include "header_file_list.h"

extern char client_ip_str[IP_STRING_LEN];
extern uint16_t client_port;

// 设备主动发往服务器的心跳
struct device_communicate comm1;
// 服务器操作设备的应答消息
struct device_communicate comm2;
// 设备主动发往服务器的非心跳消息
struct device_communicate comm3;

// 是否要求设备立即发送心跳？
int has_new_heartbeat_arrived = -1;

static bool check_device_id(uint8_t device_id[DEVICE_ID_LEN])
{
	return true;
}

static void * receive_data(void * arg)
{
	int connect_fd = *(int *)arg;
	uint8_t buffer[DEVICE_MSG_CODE_MAX_LEN];
	ssize_t read_count;

	while(true)
	{
		memset(buffer, 0, sizeof(buffer));
		read_count = read(connect_fd, buffer, sizeof(buffer));
		if(read_count == -1)
		{
			write_log(LOG_TYPE_FATAL, "read from device");
			exit(EXIT_FAILURE);
		}
		else if(read_count == 0)
		{
			write_log(LOG_TYPE_MESSAGE, "[Disconnect] Device disconnected (device closed) from %s:%d", client_ip_str, client_port);
			exit(EXIT_SUCCESS);
		}
		else if(read_count > 0)
		{
			char * mem_str_buffer = malloc(read_count * 4);
			mem_to_string(mem_str_buffer, read_count * 4, buffer, read_count, true, true);

			if(!check_crc(buffer, read_count))
			{
				write_log(LOG_TYPE_MESSAGE, "[CRC invalid] Get device data error from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
				continue;
			}

			struct device_communicate_code * code = (struct device_communicate_code *)buffer;
			
			switch(code->op)
			{
				case 0x01:	// 心跳帧
				{
					if(read_count == DEVICE_HEARTBEAT_CODE_LEN)
					{
						memset(&comm1, 0, sizeof(struct device_communicate));
						memcpy(&comm1.code, buffer, DEVICE_HEARTBEAT_CODE_LEN);
						comm1.len = DEVICE_HEARTBEAT_CODE_LEN;

						char device_status_file_name[DEVICE_ID_LEN * 2 + 2];
						mem_to_string(device_status_file_name, sizeof(device_status_file_name), code->device_id, DEVICE_ID_LEN, true, false);
						device_status_file_name[DEVICE_ID_LEN * 2] = 'S';
						device_status_file_name[DEVICE_ID_LEN * 2 + 1] = '\0';
						write_to_file(device_status_file_name, (uint8_t *)code, sizeof(struct device_communicate_code), false);
						write_log(LOG_TYPE_MESSAGE, "[Heartbeat] Get device heartbeat from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
						if(has_new_heartbeat_arrived == 0)
						{
							has_new_heartbeat_arrived = 1;
						}
					}
					else
					{
						write_log(LOG_TYPE_MESSAGE, "[Heartbeat Error] Get device heartbeat error from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
					}
				}
				break;
				case 0x02:	// 绑定套餐
				case 0x03:	// 关闭屏幕
				case 0x04:	// 打开屏幕
				case 0x05:	// 关机
				case 0x06:	// 开机
				case 0x07:	// 强冲
				case 0x08:	// 充正值
				case 0x09:	// 充负值
				case 0x0B:	// 用时同步
				case 0x0E:	// 滤芯复位与修改
				case 0x0F:	// 模式切换
				{
					memcpy(&comm2.code, buffer, read_count);
					comm2.len = read_count;
				}
				break;
				case 0x0A:	// 用水同步
				case 0x0C:	// 工作状态同步
				{
					memcpy(&comm3.code, buffer, read_count);
					comm3.len = read_count;
				}
				break;
				default:
				{
					write_log(LOG_TYPE_MESSAGE, "[Receive] Receive data from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
				}
			}

			free(mem_str_buffer);
		}

		sleep(1);
	}

	return NULL;
}

void deal_with_device(int connect_fd)
{
	uint8_t io_buffer[DEVICE_MSG_CODE_MAX_LEN];
	ssize_t read_count;

	memset(io_buffer, 0, sizeof(io_buffer));
	read_count = read(connect_fd, io_buffer, sizeof(io_buffer));
	struct device_communicate_code * code = (struct device_communicate_code *)io_buffer;
	if(read_count == -1)
	{
		write_log(LOG_TYPE_FATAL, "read from device");
		exit(EXIT_FAILURE);
	}
	else
	{
		if(read_count == DEVICE_HEARTBEAT_CODE_LEN && code->op == 0x01)
		{
			if(!check_device_id(code->device_id))
			{
				char * mem_str_buffer = malloc(DEVICE_ID_LEN * 4);
				mem_to_string(mem_str_buffer, DEVICE_ID_LEN * 4, code->device_id, DEVICE_ID_LEN, true, true);
				write_log(LOG_TYPE_MESSAGE, "[Device ID invalid] Device ID invalid from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
				free(mem_str_buffer);
				exit(EXIT_SUCCESS);
			}

			char * mem_str_buffer = malloc(read_count * 4);
			mem_to_string(mem_str_buffer, read_count * 4, (uint8_t *)code, read_count, true, true);

			char device_status_file_name[DEVICE_ID_LEN * 2 + 2];
			mem_to_string(device_status_file_name, sizeof(device_status_file_name), code->device_id, DEVICE_ID_LEN, true, false);
			device_status_file_name[DEVICE_ID_LEN * 2] = 'S';
			device_status_file_name[DEVICE_ID_LEN * 2 + 1] = '\0';

			char device_operation_file_name[DEVICE_ID_LEN * 2 + 2];
			mem_to_string(device_operation_file_name, sizeof(device_operation_file_name), code->device_id, DEVICE_ID_LEN, true, false);
			device_operation_file_name[DEVICE_ID_LEN * 2] = 'O';
			device_operation_file_name[DEVICE_ID_LEN * 2 + 1] = '\0';

			if(access(device_status_file_name, F_OK) == 0 && remove(device_status_file_name) == -1)
			{
				write_log(LOG_TYPE_FATAL, "remove status file");
				exit(EXIT_FAILURE);
			}
			if(access(device_status_file_name, F_OK) == 0 && remove(device_operation_file_name) == -1)
			{
				write_log(LOG_TYPE_FATAL, "remove operation file");
				exit(EXIT_FAILURE);
			}

			write_to_file(device_status_file_name, (uint8_t *)code, sizeof(struct device_communicate_code), false);
			write_log(LOG_TYPE_MESSAGE, "[Connect] Device connected from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);

			memset(&comm1, 0, sizeof(struct device_communicate));
			memset(&comm2, 0, sizeof(struct device_communicate));
			memset(&comm3, 0, sizeof(struct device_communicate));

			free(mem_str_buffer);

			pthread_t receive_data_thread;
			int receive_data_thread_status;

			receive_data_thread_status = pthread_create(&receive_data_thread, NULL, receive_data, &connect_fd);
			if(receive_data_thread_status != 0)
			{
				write_log(LOG_TYPE_FATAL, "pthread_create");
				exit(EXIT_FAILURE);
			}

			while(true)
			{
				// 等待设备操作文件创建
				while(access(device_operation_file_name, F_OK) == -1)
					sleep(1);
				
				// 设备文件已经创建，说明PHP服务器发来了新的操作，需要向设备发送控制码
				struct device_operation device_op;
				memset(&device_op, 0, sizeof(struct device_operation));
				read_from_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation));
				if(device_op.op == 0x0D)
				{
					has_new_heartbeat_arrived = 0;
				}
				memset(&comm2, 0, sizeof(struct device_communicate));
				if(write(connect_fd, &device_op.request_comm.code, device_op.request_comm.len) != device_op.request_comm.len)
				{
					write_log(LOG_TYPE_FATAL, "write control code to device %s:%d", client_ip_str, client_port);
					exit(EXIT_FAILURE);
				}
				char * mem_str_buffer = malloc(device_op.request_comm.len * 4);
				mem_to_string(mem_str_buffer, device_op.request_comm.len * 4, (uint8_t *)&device_op.request_comm.code, device_op.request_comm.len, true, true);
				write_log(LOG_TYPE_MESSAGE, "[Send] Sended control code to device %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
				device_op.status = DEVICE_OPERATION_STATUS_UNANSWER;
				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);

				if(device_op.op != 0x10 && device_op.op != 0x0D)
				{
					// 等待设备执行完操作并且返回应答
					while(comm2.code.op == 0)
						;
					
					memcpy(&device_op.response_comm.code, &comm2.code, comm2.len);
					device_op.response_comm.len = comm2.len;
				}
				else
				{
					if(device_op.op == 0x0D)
					{
						while(has_new_heartbeat_arrived != 1)
							;
						has_new_heartbeat_arrived = -1;
					}
					
					memcpy(&device_op.response_comm.code, &device_op.request_comm.code, device_op.request_comm.len);
					device_op.response_comm.len = device_op.request_comm.len;
				}

				device_op.status = DEVICE_OPERATION_STATUS_ANSWERED;
				write_to_file(device_operation_file_name, (uint8_t *)&device_op, sizeof(struct device_operation), false);
				mem_to_string(mem_str_buffer, device_op.response_comm.len * 4, (uint8_t *)&device_op.response_comm.code, device_op.response_comm.len, true, true);
				write_log(LOG_TYPE_MESSAGE, "[Receive] Received feedback code from device %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);

				while(access(device_operation_file_name, F_OK) == 0)
					;

				free(mem_str_buffer);

				if(device_op.op == 0x10)
				{
					// 设备恢复出厂设置后，连接会断开，所以在此主动断开连接。
					write_log(LOG_TYPE_MESSAGE, "[Disconnect] Device disconnected from %s:%d", client_ip_str, client_port);
					exit(EXIT_SUCCESS);
				}
			}
		}
		else
		{
			// 无效的心跳，应该不理会
			char * mem_str_buffer = malloc(read_count * 4);
			mem_to_string(mem_str_buffer, read_count * 4, (uint8_t *)code, read_count, true, true);
			write_log(LOG_TYPE_MESSAGE, "[Invalid heartbeat] Device disconnected from %s:%d --\n%s", client_ip_str, client_port, mem_str_buffer);
			free(mem_str_buffer);
			exit(EXIT_SUCCESS);
		}
	}
}
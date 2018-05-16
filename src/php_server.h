#ifndef PHP_SERVER_H_
#define PHP_SERVER_H_

#include "header_file_list.h"

/* 与PHP服务器之间的接口信息 */
// 最大接口大小
#define PHP_SERVER_ARGS_MAX_SIZE	11

// 向PHP服务器返回的回复
// 成功
#define PHP_RESPONSE_SUCCESS			0x00
// 无效的功能号
#define PHP_RESPONSE_INVALID_OP_CODE	0xFD
// 设备繁忙
#define PHP_RESPONSE_BUSY				0xFE
// 设备离线
#define PHP_RESPONSE_OFFLINE			0xFF

struct api_args_struct
{
	uint8_t device_id[DEVICE_ID_LEN];
	uint8_t op;
	uint8_t data[PHP_SERVER_ARGS_MAX_SIZE];
} __attribute__ ((packed));

struct device_operation
{
	uint8_t status;
	uint8_t device_id[DEVICE_ID_LEN];
	uint8_t op;
	uint16_t data_len;
	uint8_t data[DEVICE_MSG_CODE_DATA_MAX_LEN];
	struct device_communicate request_comm;
	struct device_communicate response_comm;
} __attribute__((packed));

void deal_with_php_server(int connect_fd);

#endif
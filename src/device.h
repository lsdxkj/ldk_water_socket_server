#ifndef DEVICE_H_
#define DEVICE_H_

/* 设备信息 */
#define DEVICE_COUNT_MAX	1000
#define DEVICE_ID_LEN		10
#define DEVICE_HEARTBEAT_CODE_LEN		55
#define DEVICE_MSG_CODE_MAX_LEN			55
#define DEVICE_MSG_CODE_DATA_MAX_LEN	(DEVICE_MSG_CODE_MAX_LEN - 15)

struct device_communicate_code
{
	uint8_t device_id[DEVICE_ID_LEN];
	uint8_t op;
	uint16_t data_len;
	uint8_t data[DEVICE_MSG_CODE_DATA_MAX_LEN];
	uint16_t crc;
} __attribute__((packed));

struct device_communicate
{
	size_t len;
	struct device_communicate_code code;
} __attribute__((packed));

void deal_with_device(int connect_fd);

#endif
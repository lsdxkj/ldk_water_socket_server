#ifndef TOOLS_H_
#define TOOLS_H_

#include "header_file_list.h"

/* 文件操作 */
bool read_from_file(const char * file_name, uint8_t * data, size_t len);
bool write_to_file(const char * file_name, uint8_t * data, size_t len, bool is_append);

/* 日志管理 */
#define LOG_TYPE_MESSAGE	0
#define LOG_TYPE_FATAL		1
void write_log(int log_type, const char * format, ...);

/* 日期时间 */
void get_now_time_14(char * buffer, size_t buffer_size);
void get_now_time_19(char * buffer, size_t buffer_size);

/* 内存 */
bool mem_to_string(char * buffer, size_t buffer_size, uint8_t * addr, size_t len, bool is_upper, bool has_whitespace);

/* crc16 */
uint16_t get_crc16(uint8_t * buf, uint16_t len);
bool check_crc(uint8_t * buffer, uint16_t len);

#endif
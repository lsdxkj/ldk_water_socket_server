#include "header_file_list.h"

bool read_from_file(const char * file_name, uint8_t * data, size_t len)
{
	FILE * fp = NULL;
	size_t read_count = 0;

	fp = fopen(file_name, "r");
	if(fp == NULL)
	{
		write_log(LOG_TYPE_FATAL, "fopen");
		return false;
	}

	read_count = fread(data, 1, len, fp);
	if(read_count != len)
	{
		write_log(LOG_TYPE_FATAL, "fread(len = %ld, read_count = %ld)", len, read_count);
		return false;
	}

	if(fclose(fp) != 0)
	{
		write_log(LOG_TYPE_FATAL, "fclose");
		return false;
	}

	return true;
}

bool write_to_file(const char * file_name, uint8_t * data, size_t len, bool is_append)
{
	FILE * fp = NULL;
	size_t write_count = 0;

	if(is_append)
		fp = fopen(file_name, "a");
	else
		fp = fopen(file_name, "w");
	if(fp == NULL)
	{
		perror("fopen");
		return false;
	}

	write_count = fwrite(data, 1, len, fp);
	if(write_count != len)
	{
		perror("write");
		return false;
	}

	if(fclose(fp) != 0)
	{
		perror("fclose");
		return false;
	}

	return true;
}

void write_log(int log_type, const char * format, ...)
{
	char buffer[1024];
	char time_str[20];

	va_list args;
	va_start(args, format);

	memset(buffer, 0, sizeof(buffer));
	get_now_time_19(time_str, sizeof(time_str));

	switch(log_type)
	{
		case LOG_TYPE_MESSAGE:
		{
			char log_content[1000];

			vsnprintf(log_content, sizeof(log_content), format, args);
			sprintf(buffer, "[%s] %s\n", time_str, log_content);
			if(!write_to_file(LOG_FILE_NAME_MESSAGE, (uint8_t *)buffer, strlen(buffer), true))
			{
				fprintf(stderr, buffer, NULL);
			}
		}
		break;
		case LOG_TYPE_FATAL:
		{
			char error_str[256];
			char log_content[700];
			
			memset(log_content, 0, sizeof(log_content));
			vsnprintf(log_content, sizeof(log_content), format, args);
			sprintf(buffer, "[%s] %s -- %s\n", time_str, log_content, strerror_r(errno, error_str, sizeof(error_str)));
			if(!write_to_file(LOG_FILE_NAME_FATAL, (uint8_t *)buffer, strlen(buffer), true))
			{
				fprintf(stderr, buffer, NULL);
			}
		}
		break;
	}

	va_end(args);
}

void get_now_time_14(char * buffer, size_t buffer_size)
{
	memset(buffer, 0, buffer_size);

	time_t now_time;
	time(&now_time);
	strftime(buffer, buffer_size, "%Y%m%d%H%M%S", localtime(&now_time));
}

void get_now_time_19(char * buffer, size_t buffer_size)
{
	memset(buffer, 0, buffer_size);

	time_t now_time;
	time(&now_time);
	strftime(buffer, buffer_size, "%F %T", localtime(&now_time));
}

bool mem_to_string(char * buffer, size_t buffer_size, uint8_t * addr, size_t len, bool is_upper, bool has_whitespace)
{
	if(has_whitespace)
	{
		if(buffer_size < len * 3 + len / 16 + 1)
			return false;
	}
	else
	{
		if(buffer_size < len * 2 + 1)
			return false;
	}

	memset(buffer, 0, buffer_size);
	for(uint8_t i = 0; i < len; i++)
	{
		int left_num = addr[i] >> 4;
		int right_num = addr[i] & 0x0F;
		if(left_num < 10)
		{
			*buffer++ = left_num + '0';
		}
		else
		{
			*buffer++ = left_num - 10 + (is_upper ? 'A' : 'a');
		}
		if(right_num < 10)
		{
			*buffer++ = right_num + '0';
		}
		else
		{
			*buffer++ = right_num - 10 + (is_upper ? 'A' : 'a');
		}

		if(has_whitespace)
		{
			if(i % 16 == 7)
			{
				*buffer++ = ' ';
				*buffer++ = ' ';
			}
			else if(i % 16 == 15 && i != len - 1)
			{
				*buffer++ = '\n';
			}
			else
			{
				*buffer++ = ' ';
			}
		}
	}

	return true;
}

uint16_t get_crc16(uint8_t * buffer, uint16_t len)
{
	uint16_t crc = 0xFFFF;
	
	for(int j = 0; j < len; j++)
	{
		crc = crc ^ *buffer++;
		for(int i = 0; i < 8; i++)
		{
			if((crc & 0x0001) > 0)
			{
				crc = crc >> 1;
				crc = crc ^ 0xA001;
			}
			else
			{
				crc = crc >> 1;
			}   
		 }
	}

	return crc;
}

bool check_crc(uint8_t * buffer, uint16_t len)
{
	uint16_t original_crc = *(uint16_t *)(buffer + len - 2);
	return get_crc16(buffer, len - 2) == htons(original_crc);
}
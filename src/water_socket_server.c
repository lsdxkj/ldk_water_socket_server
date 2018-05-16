#include "header_file_list.h"

char client_ip_str[IP_STRING_LEN];
uint16_t client_port;

static void sigchld_handler(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0)
		;
}

static void sigkill_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

static void * check_if_exit(void * arg)
{
	uint8_t control_code;
	
	while(true)
	{
		if(access(SERVER_CONTROL_FILE_NAME, F_OK) == 0
			&& read_from_file(SERVER_CONTROL_FILE_NAME, &control_code, 1)
			&& control_code == SERVER_CONTROL_CODE_STOP)
		{
			exit(EXIT_SUCCESS);
		}

		sleep(1);
	}

	return NULL;
}

void init(void)
{
	// 注册SIGCHLD信号处理程序
	if(signal(SIGCHLD, sigchld_handler) == SIG_ERR)
	{
		write_log(LOG_TYPE_FATAL, "signal");
		exit(EXIT_FAILURE);
	}

	pthread_t check_if_exit_thread;
	int check_if_exit_thread_status;
	check_if_exit_thread_status = pthread_create(&check_if_exit_thread, NULL, check_if_exit, NULL);
	if(check_if_exit_thread_status != 0)
	{
		write_log(LOG_TYPE_FATAL, "pthread_create");
		exit(EXIT_FAILURE);
	}

	// 清除历史数据
	system("rm -f *S");
	system("rm -f *O");

	// 备份上一次的消息日志
	if(access(LOG_FILE_NAME_MESSAGE, F_OK) == 0)
	{
		char buffer[100];
		memset(buffer, 0, sizeof(buffer));
		strncpy(buffer, LOG_FILE_NAME_MESSAGE, strlen(LOG_FILE_NAME_MESSAGE));
		buffer[strlen(LOG_FILE_NAME_MESSAGE)] = '.';
		get_now_time_14(&buffer[strlen(LOG_FILE_NAME_MESSAGE) + 1], sizeof(buffer) - strlen(LOG_FILE_NAME_MESSAGE) - 1);
		if(rename(LOG_FILE_NAME_MESSAGE, buffer) == -1)
		{
			fprintf(stderr, "Backup log file error!\n");
			exit(EXIT_FAILURE);
		}
	}

	// 备份上一次的消息日志
	if(access(LOG_FILE_NAME_FATAL, F_OK) == 0)
	{
		char buffer[100];
		memset(buffer, 0, sizeof(buffer));
		strncpy(buffer, LOG_FILE_NAME_FATAL, strlen(LOG_FILE_NAME_FATAL));
		buffer[strlen(LOG_FILE_NAME_FATAL)] = '.';
		get_now_time_14(&buffer[strlen(LOG_FILE_NAME_FATAL) + 1], sizeof(buffer) - strlen(LOG_FILE_NAME_FATAL) - 1);
		if(rename(LOG_FILE_NAME_FATAL, buffer) == -1)
		{
			fprintf(stderr, "Backup log file error!\n");
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char * argv[])
{
	printf("Running water socket server v%s ...\n", SERVER_VERSION);

	init();

	int listen_fd;
	struct sockaddr_in server_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listen_fd == -1)
	{
		write_log(LOG_TYPE_FATAL, "socket");
		exit(EXIT_FAILURE);
	}

	// 这段代码是为了能够复用监听的端口。
	int on = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		write_log(LOG_TYPE_FATAL, "setsockopt");
		exit(EXIT_FAILURE);
	}

	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(DEVICE_SERVER_PORT);

	if(bind(listen_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
	{
		write_log(LOG_TYPE_FATAL, "bind");
		exit(EXIT_FAILURE);
	}

	if(listen(listen_fd, DEVICE_COUNT_MAX) == -1)
	{
		write_log(LOG_TYPE_FATAL, "listen");
		exit(EXIT_FAILURE);
	}

	for(;;)
	{
		int connect_fd;
		struct sockaddr_in client_address;
		socklen_t sock_len;

		sock_len = sizeof(struct sockaddr_in);
		connect_fd = accept(listen_fd, (struct sockaddr *)&client_address, &sock_len);
		if(connect_fd == -1)
		{
			write_log(LOG_TYPE_FATAL, "accept");
			exit(EXIT_FAILURE);
		}

		pid_t pid = fork();
		if(pid == -1)
		{
			write_log(LOG_TYPE_FATAL, "fork");
			exit(EXIT_FAILURE);
		}
		else if(pid == 0)
		{
			if(close(listen_fd) == -1)
			{
				write_log(LOG_TYPE_FATAL, "close listen_fd");
				exit(EXIT_FAILURE);
			}

			signal(SIGKILL, sigkill_handler);
          	prctl(PR_SET_PDEATHSIG, SIGKILL);

			inet_ntop(AF_INET, &client_address.sin_addr, client_ip_str, sizeof(client_ip_str));
			client_port = ntohs(client_address.sin_port);

			if(strncmp(client_ip_str, PHP_SERVER_IP1, IP_STRING_LEN) == 0
				|| strncmp(client_ip_str, PHP_SERVER_IP2, IP_STRING_LEN) == 0)
			{
				deal_with_php_server(connect_fd);
			}
			else
			{
				deal_with_device(connect_fd);
				exit(EXIT_SUCCESS);
			}
		}

		if(close(connect_fd) == -1)
		{
			write_log(LOG_TYPE_FATAL, "close connect_fd");
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
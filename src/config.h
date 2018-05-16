#ifndef CONFIG_H_
#define CONFIG_H_

/* 当前版本 */
#define SERVER_VERSION		"0.21"

/* IP/端口 */
#define IP_STRING_LEN	16
#define DEVICE_SERVER_PORT		6500
#define PHP_SERVER_PORT		6501
#define PHP_SERVER_IP1		"120.79.169.91"
#define PHP_SERVER_IP2		"127.0.0.1"

/* 日志文件 */
#define LOG_FILE_NAME_FATAL		"log_fatal"
#define LOG_FILE_NAME_MESSAGE	"log_message"

/* 本服务器控制文件 */
#define SERVER_CONTROL_FILE_NAME	"server_control_file"

/* 本服务器控制命令码 */
#define SERVER_CONTROL_CODE_START	'1'
#define SERVER_CONTROL_CODE_STOP	'2'

/* 系统等待提示符 */
#define SYSTEM_WAIT_FOR_CONNECT		'.'
#define SYSTEM_WAIT_FOR_NEW_OP		'+'
#define SYSTEM_WAIT_FOR_ANSWER		'-'
#define SYSTEM_WAIT_FOR_RESPONSE	'*'

// 设备操作过程中的状态
// socket服务器获得从PHP服务器发来的新的命令
#define DEVICE_OPERATION_STATUS_NEW			1
// socket服务器向设备发出控制码但却未得到应答
#define DEVICE_OPERATION_STATUS_UNANSWER	2
// socket服务器已得到设备的应答
#define DEVICE_OPERATION_STATUS_ANSWERED	3

#endif
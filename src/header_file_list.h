#ifndef HEADER_FILE_LIST_H
#define HEADER_FILE_LIST_H

/* Standard C 99 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

/* Standard C 99 */
//#include <stdint.h>
typedef unsigned int	uint32_t;
typedef unsigned short	uint16_t;
typedef unsigned char	uint8_t;
//#include <stdbool.h>
typedef int bool;
#define true	1
#define false	0

/* System call */
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <wait.h>
#include <sys/prctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* User defined */
#include "config.h"
#include "tools.h"
#include "device.h"
#include "php_server.h"

#endif
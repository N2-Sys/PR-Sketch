/*********************************************************************
 * Copyright (C)
 * File name: 	ancillary
 * Author: 		Siyuan Sheng
 * Version: 	1
 * Date: 		04/09/2019
 * Description: ancillary functions
**********************************************************************/

#ifndef __ANCILLARY_H__
#define __ANCILLARY_H__

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"

#define PATH_LOG_PREFIX "[ancillary] "

#define MAX_PATH_LENGTH 100

#define MAX_STRING_LENGTH 100

const char* path_join(const char* dirname, const char* filename);

static inline const char* ntoa(uint32_t ip) {
	struct in_addr in;
	in.s_addr = ip;
	return inet_ntoa(in);
}

#endif

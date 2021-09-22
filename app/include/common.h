/*!
* @file common.h
* @brief 公共头文件
* @author xiongbinbin
* @date 2021.7
*/
#ifndef ADAPTER_COMMON_H
#define ADAPTER_COMMON_H

#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "logger.h"

#define     SUCCESS             (0)
#define     FAILED              (-1)

#define     ADAPTER_HTTP_CLIENT_RECONNET_TIMES   (0)            //HTTP客户端重连次数
#define     ADAPTER_HTTP_CLIENT_TIMEOUT_SECONDS  (5)            //HTTP客户端连接超时时间

#define     ASSERT(expression) assert(expression)

#define SAFE_FREE(x) do\
{\
	if(NULL != (x))\
	{\
		free(x);\
		(x)=NULL;\
	}\
}while(0)

#endif

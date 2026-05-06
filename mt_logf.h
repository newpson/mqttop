#pragma once
#include <stdio.h>

#define mt_logf(type, fmt, ...) fprintf(stderr, type " | " __FILE__ ":%u@%s() | " fmt "\n", __LINE__,  __func__, ##__VA_ARGS__)

#define mt_errorf(fmt, ...) mt_logf("E", fmt, ##__VA_ARGS__)
#define mt_notef(fmt, ...) mt_logf("N", fmt, ##__VA_ARGS__)
#define mt_warnf(fmt, ...) mt_logf("W", fmt, ##__VA_ARGS__)
#ifndef NDEBUG
#define mt_debugf(fmt, ...) mt_logf("D", fmt, ##__VA_ARGS__)
#else
#define mt_debugf(fmt, ...) (void)(0)
#endif
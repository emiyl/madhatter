#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

void log_info(const char *format, ...);
void log_error(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
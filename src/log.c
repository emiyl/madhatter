#include "log.h"

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[INFO] ");
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
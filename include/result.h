#ifndef WIDEBRIM_RESULT_H
#define WIDEBRIM_RESULT_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    RESULT_ERROR_UNKNOWN = -1,
    RESULT_OK = 0,
    RESULT_ERROR_OUT_OF_MEMORY = 1,
    RESULT_ERROR_FILE_NOT_FOUND = 2,
    RESULT_ERROR_INVALID_ARGUMENT = 3,
} result_code;

#if defined(__cplusplus)
}
#endif

#endif
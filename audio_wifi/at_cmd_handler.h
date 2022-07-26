#ifndef __AT_CMD_HANDLER_H
#define __AT_CMD_HANDLER_H

#include <stdint.h>
#include "at_device.h"

#define AT_CMD_HANDLER_DEBUG 0

typedef enum : uint8_t {
/*00*/AT_CMD = 0,
/*01*/AT_QUESTION,
/*02*/AT_CMD_HANDLE_NUM
} at_cmd_handle_t;

extern at_funcation_t at_fun_handle[];
void at_test_attach_cb(uint8_t cmd, at_test_callback cb, void* arg = nullptr);
void at_test_detach_cb(uint8_t cmd);
void at_query_attach_cb(uint8_t cmd, at_query_callback cb, void* arg = nullptr);
void at_query_detach_cb(uint8_t cmd);
void at_exe_attach_cb(uint8_t cmd, at_exe_callback cb, void* arg = nullptr);
void at_exe_detach_cb(uint8_t cmd);
void at_setup_attach_cb(uint8_t cmd, at_setup_callback cb, void* arg = nullptr);
void at_setup_detach_cb(uint8_t cmd);

#endif // __AT_CMD_HANDLER_H
#include <arduino.h>
#include "at_cmd_handler.h"
#include "c_utility.h"
#include "at_device.h"

#if (defined AT_CMD_HANDLER_DEBUG) && (AT_CMD_HANDLER_DEBUG == 1)
#include "THIoT_SerialTrace.h"
#define AT_CMD_HANDLER_TAG_CONSOLE(...) SERIAL_TAG_LOGI("[AT_CMD_HANDLER]", __VA_ARGS__)
#define AT_CMD_HANDLER_CONSOLE(...) SERIAL_LOGI(__VA_ARGS__)
#else
#define AT_CMD_HANDLER_TAG_CONSOLE(...)
#define AT_CMD_HANDLER_CONSOLE(...)
#endif

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  AT_CMD_TYPE_TEST = 0,
  AT_CMD_TYPE_QUERY,
  AT_CMD_TYPE_SETUP,
  AT_CMD_TYPE_EXE
} at_cmd_type_t;

/* Private define ------------------------------------------------------------*/
#define AT_DEVICE_RESP(W, f_, ...)    vspfunc(W, (f_), ##__VA_ARGS__)

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void at_exe_cmd_null(at_funcation_t *at);

/* cmd_name, cmd_len, (*test_cmd), (*query_cmd), (*setup_cmd), (*exe_cmd)
 *         ,        , AT+<CMD>=?\r, AT+<CMD>?\r, AT+<CMD>=..\r, AT+<CMD>\r */
at_funcation_t at_fun_handle[AT_CMD_HANDLE_NUM] = {
  {(const char *)""           ,  0 , nullptr, nullptr , nullptr , at_exe_cmd_null},
  {(const char *)"+QUESTION"  ,  9 , nullptr, nullptr , nullptr , nullptr},
  {(const char *)"+NEW_DRIVER",  11, nullptr, nullptr , nullptr , nullptr},
  {(const char *)"+DROWSINESS",  11, nullptr, nullptr , nullptr , nullptr},
  {(const char *)"+NO_DRIVER" ,  10, nullptr, nullptr , nullptr , nullptr}
};

static void at_exe_cmd_null(at_funcation_t *at)
{
  at->write((const uint8_t*)at_cmd_ok, AT_OK_LENGTH);
}

void at_test_attach_cb(uint8_t cmd, at_test_callback cb, void* arg)
{
  at_fun_handle[cmd].test_cmd = cb;
  at_fun_handle[cmd].arg = arg;
}

void at_test_detach_cb(uint8_t cmd)
{
  at_fun_handle[cmd].test_cmd = NULL;
}

void at_query_attach_cb(uint8_t cmd, at_query_callback cb, void* arg)
{
  at_fun_handle[cmd].query_cmd = cb;
  at_fun_handle[cmd].arg = arg;
}

void at_query_detach_cb(uint8_t cmd)
{
  at_fun_handle[cmd].query_cmd = NULL;
}

void at_exe_attach_cb(uint8_t cmd, at_exe_callback cb, void* arg)
{
  at_fun_handle[cmd].exe_cmd = cb;
  at_fun_handle[cmd].arg = arg;
}

void at_exe_detach_cb(uint8_t cmd)
{
  at_fun_handle[cmd].exe_cmd = NULL;
}

void at_setup_attach_cb(uint8_t cmd, at_setup_callback cb, void* arg)
{
  at_fun_handle[cmd].setup_cmd = cb;
  at_fun_handle[cmd].arg = arg;
}

void at_setup_detach_cb(uint8_t cmd)
{
  at_fun_handle[cmd].setup_cmd = NULL;
}

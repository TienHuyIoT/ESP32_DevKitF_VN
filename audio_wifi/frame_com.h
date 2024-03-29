#ifndef _FRAME_COM_
#define _FRAME_COM_

/* Macro enable debug frame communication */
#define FRAME_COM_DEBUG_ENABLE	0

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "platform_ticker.h"
#include "frame.h"

typedef enum {
  FRAME_COM_NONE, FRAME_COM_WAIT_START, FRAME_COM_GET_LENGTH, FRAME_COM_WAIT_STOP, FRAME_COM_FINISH
} sm_frame_com_typedef;

typedef struct {
  struct {
    uint8_t *buf;
    size_t size;
  } tx;
  
  struct {
    uint8_t *buf;
    size_t size;
  } rx;
} fc_buffer_t;

#define FRAME_COM_INIT(P_COMM_PARAMS, EVENT_CALLBACK, RX_BUFF_SIZE, TX_BUFF_SIZE) \
  do                                                                   \
  {                                                                    \
    fc_buffer_t buffers;                                               \
    static uint8_t rx_buf[RX_BUFF_SIZE];                               \
    static uint8_t tx_buf[TX_BUFF_SIZE];                               \
    buffers.rx.buf = rx_buf;                                           \
    buffers.rx.size = sizeof(rx_buf);                                  \
    buffers.tx.buf = tx_buf;                                           \
    buffers.tx.size = sizeof(tx_buf);                                  \
    frame_com_begin(P_COMM_PARAMS, EVENT_CALLBACK, &buffers);\
  } while (0)

struct frame_com_cxt;

typedef void (*frame_com_event_cb)(struct frame_com_cxt*, uint8_t result, uint8_t cmd, uint8_t *data, uint16_t length);
typedef uint32_t (*framewrok_rx_cb)(uint8_t*, uint32_t);
typedef uint32_t (*framewrok_tx_cb)(uint8_t*, uint32_t);

typedef struct frame_com_cxt {
  uint32_t instance;
  frame_com_event_cb event_cb;
  framewrok_rx_cb input_cb;
  framewrok_tx_cb output_cb;
  ticker_t timeout;
  uint8_t *tx_buff; /*!< pointer to tx buffer store data of frame*/
  uint8_t *rx_buff; /*!< pointer to rx buffer store data of frame*/
  uint16_t tx_length; /*!< maximum length is FRAME_SIZE_MAX(258)*/
  uint16_t rx_length; /*!< maximum length is FRAME_SIZE_MAX(258)*/
  uint8_t step; /*!< step machine*/
  uint8_t index;
  uint8_t remain_byte_cnt;
} frame_com_cxt_t;

void frame_com_begin(frame_com_cxt_t *fc, frame_com_event_cb event_cb, fc_buffer_t *p_buffer);

void frame_com_process(frame_com_cxt_t *fc);
uint8_t frame_com_transmit(frame_com_cxt_t *fc, uint8_t cmd, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* _FRAME_COM_ */

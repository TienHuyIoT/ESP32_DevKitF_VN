/** Common include */
#include <stdint.h>
#include <stdbool.h>
#include "common_util.h"
#include "app_fifo_extra.h"

/* Brief: get number byte has been add into fifo
 * Return: 0 (fifo empty), otherwise fifo available
 * */
uint32_t app_fifo_length(app_fifo_t * const fifo)
{
  uint32_t tmp = fifo->read_pos;
  return fifo->write_pos - tmp;
}

/* Brief: get number byte be can push into fifo
 * Return: 0 (fifo full) that mean can not push data into fifo
 *         Otherwise fifo available
 * */
uint32_t app_fifo_available(app_fifo_t * const fifo)
{
  return (fifo->buf_size_mask - app_fifo_length(fifo) + 1);
}

uint32_t app_fifo_buff_peek(app_fifo_t * p_fifo, uint8_t * p_byte_array, uint32_t * p_size)
{
    const uint32_t byte_count    = app_fifo_length(p_fifo);
    const uint32_t requested_len = (*p_size);
    uint32_t       index         = 0;
    uint32_t       read_size     = MIN(requested_len, byte_count);

    (*p_size) = byte_count;

    // Check if the FIFO is empty.
    if (byte_count == 0)
    {
        return APP_FIFO_ERROR;
    }

    // Check if application has requested only the size.
    if (p_byte_array == NULL)
    {
        return APP_FIFO_OK;
    }

    // peek bytes from the FIFO.
    while (index < read_size)
    {
        app_fifo_peek(p_fifo, index, &p_byte_array[index]);
        index++;
    }

    (*p_size) = read_size;

    return APP_FIFO_OK;
}

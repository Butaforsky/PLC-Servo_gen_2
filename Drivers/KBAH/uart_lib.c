#include "uart_lib.h"
/*typdefs */

/* private variables */
/* private functions */
u8 uart_send(uart_t *uart, char *string_to_send)
{
  sprintf((char *)uart->tx_buffer, string_to_send);
  return HAL_UART_Transmit(uart->uart,
                           (u8 *)uart->tx_buffer,
                           strlen((char *)uart->tx_buffer),
                           uart->timeout_ms);
}

u8 uart_send_dma(uart_t *uart, char *string_to_send)
{
  sprintf((char *)uart->tx_buffer, string_to_send);
  HAL_UART_Transmit_DMA(uart->uart,
                        (u8 *)uart->tx_buffer,
                        strlen((char *)uart->tx_buffer));
}

u8 uart_send_format(uart_t *uart, int *string_to_send)
{
  return HAL_UART_Transmit(uart->uart,
                           (u8 *)uart->tx_buffer,
                           strlen((char *)uart->tx_buffer), uart->timeout_ms);
}

u8 uart_send_ok(uart_t *uart)
{
  sprintf((char *)uart->tx_buffer, "OK\r\n");
  return HAL_UART_Transmit(uart->uart, (u8 *)uart->tx_buffer,
                           strlen((char *)uart->tx_buffer),
                           uart->timeout_ms);
}

u8 uart_send_error(uart_t *uart)
{
  sprintf((char *)uart->tx_buffer, "ERROR\r\n");
  return HAL_UART_Transmit(&uart->uart, (u8 *)uart->tx_buffer,
                           strlen((char *)uart->tx_buffer),
                           uart->timeout_ms);
}

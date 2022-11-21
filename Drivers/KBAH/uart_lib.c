#include "uart_lib.h"

/*typdefs */
extern UART_HandleTypeDef huart1;
/* private variables */
uart_t uart =
    {
        .uart = &huart1,
        .timeout_ms = 1000,
        .tail = 0,
        .tx_buffer = {
            0,
        },
        .rx_buffer = {
            0,
        },
};

/* private functions */
u8 uart_send(uart_t *uart, char *string_to_send)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
  sprintf((char *)uart->tx_buffer, string_to_send);
  HAL_UART_Transmit(uart->uart,
                    (u8 *)uart->tx_buffer,
                    strlen((char *)uart->tx_buffer),
                    uart->timeout_ms);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
  memset(uart->tx_buffer, 0, UART_TX_LEN);
  return 0;
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
  HAL_UART_Transmit(uart->uart,
                           (u8 *)uart->tx_buffer,
                           strlen((char *)uart->tx_buffer), uart->timeout_ms);
  memset(uart->tx_buffer, 0, UART_TX_LEN);
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

u8 uart_send_r(uart_t *uart)
{ 
  uart_send(uart, "#\r\n");
}


#ifndef __UART_LIB_H
#define __UART_LIB_H
/* defines */
#define UART_TX_LEN 600
#define UART_RX_LEN 100
/* includes */
#include <stdio.h>
#include "string.h"
#include "short_types.h"
#include "main.h"

#include "usart.h"


/* typedef structs */
typedef struct uart_param
{
  UART_HandleTypeDef* uart;
  u16 timeout_ms;

  u8 tx_buffer[UART_TX_LEN];
  u8 rx_buffer[UART_RX_LEN];

  u8 byte[1];
  u8 tail;
  
} uart_t;


/* functions prototypes */

/**
 * @brief send string to uart 
 * 
 * @param jet - pointer on jetson_t struct with defines of periph
 * @param string_to_send - string literal to send 
 * @return u8 HAL_STATUS 
 */
u8 uart_send(uart_t *uart, char *string_to_send);

/**
 * @brief send string to uart using DMA
 * 
 * @param jet - pointer on jetson_t struct with defines of periph
 * @param string_to_send - string literal to send 
 * @return u8 HAL_STATUS 
 */
u8 uart_send_dma(uart_t *uart, char *string_to_send);

/**
 * @brief user formatable uart send (implements DMA send)
 * 
 * @param jet 
 * @param string_to_send 
 * @return u8 
 */
u8 uart_send_format(uart_t *uart, int* string_to_send);
u8 uart_send_ok(uart_t *uart);
u8 uart_send_error(uart_t *uart);
u8 uart_send_r(uart_t *uart);

#endif  

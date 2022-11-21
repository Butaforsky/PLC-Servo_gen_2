#include "parser.h"
/* private variables */
extern UART_HandleTypeDef huart1;
extern uart_t uart;

word_t word_desc = {.beg = 0, .end = 0, .end_found = 0, .word_len = 0};
data_t parsed_data = {.x_dir = 0, .x_pos = 0, .y_dir = 0, .y_dir = 0, .z_dir = 0, .z_pos = 0};

parser_t parser =
    {
        .uart_inst = &uart,
        .first_word_desc = &word_desc,
        .parsed_parameters = &parsed_data,

};
/* private functions */
// TODO - clear first word params after successful parse
void parser_uart_callback(UART_HandleTypeDef *huart, parser_t *parser)
{
  if (huart->Instance == USART1)
  {
    if (huart->Instance->DR >= 85 && huart->Instance->DR <= 90)
    {
      parser->uart_inst->byte[0] = convert_caps_to_reg(parser->uart_inst->byte[0]);
    }
    parser->uart_inst->rx_buffer[parser->uart_inst->tail] = parser->uart_inst->byte[0];
    if (huart->Instance->DR != 0x0A) // if recieved byte is not \n
    {
      if (parser->first_word_desc->end_found == 0 &&
          parser->uart_inst->rx_buffer[parser->uart_inst->tail] == 0x20)
      {
        parser->first_word_desc->end_found = 1;
        parser->first_word_desc->end = parser->uart_inst->tail;
        parser->first_word_desc->word_len = parser->uart_inst->tail;
      }
      parser->uart_inst->tail++;
      HAL_UART_Receive_DMA(&huart1, (u8 *)parser->uart_inst->byte, 1);
    }
    else if (huart->Instance->DR == 0x0A)
    {
      parser->end_string_found = 1;
      parser->uart_inst->tail = 0;
      HAL_UART_Receive_DMA(&huart1, (u8 *)parser->uart_inst->byte, 1);
    }
  }
}

void input_clear(word_t *input)
{
  input->beg = 0;
  input->end = 0;
  input->end_found = 0;
}

void clear_get_new_command(parser_t *parser)
{
  parser->end_string_found = 0;
  memset(parser->uart_inst->tx_buffer, 0, UART_TX_LEN);
  memset(parser->uart_inst->rx_buffer, 0, UART_RX_LEN);
}

u8 clear(void *word)
{
  memset(word, 0, strlen(word));
  return 0;
}

u8 convert_caps_to_reg(u8 caps_symb)
{
  return caps_symb + (97 - 65); /* 97 - A, 65 - a */
}

u8 compare(char *word, const void *text)
{
  return memcmp((void *)word, text, strlen(text));
}

u8 uart_parse_error(parser_t *parser)
{
  sprintf((char *)parser->uart_inst->tx_buffer, "ERROR\n");
  return HAL_UART_Transmit(parser->uart_inst->uart,
                           (u8 *)parser->uart_inst->tx_buffer,
                           strlen((char *)parser->uart_inst->tx_buffer),
                           parser->uart_inst->timeout_ms);
}

u8 uart_parse_clear(parser_t *jet, word_t *input)
{
  jet->end_string_found = 0;
  input->beg = 0;
  input->end = 0;
  input->end_found = 0;
  input->word_len = 0;
}

u8 uart_parse(parser_t *parser)
{
  u8 word_len = parser->first_word_desc->word_len;

  char word[3][WORD_MAX_LEN] = {
      0,
  }; /* create a local array for parsed words */

  if (word_len == 3) /* if first word len is 3 */
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s %s ", word[0], word[1]); /*get first two words */
    if (!compare(word[0], "set") && !compare(word[1], "x"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set x %d %u",
             &parser->parsed_parameters->x_pos,
             &parser->parsed_parameters->x_arr);
      return TASK_SET_X;
    }
    else if (!compare(word[0], "set") && !compare(word[1], "z"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set z %d %u",
       &parser->parsed_parameters->z_pos,
       &parser->parsed_parameters->z_arr);
      return TASK_SET_Z;
    }
    else if (!compare(word[0], "set") && !compare(word[1], "y"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set y %d %u",
       &parser->parsed_parameters->y_pos,
       &parser->parsed_parameters->y_arr);
      return TASK_SET_Y;
    }
    else if (!compare(word[0], "set") && !compare(word[1], "relay_1"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set relay_1 %d", 
      &parser->parsed_parameters->relay_1_state);
      return TASK_SET_RELAY_1;
    }

    else if (!compare(word[0], "set") && !compare(word[1], "relay_2"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set relay_2 %d", &parser->parsed_parameters->relay_2_state);
      return TASK_SET_RELAY_2;
    }

    else if (!compare(word[0], "set") && !compare(word[1], "relay_3"))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set relay_3 %d", &parser->parsed_parameters->relay_3_state);
      return TASK_SET_RELAY_3;
    }

    else if (!compare(word[0], "get") && !compare(word[1], "x") /*&& !compare(word[2], "pos")*/)
    {
      return TASK_GET_X;
    }
    else if (!compare(word[0], "get") && !compare(word[1], "y") /*&& !compare(word[2], "pos")*/)
    {
      return TASK_GET_Y;
    }
    else if (!compare(word[0], "get") && !compare(word[1], "z") /*&& !compare(word[2], "pos")*/)
    {
      return TASK_GET_Z;
    }
    // relay
    else if (!compare(word[0], "get") && !compare(word[1], "relay_1"))
    {
      return TASK_GET_RELAY_1;
    }

    else if (!compare(word[0], "get") && !compare(word[1], "relay_2"))
    {
      return TASK_GET_RELAY_2;
    }

    else if (!compare(word[0], "get") && !compare(word[1], "relay_3"))
    {
      return TASK_GET_RELAY_3;
    }
    else
    {
      return TASK_ERROR;
    }
  }

  else if (word_len == 4) /* first word == "help" || stop */
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s %s", word[0], word[1]);
    if (!compare(word[0], "help"))
    {
      uart_send(parser->uart_inst, "HELP\r\n");
      uart_send(parser->uart_inst, "\r\nSupported commands:\r\n"
                                   "HELP - call this instruction\r\n"
                                   "SET x [position] - set new position for axys x\r\n"
                                   "SET y [position] - set new position for axys y\r\n"
                                   "SET z [position] - set new position for axys z\r\n\n"
                                   "GET x pos - return position for axys x\r\n"
                                   "GET y pos - return position for axys y\r\n"
                                   "GET z pos - return position for axys z\r\n\n"

                                   "STOP - stop present move\r\n");
      return TASK_HELP;
    }
    else if (!compare(word[0], "stop") && !compare(word[1], "x"))
    {
      uart_send(parser->uart_inst, "STOP X\r\n");
      return TASK_STOP_X;
    }

    else if (!compare(word[0], "stop") && !compare(word[1], "y"))
    {
      uart_send(parser->uart_inst, "STOP Y\r\n");
      return TASK_STOP_Y;
    }

    else if (!compare(word[0], "stop") && !compare(word[1], "z"))
    {
      uart_send(parser->uart_inst, "STOP Z\r\n");
      return TASK_STOP_Z;
    }
    else if((!compare(word[0], "shut") && !compare(word[1], "uC")))
    {
      return TASK_UC_RESET;
    }
    else
    {
      return TASK_ERROR;
    }
  }
#if WORD_LEN > 4
  else if (word_len == 5)
  {

    sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
    if (compare(word[0], "close") == 0)
    {
      uart_send(parser->uart_inst, "CLOSE\r\n");
      return TASK_CLOSE;
    }
  }
  else if (word_len == 6)
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
    if (!compare(word[0], "unmute"))
    {
      uart_send(parser->uart_inst, "˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜\r\n");
      parser->mute = 0;
      return TASK_UNMUTE;
    }
  }
  else if (word_len == 7)
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
    if (!compare(word[0], "restart"))
    {
      HAL_NVIC_SystemReset();
      uart_send_ok(parser->uart_inst);
    }
  }
#endif
  else
  {
    uart_send_error(parser->uart_inst);
    return TASK_ERROR;
  }
  uart_send_error(parser->uart_inst);
  return TASK_ERROR;
}